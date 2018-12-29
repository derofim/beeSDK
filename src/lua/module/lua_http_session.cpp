#include "lua_http_session.h"
#include "http/http_response.h"

namespace bee {

//////////////////////////////////Setup////////////////////////////////////////
LuaHttpSession::LuaHttpSession(IOSPtr ios):
    HttpSession(ios),
    main_(NULL), 
    co_(NULL), 
    manual_read_body_(false),
    one_off_(true),
    status_code_(0), 
    resumed_(false), 
    is_chunk_(false),
    cur_chunk_size_(0),
    cur_chunk_left_(0),
    logger_("LuaHttpSession") {
    for (int32_t i = 0;i < eLuaHttpCallback_Count;++i) {
        lua_callbacks_[i] = LUA_REFNIL;
    }
    logger_.Debug("LuaHttpSession created %x\n", (unsigned int)(long)this);
}

LuaHttpSession::~LuaHttpSession() {
    logger_.Debug("LuaHttpSession deleted %x\n", (unsigned int)(long)this);
}

BeeErrorCode LuaHttpSession::get(
    lua_State *main,
    lua_State *co,
    const std::string& url,
    bool manual_read_body,
    bool one_off,
    const std::string& ref_url, 
    int64_t range_start, 
    int64_t range_end,
    int32_t timeout) {
    BeeErrorCode ret = kBeeErrorCode_Success;
    do {
        if (main == NULL || co == NULL) {
            ret = kBeeErrorCode_Invalid_Param;
            break;
        }

        //Already have a request.
        if (main_ != NULL) {
            ret = kBeeErrorCode_Invalid_State;
            break;
        }

        main_ = main;
        co_ = co;
        manual_read_body_ = manual_read_body;
        one_off_ = one_off;
        
        logger_.Debug("LuaHttpSession requesting %s %x\n", url.c_str(), (unsigned int)(long)this);

        if (!request(url, ref_url, range_start, range_end)) {
            ret = kBeeErrorCode_Error_State_Machine;
            break;
        }

        yielding_ = true;
    } while (0);
    return ret;
}

BeeErrorCode LuaHttpSession::async_get(
    lua_State *main,
    int32_t get_callback,
    const std::string& url,
    bool manual_read_body,
    bool one_off,
    const std::string& ref_url,
    int64_t range_start,
    int64_t range_end,
    int32_t timeout) {
    BeeErrorCode ret = kBeeErrorCode_Success;
    do {
        if (main == NULL || get_callback == LUA_REFNIL) {
            ret = kBeeErrorCode_Invalid_Param;
            break;
        }

        //Already have a request.
        if (main_ != NULL) {
            ret = kBeeErrorCode_Invalid_State;
            break;
        }

        main_ = main;
        get_callback_ = get_callback;
        manual_read_body_ = manual_read_body;
        one_off_ = one_off;

        logger_.Debug("LuaHttpSession async requesting %s %x\n", url.c_str(), (unsigned int)(long)this);

        if (!request(url, ref_url, range_start, range_end)) {
            ret = kBeeErrorCode_Error_State_Machine;
            break;
        }
    } while (0);
    return ret;
}

BeeErrorCode LuaHttpSession::read(lua_State *co, uint8_t *buff, size_t len, int32_t cond) {
    BeeErrorCode ret = kBeeErrorCode_Success;
    do {
        if (buff == NULL || len == 0) {
            ret = kBeeErrorCode_Invalid_Param;
            break;
        }

        if (!connected_) {
            ret = kBeeErrorCode_Invalid_State;
            break;
        }        

        if (cond < eReadCompletionConditon_Some || cond >= eReadCompletionConditon_Count) {
            ret = kBeeErrorCode_Invalid_Param;
            break;
        }

        ReadCompletionConditon econd = static_cast<ReadCompletionConditon>(cond);
        switch (econd) {
            case eReadCompletionConditon_Some:
                reader_.reset(new LuaHttpSomeReader(shared_from_base<LuaHttpSession>(), buff, len, main_, co));
                break;
            case eReadCompletionConditon_Exactly:
            case eReadCompletionConditon_Enough:
            default:
                reader_.reset(new LuaHttpExactlyReader(shared_from_base<LuaHttpSession>(), buff, len, main_, co));
                break;
        }

        if (!reader_->read()) {
            ret = kBeeErrorCode_Read_Fail;
            break;
        }
    } while (0);
    return ret;
}

BeeErrorCode LuaHttpSession::scanf(lua_State *co, const std::vector<DataField> &format, size_t total_size) {
    BeeErrorCode ret = kBeeErrorCode_Success;
    do {
        if (!connected_) {
            ret = kBeeErrorCode_Invalid_State;
            break;
        }

        reader_.reset(new LuaHttpScanfReader(shared_from_base<LuaHttpSession>(), NULL, 0, main_, co));
        LuaHttpScanfReader *preader = (LuaHttpScanfReader *)reader_.get();
        if (!preader->init(total_size, format)) {
            ret = kBeeErrorCode_Invalid_Param;
            break;
        }

        if (!reader_->read()) {
            ret = kBeeErrorCode_Read_Fail;
            break;
        }
    } while (0);
    return ret;
}

void LuaHttpSession::set_lua_callback(LuaHttpCallbackType type, int32_t callback) {
    lua_callbacks_[type] = callback;
}

void LuaHttpSession::close() {
    logger_.Debug("LuaHttpSession closing, %x\n", (unsigned int)(long)this);
    if (reader_ != NULL) {
        reader_->close();
        reader_.reset();
    }
    main_ = NULL;
    co_ = NULL;
    TcpStateMachine::stop();
}

BeeErrorCode LuaHttpSession::handle_resolve(BeeErrorCode ec1, const boost::system::error_code &ec2, const std::vector<tcp::endpoint> &resolved_addr) {
    BeeErrorCode ret = kBeeErrorCode_Success;
    do {
        logger_.Debug("LuaHttpSession handle_resolve, %x\n", (unsigned int)(long)this);
        
        if (main_ == NULL) {
            ret = kBeeErrorCode_Engine_Internal_Error;
            break;
        }

        int32_t callback = lua_callbacks_[eLuaHttpCallback_Resolve];
        if (callback == LUA_REFNIL) {
            break;
        }
        
        lua_State *co = lua_newthread(main_);

        //Push callback.
        lua_rawgeti(co, LUA_REGISTRYINDEX, callback);

        //1.Bee Error Code.
        lua_pushinteger(co, (lua_Integer)ec1);

        //2.Platform Error Code.
        lua_pushinteger(co, (lua_Integer)ec2.value());

        //3.Platform Error Message.
        lua_pushstring(co, ec2.message().c_str());

        //4.Hosts table.
        lua_newtable(co);
        lua_pushnumber(co, -1);
        lua_rawseti(co, -2, 0);

        for (size_t i = 0; i < resolved_addr.size(); ++i) {
            lua_pushstring(co, resolved_addr[i].address().to_string().c_str()); //push   
            lua_rawseti(co, -2, i + 1); // 
        }

        logger_.Debug("LuaHttpSession resume in handle_resolve, %x\n", (unsigned int)(long)this);
        
        int32_t nret = lua_resume(co, main_, 4);
        lua_pop(main_, 1);

        bool accept = true;
        if (LUA_OK == nret && lua_gettop(co) == 1) {
            accept = lua_toboolean(co, -1) == 0 ? false : true;
        }

        if (!accept) {
            ret = kBeeErrorCode_Engine_Reject;
        }

        if (nret != LUA_OK && nret != LUA_YIELD) {
            logger_.Error("handle_resolve resume failed.\n");
            ret = kBeeErrorCode_Engine_Script_Error;
            int32_t args_len = lua_gettop(co);
            if (args_len > 0 && lua_type(co, -1) == LUA_TSTRING) {
                logger_.Error("handle_resolve error %s.\n", lua_tostring(co, -1));
            }
        }
    } while (0);
    return ret;
}

BeeErrorCode LuaHttpSession::handle_connect(BeeErrorCode ec1, const boost::system::error_code &ec2, const tcp::endpoint &connected_endpoint) {
    BeeErrorCode ret = kBeeErrorCode_Success;
    do {
        if (main_ == NULL) {
            ret = kBeeErrorCode_Engine_Internal_Error;
            break;
        }
        
        logger_.Debug("LuaHttpSession handle_connect, %x\n", (unsigned int)(long)this);

        int32_t callback = lua_callbacks_[eLuaHttpCallback_Connect];
        if (callback == LUA_REFNIL) {
            break;
        }

        lua_State *co = lua_newthread(main_);

        //Push callback.
        lua_rawgeti(co, LUA_REGISTRYINDEX, callback);

        //1.Bee Error Code.
        lua_pushinteger(co, (lua_Integer)ec1);

        //2.Platform Error Code.
        lua_pushinteger(co, (lua_Integer)ec2.value());

        //3.Platform Error Message.
        lua_pushstring(co, ec2.message().c_str());

        //4.Connect host.
        std::string host = connected_endpoint.address().to_string();
        lua_pushstring(co, host.c_str());

        logger_.Debug("LuaHttpSession resume in handle_connect, %x\n", (unsigned int)(long)this);
        
        int32_t nret = lua_resume(co, main_, 4);
        lua_pop(main_, 1);

        bool accept = true;
        if (LUA_OK == nret && lua_gettop(co) == 1) {
            accept = lua_toboolean(co, -1) == 0 ? false : true;
        }

        if (!accept) {
            ret = kBeeErrorCode_Engine_Reject;
        }

        if (nret != LUA_OK && nret != LUA_YIELD) {
            logger_.Error("handle_connect resume failed.\n");
            ret = kBeeErrorCode_Engine_Script_Error;
            int32_t args_len = lua_gettop(co);
            if (args_len > 0 && lua_type(co, -1) == LUA_TSTRING) {
                logger_.Error("handle_connect error %s.\n", lua_tostring(co, -1));
            }
        }
    } while (0);
    return ret;
}

BeeErrorCode LuaHttpSession::handle_send(BeeErrorCode ec1, const boost::system::error_code &ec2, size_t bytes_send) {
    BeeErrorCode ret = kBeeErrorCode_Success;
    do {
        if (main_ == NULL) {
            ret = kBeeErrorCode_Engine_Internal_Error;
            break;
        }
        
        logger_.Debug("LuaHttpSession handle_send, %x\n", (unsigned int)(long)this);

        int32_t callback = lua_callbacks_[eLuaHttpCallback_Request];
        if (callback == LUA_REFNIL) {
            break;
        }

        lua_State *co = lua_newthread(main_);

        //Push callback.
        lua_rawgeti(co, LUA_REGISTRYINDEX, callback);

        //1.Bee Error Code.
        lua_pushinteger(co, (lua_Integer)ec1);

        //2.Platform Error Code.
        lua_pushinteger(co, (lua_Integer)ec2.value());

        //3.Platform Error Message.
        lua_pushstring(co, ec2.message().c_str());

        //4.Bytes send.
        lua_pushinteger(co, bytes_send);

        logger_.Debug("LuaHttpSession resume in handle_send, %x\n", (unsigned int)(long)this);
        
        int32_t nret = lua_resume(co, main_, 4);
        lua_pop(main_, 1);

        bool accept = true;
        if (LUA_OK == nret && lua_gettop(co) == 1) {
            accept = lua_toboolean(co, -1) == 0 ? false : true;
        }

        if (!accept) {
            ret = kBeeErrorCode_Engine_Reject;
        }

        if (nret != LUA_OK && nret != LUA_YIELD) {
            logger_.Error("handle_send resume failed.\n");
            ret = kBeeErrorCode_Engine_Script_Error;
            int32_t args_len = lua_gettop(co);
            if (args_len > 0 && lua_type(co, -1) == LUA_TSTRING) {
                logger_.Error("handle_send error %s.\n", lua_tostring(co, -1));
            }
        }
    } while (0);
    return ret;
}

BeeErrorCode LuaHttpSession::handle_rcv_header(BeeErrorCode ec1, const boost::system::error_code &ec2, std::shared_ptr<HttpResponse> header) {
    BeeErrorCode ret = kBeeErrorCode_Success;
    do {
        if (main_ == NULL) {
            ret = kBeeErrorCode_Engine_Internal_Error;
            break;
        }

        if (header == NULL) {
            ret = kBeeErrorCode_Null_Pointer;
            break;
        }
        
        logger_.Debug("LuaHttpSession handle_rcv_header, %x\n", (unsigned int)(long)this);

        header_table_ = header->get_header_table();
        status_code_  = header->get_status_code();
        if (header->get_content_len() == -1) {
            is_chunk_ = true;
        }

        int32_t callback = lua_callbacks_[eLuaHttpCallback_Header];
        if (callback > LUA_REFNIL) {
            lua_State *co = lua_newthread(main_);

            //Push callback.
            lua_rawgeti(co, LUA_REGISTRYINDEX, callback);

            //Push response.
            push_response(co, ec1, ec2, header_table_);

            logger_.Debug("LuaHttpSession resume in handle_rcv_header, %x\n", (unsigned int)(long)this);
            
            int32_t nret = lua_resume(co, main_, 4);
            lua_pop(main_, 1);

            bool accept = true;
            if (LUA_OK == nret && lua_gettop(co) == 1) {
                accept = lua_toboolean(co, -1) == 0 ? false : true;
            }

            if (!accept) {
                ret = kBeeErrorCode_Engine_Reject;
            }

            if (nret != LUA_OK && nret != LUA_YIELD) {
                logger_.Error("handle_rcv_header resume failed.\n");
                ret = kBeeErrorCode_Engine_Script_Error;
                int32_t args_len = lua_gettop(co);
                if (args_len > 0 && lua_type(co, -1) == LUA_TSTRING) {
                    logger_.Error("handle_rcv_header error %s.\n", lua_tostring(co, -1));
                }
            }
        }

        if (!manual_read_body_) { //Need to wait for body.
            logger_.Debug("LuaHttpSession handle_rcv_header wait for body, %x\n", (unsigned int)(long)this);
            break;
        }

        ret = kBeeErrorCode_State_Machine_Finished;
        
        logger_.Debug("LuaHttpSession resume in handle_rcv_header, manual read body, %x\n", (unsigned int)(long)this);
        report(ec1, ec2, NULL, 0); //If manually read body, no need to wait.
    } while (0);
    return ret;
}

BeeErrorCode LuaHttpSession::handle_redirect(int32_t status_code, const std::string &location) {
    BeeErrorCode ret = kBeeErrorCode_Success;
    do {
        if (main_ == NULL) {
            ret = kBeeErrorCode_Engine_Internal_Error;
            break;
        }

        logger_.Info("LuaHttpSession handle_redirect %d %s, %x\n", status_code, location.c_str(), (unsigned int)(long)this);

        int32_t callback = lua_callbacks_[eLuaHttpCallback_Redirect];
        if (callback == LUA_REFNIL) {
            break;
        }

        lua_State *co = lua_newthread(main_);

        //Push callback.
        lua_rawgeti(co, LUA_REGISTRYINDEX, callback);

        //1.Status Code.
        lua_pushinteger(co, (lua_Integer)status_code);

        //2.location.
        lua_pushstring(co, location.c_str());

        logger_.Debug("LuaHttpSession resume in handle_redirect, %x\n", (unsigned int)(long)this);

        int32_t nret = lua_resume(co, main_, 2);
        lua_pop(main_, 1);

        bool accept = true;
        if (LUA_OK == nret && lua_gettop(co) == 1) {
            accept = lua_toboolean(co, -1) == 0 ? false : true;
        }

        if (!accept) {
            ret = kBeeErrorCode_Engine_Reject;
        }

        if (nret != LUA_OK && nret != LUA_YIELD) {
            logger_.Error("handle_redirect resume failed.\n");
            ret = kBeeErrorCode_Engine_Script_Error;
            int32_t args_len = lua_gettop(co);
            if (args_len > 0 && lua_type(co, -1) == LUA_TSTRING) {
                logger_.Error("handle_redirect error %s.\n", lua_tostring(co, -1));
            }
        }
    } while (0);
    return ret;
}

BeeErrorCode LuaHttpSession::handle_rcv_content(BeeErrorCode ec1, const boost::system::error_code &ec2, IOBuffer &data) {
    BeeErrorCode ret = kBeeErrorCode_Success;
    do {
        if (main_ == NULL) {
            ret = kBeeErrorCode_Engine_Internal_Error;
            break;
        }
        
        logger_.Debug("LuaHttpSession handle_rcv_content %x\n", (unsigned int)(long)this);

        int32_t callback = lua_callbacks_[eLuaHttpCallback_Body];
        if (callback > LUA_REFNIL) {
            lua_State *co = lua_newthread(main_);

            //Push callback.
            lua_rawgeti(co, LUA_REGISTRYINDEX, callback);

            //Push response.
            push_response(co, ec1, ec2, HttpHeaderTable(), (const char*)data.data(), data.size());

            logger_.Debug("LuaHttpSession resume in handle_rcv_content 1 %x\n", (unsigned int)(long)this);
            
            int32_t nret = lua_resume(co, main_, 4);
            lua_pop(main_, 1);

            bool accept = true;
            if (LUA_OK == nret && lua_gettop(co) == 1) {
                accept = lua_toboolean(co, -1) == 0 ? false : true;
            }

            if (!accept) {
                ret = kBeeErrorCode_Engine_Reject;
            }

            if (nret != LUA_OK && nret != LUA_YIELD) {
                logger_.Error("handle_rcv_content resume failed.\n");
                ret = kBeeErrorCode_Engine_Script_Error;
                int32_t args_len = lua_gettop(co);
                if (args_len > 0 && lua_type(co, -1) == LUA_TSTRING) {
                    logger_.Error("handle_rcv_content error %s.\n", lua_tostring(co, -1));
                }
            }
        }

        logger_.Debug("LuaHttpSession resume in handle_rcv_content 2 %x\n", (unsigned int)(long)this);       
        report(ec1, ec2, (const char*)data.data(), data.size());
        //Close connection right after report, so lua doesn't need to take care of connection,
        //so notice lua http session is a one-off object, do not call get twice.
        if (one_off_ && ios_ != NULL) {
            ios_->post(boost::bind(&LuaHttpSession::close, shared_from_base<LuaHttpSession>()));
        }
    } while (0);
    return ret;
}

bool LuaHttpSession::async_load() {
    bool ret = true;
    do {
        IOSPtr _ios = ios();
        if (_ios == NULL) {
            ret = false;
            break;
        }

        _ios->post(boost::bind(&LuaHttpSession::load, shared_from_base<LuaHttpSession>()));
    } while (0);
    return ret;
}

bool LuaHttpSession::push_response(lua_State *l, BeeErrorCode ec1, const boost::system::error_code &ec2, const HttpHeaderTable &header_table, const char *body, size_t body_size) {
    bool ret = true;
    do {
        if (l == NULL) {
            ret = false;
            break;
        }

        //1.Bee Error Code.
        lua_pushinteger(l, (lua_Integer)ec1);

        //2.Platform Error Code.
        lua_pushinteger(l, (lua_Integer)ec2.value());

        //3.Platform Error Message.
        lua_pushstring(l, ec2.message().c_str());

        //4.Response.
        lua_newtable(l);

        lua_pushstring(l, "status_code");
        lua_pushinteger(l, status_code_);
        lua_settable(l, -3);

        lua_pushstring(l, "http_time");
        lua_pushinteger(l, get_total_time());
        lua_settable(l, -3);

        if (!header_table.empty()) {
            lua_pushstring(l, "http_header");
            lua_newtable(l);
            HttpHeaderTable::const_iterator iter = header_table.begin();
            for (;header_table.end() != iter; ++iter) {
                lua_pushstring(l, iter->first.c_str());
                lua_pushstring(l, iter->second.c_str());
                lua_settable(l, -3);
            }
            lua_settable(l, -3);
        }
 
        if (body != NULL && body_size > 0) {
            lua_pushstring(l, "http_body");
            lua_pushlstring(l, body, body_size);
            lua_settable(l, -3);
        }

        if (status_code_ == 404) {
            int n = lua_gettop(l);
            break;
        }
    } while (0);
    return ret;
}

void LuaHttpSession::report(BeeErrorCode ec1, const boost::system::error_code &ec2, const char *body, size_t body_size) {
    if (get_callback_ != LUA_REFNIL) {
        report_async_get_result(ec1, ec2, body, body_size);
    } else {
        resume(ec1, ec2, body, body_size);
    }
}

int32_t LuaHttpSession::report_async_get_result(BeeErrorCode ec1, const boost::system::error_code &ec2, const char *body, size_t body_size) {
    int32_t ret = -1;
    if (main_ != NULL && get_callback_ != LUA_REFNIL) {
        lua_State *co = lua_newthread(main_);

        //Push callback.
        lua_rawgeti(co, LUA_REGISTRYINDEX, get_callback_);

        //Push response.
        push_response(co, ec1, ec2, header_table_, body, body_size);

        logger_.Debug("report_async_get_result %x\n", (unsigned int)(long)this);

        ret = lua_resume(co, main_, 4);
        if (ret != LUA_OK && ret != LUA_YIELD) {
            logger_.Error("report_async_get_result resume failed.\n");
            int32_t args_len = lua_gettop(co);
            if (args_len > 0 && lua_type(co, -1) == LUA_TSTRING) {
                logger_.Error("report_async_get_result error %s.\n", lua_tostring(co, -1));
            }
        }
    }
    return ret;
}

void LuaHttpSession::resume(BeeErrorCode ec1, const boost::system::error_code &ec2, const char *body, size_t body_size) {
    if (co_ != NULL && main_ != NULL && yielding_ && !resumed_) {
        logger_.Info("LuaHttpSession::resume %x.\n", (unsigned int)(long)this);
        lua_settop(co_, 0);
        push_response(co_, ec1, ec2, header_table_, body, body_size);
        resumed_ = true;
        yielding_ = false;
        lua_resume(co_, main_, 0);
    }
}

void LuaHttpSession::load() {
    do {
        if (!is_chunk_) {//TBD,not chunk
            break;
        }

        if (cur_chunk_left_ > 0) {
            if (reader_ != NULL) {
                boost::system::error_code ec;
                reader_->notify(kBeeErrorCode_Success, ec, cur_chunk_left_);
            }
            break;
        }

        read_chunk_size();
    } while (0);
}

void LuaHttpSession::read_chunk_size() {
    std::string delim("\r\n");
    async_read_until(
        delim,
        boost::bind(
        &LuaHttpSession::handle_read_chunk_size, 
        shared_from_base<LuaHttpSession>(),
        boost::asio::placeholders::error,
        boost::asio::placeholders::bytes_transferred));
}

void LuaHttpSession::load_chunk_body(size_t chunk_size) {
    const size_t extra_bytes = 2;
    std::size_t need_read_len = chunk_size + extra_bytes;
    async_read(
        boost::asio::transfer_exactly(need_read_len),
        boost::bind(
        &LuaHttpSession::handle_load_chunk_body, 
        shared_from_base<LuaHttpSession>(),
        boost::asio::placeholders::error,
        boost::asio::placeholders::bytes_transferred));
}

void LuaHttpSession::output(StateEvent::Ptr ev) {
    //Report error when state machine interrupted.
    if (ev->ec1 != kBeeErrorCode_Success && ev->ec1 != kBeeErrorCode_State_Machine_Finished) {
        report(ev->ec1, ev->ec2, NULL, 0);
        if (one_off_ && ios_ != NULL) {
            ios_->post(boost::bind(&LuaHttpSession::close, shared_from_base<LuaHttpSession>()));
        }
    }
}

void LuaHttpSession::handle_read_chunk_size(const boost::system::error_code& ec2, size_t trans_bytes) {
    BeeErrorCode ec1 = kBeeErrorCode_Success;
    do {
        if (ec2) {
            ec1 = kBeeErrorCode_Read_Http_Chunk_Size_Fail;
            break;
        }

        std::istream is(&rcv_streambuf_);
        is >> std::hex >> cur_chunk_size_;
        char ch;
        while (is.get(ch) && ch != '\n');

        if (cur_chunk_size_ == 0) {
            ec1 = kBeeErrorCode_Eof;
            break;
        }

        load_chunk_body(cur_chunk_size_);
    } while (0);
    if (ec1 != kBeeErrorCode_Success && reader_ != NULL) {
        reader_->notify(ec1, ec2, 0);
    }
}

void LuaHttpSession::handle_load_chunk_body(const boost::system::error_code& ec2, size_t trans_bytes) {
    BeeErrorCode ec1 = kBeeErrorCode_Success;
    size_t availble_len = 0;
    do {
        if (ec2) {
            ec1 = kBeeErrorCode_Read_Http_Chunk_Body_Fail;
            break;
        }

        cur_chunk_left_ = cur_chunk_size_;
        availble_len = cur_chunk_size_;
    } while (0);
    if (reader_ != NULL) {
        reader_->notify(ec1, ec2, availble_len);
    }
}

size_t LuaHttpSession::read_stream(uint8_t *buff, size_t len) {
    size_t ret = 0;
    do {
        if (buff == NULL || len == 0 || cur_chunk_left_ == 0) {
            break;
        }

        ret = std::min<size_t>(len, cur_chunk_left_);
        std::istream is(&rcv_streambuf_);
        is.read((char*)buff, ret);
        cur_chunk_left_ -= ret;
        if (cur_chunk_left_ == 0) {
            is.get(); //"\r"
            is.get(); //"\n"
        }
    } while (0);
    return ret;
}

int32_t LuaHttpSession::http_get_result(lua_State *l, int32_t status, lua_KContext ctx) {
    int32_t n = lua_gettop(l);
    return n;
}

} // namespace bee
