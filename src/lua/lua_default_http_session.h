#ifndef __LUA_DEFAULT_HTTP_SESSION_H__
#define __LUA_DEFAULT_HTTP_SESSION_H__

#include "utility/common.h"
#include "network/http_session.h"

extern "C" {
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
}

namespace bee {

class BeeSession;
class LuaVideoCache;
class LuaDefaultHttpSession :
    public HttpHandler,
    public std::enable_shared_from_this<LuaDefaultHttpSession> {
public:
    typedef std::shared_ptr<LuaDefaultHttpSession> Ptr;
    LuaDefaultHttpSession();
    ~LuaDefaultHttpSession();

public:
    bool request(
        IOSPtr ios,
        lua_State *main,
        lua_State *co,
        const std::string& url,
        const std::string& ref_url = "", 
        int64_t range_start = -1, 
        int64_t range_end = -1,
        int32_t timeout = -1,
        LuaVideoCache *cache = NULL);

public:
    bool handle_resolve(const boost::system::error_code &ec, const std::vector<tcp::endpoint> &resolved_addr);
    bool handle_connect(const boost::system::error_code &ec, const tcp::endpoint &connected_endpoint);
    bool handle_send(const boost::system::error_code &ec, size_t bytes_send);
    bool handle_rcv_header(const boost::system::error_code &ec, std::shared_ptr<HttpResponse> header);
    bool handle_rcv_content(const boost::system::error_code &ec, IOBuffer &data);
    bool feedback(IOBuffer &data);

protected:
    HttpSession::Ptr http_session_;
    lua_State *main_;
    lua_State *co_;
    LuaVideoCache *video_cache_;
    int32_t timeout_;
    uint32_t response_code_;
    HttpHeaderTable header_table_;
};

} // namespace bee

#endif
