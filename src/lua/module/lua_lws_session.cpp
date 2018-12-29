#include "lua_lws_session.h"
#include "lua_lws_service.h"
#include "lua_module_mgr.h"

namespace bee {

LuaLwsSession::LuaLwsSession(
    IOSPtr ios, 
    std::shared_ptr<LuaLwsService> lws_service, 
    const std::string &protocol, 
    bool json_completed,
    int32_t read_callback, 
    int32_t error_callback, 
    lua_State *main, 
    lua_State *co)
    : ios_(ios),
      lws_service_(lws_service),
      protocol_(protocol),
      check_json_completed_(json_completed),
      main_(main),
      co_(co),
      logger_("LuaLwsSession") {
    lua_callbacks_[eLuaLwsCallback_Connect] = LUA_REFNIL;
    lua_callbacks_[eLuaLwsCallback_Read] = read_callback;
    lua_callbacks_[eLuaLwsCallback_Error] = error_callback;
    logger_.Debug("LuaLwsSession created %x.\n", (unsigned int)(long)this);
}

LuaLwsSession::~LuaLwsSession() {
    logger_.Debug("LuaLwsSession deleted %x.\n", (unsigned int)(long)this);
}

BeeErrorCode LuaLwsSession::connect(const char *url, int32_t timeout, int32_t connect_callback) {
    BeeErrorCode ret = kBeeErrorCode_Success;
    do {
        if (url == NULL || lws_service_ == NULL || connect_callback == LUA_REFNIL) {
            ret = kBeeErrorCode_Invalid_Param;
            break;
        }

        lua_callbacks_[eLuaLwsCallback_Connect] = connect_callback;
        lws_service_->connect(shared_from_this(), url, protocol_);

        if (timeout > 0 && ios_ != NULL) {
            connect_timer_ = AsyncWaitTimer::create(*ios_);
            connect_timer_->set_wait_millseconds(timeout);
            connect_timer_->set_wait_times(1);
            connect_timer_->set_immediately(false);
            connect_timer_->async_wait(boost::bind(&LuaLwsSession::handle_connect_timeout, shared_from_this()));
        }
    } while (0);
    return ret;
}

void LuaLwsSession::close() {
    do {
        if (closed_ || closing_) {
            break;
        }

        logger_.Trace("LuaLwsSession closing %x %x.\n", (unsigned int)(long)this, (unsigned int)(long)wsi_);
        closing_ = true;

        if (lws_service_ == NULL) {
            break;
        }

        lws_service_->close(wsi_);
        lws_service_.reset();

        if (connect_timer_ != NULL) {
            connect_timer_->cancel();
            connect_timer_.reset();
        }
    } while (0);
}

BeeErrorCode LuaLwsSession::write(int32_t handle, const char *data) {
    BeeErrorCode ret = kBeeErrorCode_Success;
    do {
        if (closing_ || closed_) {
            ret = kBeeErrorCode_Not_Connected;
            break;
        }

        if (data == NULL || lws_service_ == NULL) {
            ret = kBeeErrorCode_Invalid_Param;
            break;
        }

        lws_service_->write(wsi_, data);
    } while (0);
    return ret;
}

void LuaLwsSession::report_event(struct lws *wsi, lws_callback_reasons reason, void *in, size_t len) {
    if (ios_ != NULL) {
        const std::string &data = (in == NULL) ? std::string("") : std::string((const char*)in, len);
        ios_->post(boost::bind(&LuaLwsSession::on_event, shared_from_this(), wsi, reason, data));
    }
}

void LuaLwsSession::on_connected(lws *wsi) {
    lws_callback_on_writable(wsi);
}

int32_t LuaLwsSession::on_wait_result(lua_State *l, int32_t status, lua_KContext ctx) {
    int32_t nresult = lua_gettop(l);
    return nresult;
}

void LuaLwsSession::report_connection_result(struct lws *wsi, bool success, const std::string &data) {
    do {
        if (main_ == NULL || connect_reported) {
            break;
        }

        int32_t callback = lua_callbacks_[eLuaLwsCallback_Connect];
        if (callback == LUA_REFNIL) {
            break;
        }

        if (success) {
            wsi_ = wsi;
        }

        lua_State *co = lua_newthread(main_);
        lua_rawgeti(co, LUA_REGISTRYINDEX, callback);
        lua_pushboolean(co, success);
        lua_pushlstring(co, data.c_str(), data.size());
        int32_t ret = lua_resume(co, main_, 2);
        lua_pop(main_, 1);

        connect_reported = true;
        if (connect_timer_ != NULL) {
            connect_timer_->cancel();
            connect_timer_.reset();
        }

        if (ret != LUA_OK && ret != LUA_YIELD) {
            logger_.Error("report_connection_result resume failed.\n");
            int32_t args_len = lua_gettop(co);
            if (args_len > 0 && lua_type(co, -1) == LUA_TSTRING) {
                logger_.Error("report_connection_result error %s.\n", lua_tostring(co, -1));
            }
        }
    } while (0);
}

void LuaLwsSession::report_disconnected(struct lws *wsi) {
    do {
        if (main_ == NULL || !connected_) {
            break;
        }

        int32_t callback = lua_callbacks_[eLuaLwsCallback_Connect];
        if (callback == LUA_REFNIL) {
            break;
        }

        lua_State *co = lua_newthread(main_);
        lua_rawgeti(co, LUA_REGISTRYINDEX, callback);
        lua_pushboolean(co, false);
        std::string data = "Disconnected";
        lua_pushlstring(co, data.c_str(), data.size());
        int32_t ret = lua_resume(co, main_, 2);
        lua_pop(main_, 1);

        if (ret != LUA_OK && ret != LUA_YIELD) {
            logger_.Error("report_connection_result resume failed.\n");
            int32_t args_len = lua_gettop(co);
            if (args_len > 0 && lua_type(co, -1) == LUA_TSTRING) {
                logger_.Error("report_connection_result error %s.\n", lua_tostring(co, -1));
            }
        }
    } while (0);
}

void LuaLwsSession::report_read_data(const std::string &data) {
    do {
        std::string *msg = (std::string*)&data;
        if (check_json_completed_ && !get_completed_json_message(data, msg)) {
            break;
        }

        if (msg == NULL) {
            break;
        }

        if (main_ == NULL) {
            break;
        }

        int32_t callback = lua_callbacks_[eLuaLwsCallback_Read];
        if (callback == LUA_REFNIL) {
            break;
        }

        lua_State *co = lua_newthread(main_);
        lua_rawgeti(co, LUA_REGISTRYINDEX, callback);
        lua_pushlstring(co, msg->c_str(), msg->size());
        int32_t nret = lua_resume(co, main_, 1);
        lua_pop(main_, 1);

        if (!current_msg_.empty()) {
            current_msg_.clear();
        }

        if (LUA_YIELD != nret && LUA_OK != nret) {
            logger_.Error("%x report_read_data resume failed.\n", (unsigned int)(long)this);
            int32_t args_len = lua_gettop(co);
            if (args_len > 0 && lua_type(co, -1) == LUA_TSTRING) {
                logger_.Error("lws thread error %s.\n", lua_tostring(co, -1));
            }
            break;
        }
    } while (0);
}

bool LuaLwsSession::get_completed_json_message(const std::string &data, std::string *&msg) {
    bool ret = true;
    do {
        bool cached_msg = false;
        if (!current_msg_.empty()) {
            cached_msg = true;
            current_msg_ += data;
        }

        BeeJson::Reader jreader;
        BeeJson::Value jdata;
        const std::string &parse_msg = cached_msg ? current_msg_ : data;
        if (!jreader.parse(parse_msg, jdata)) {
            if (!cached_msg) {
                current_msg_ = data;
            }
            ret = false;
            break;
        }

        if (msg != NULL) {
            msg = (std::string *)&parse_msg;
        }
    } while (0);
    return ret;
}

void LuaLwsSession::on_event(struct lws *wsi, lws_callback_reasons reason, const std::string &data) {
    if (closing_ || closed_) { //At this time, Lua Context could have been released, it's may be last event, but closed flag should be set, just ignore it.
        return;
    }

    switch (reason) {
    case LWS_CALLBACK_CLIENT_ESTABLISHED:
        connected_ = true;
        report_connection_result(wsi, true, data);
        break;
    case LWS_CALLBACK_CLIENT_CONNECTION_ERROR:
        report_connection_result(wsi, false, data);
        break;
    case LWS_CALLBACK_CLOSED:
        logger_.Error("lws client closed by remote %x\n", (unsigned int)(long)this);
        closed_ = true;
        report_disconnected(wsi);
        break;
    case LWS_CALLBACK_CLIENT_RECEIVE:
        report_read_data(data);
        break;
    case LWS_CALLBACK_WS_PEER_INITIATED_CLOSE:
        logger_.Error("lws client initiated closed by remote %x\n", (unsigned int)(long)this);
        closed_ = true;
        report_disconnected(wsi);
        break;
    case LWS_CALLBACK_WSI_DESTROY:
        logger_.Error("lws client destroyed by internal %x\n", (unsigned int)(long)this);
        closed_ = true;
        report_disconnected(wsi);
        break;
    default:
        break;
    }
}

void LuaLwsSession::handle_connect_timeout() {
    if (!connect_reported) {
        report_connection_result(NULL, false, "timeout");
    }
}

} // namespace bee
