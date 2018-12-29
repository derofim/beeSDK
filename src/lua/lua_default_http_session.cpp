#include "lua_default_http_session.h"
#include "session/session_manager.h"
#include "http/http_response.h"
#include "module/lua_video_cache.h"

namespace bee {

LuaDefaultHttpSession::LuaDefaultHttpSession():
    main_(NULL),
    co_(NULL),
    video_cache_(NULL),
    timeout_(-1),
    response_code_(0) {

}

LuaDefaultHttpSession::~LuaDefaultHttpSession() {

}

bool LuaDefaultHttpSession::request(
    IOSPtr ios,
    lua_State *main,
    lua_State *co,
    const std::string& url,
    const std::string& ref_url, 
    int64_t range_start, 
    int64_t range_end,
    int32_t timeout,
    LuaVideoCache *cache) {
    bool ret = true;
    do {
        if (ios == NULL || main == NULL || co == NULL || url.empty()) {
            ret = false;
            break;
        }

        http_session_.reset(new HttpSession(ios));
        http_session_->set_handler(shared_from_this());
        http_session_->set_wait_content_complete(cache == NULL?true:false);

        ret = http_session_->request(url, ref_url, range_start, range_end);
        if (!ret) {
            break;
        }

        main_ = main;
        co_ = co;
        video_cache_ = cache;
        timeout_ = timeout;
    } while (0);
    return ret;
}

bool LuaDefaultHttpSession::handle_resolve(const boost::system::error_code &ec, const std::vector<tcp::endpoint> &resolved_addr) {
    return true;
}

bool LuaDefaultHttpSession::handle_connect(const boost::system::error_code &ec, const tcp::endpoint &connected_endpoint) {
    return true;
}

bool LuaDefaultHttpSession::handle_send(const boost::system::error_code &ec, size_t bytes_send) {
    return true;
}

bool LuaDefaultHttpSession::handle_rcv_header(const boost::system::error_code &ec, std::shared_ptr<HttpResponse> header) {
    bool ret = true;
    do {
        if (ec || header == NULL) {
            ret = false;
            break;
        }

        response_code_ = header->get_status_code();
        header_table_ = header->get_header_table();
    } while (0);
    return ret;
}

bool LuaDefaultHttpSession::handle_rcv_content(const boost::system::error_code &ec, IOBuffer &data) {
    bool ret = true;
    do {
        if (ec) {
            ret = false;
            break;
        }

        if (video_cache_ == NULL) {
            feedback(data);
        } else {
            video_cache_->PromiseWriteData((uint8_t*)data.data(), data.size());
        }
    } while (0);
    return ret;
}

bool LuaDefaultHttpSession::feedback(IOBuffer &data) {
    bool ret = true;
    do {
        if (main_ == NULL || co_ == NULL) {
            ret = false;
            break;
        }

        lua_settop(co_, 0);

        lua_newtable(co_);
        if (!data.empty()) {
            lua_pushstring(co_, "http_body");
            lua_pushlstring(co_, data.data(), data.size());
            lua_settable(co_, -3);
        }

        lua_pushstring(co_, "status_code");
        lua_pushinteger(co_, response_code_);
        lua_settable(co_, -3);

        lua_pushstring(co_, "http_time");
        lua_pushinteger(co_, http_session_->get_total_time());
        lua_settable(co_, -3);

        lua_pushstring(co_, "http_header");
        lua_newtable(co_);
        HttpHeaderTable::iterator iter = header_table_.begin();
        for (;header_table_.end() != iter; ++iter) {
            lua_pushstring(co_, iter->first.c_str());
            lua_pushstring(co_, iter->second.c_str());
            lua_settable(co_, -3);
        }
        lua_settable(co_, -3);

        lua_resume(co_, main_, 0);
    } while (0);
    return ret;
}

} // namespace bee
