#ifndef __LUA_HTTP_SESSION_H__
#define __LUA_HTTP_SESSION_H__

#include "lua_http_reader.h"
#include "network/http_session.h"
#include "log/logger.h"

extern "C" {
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
}

namespace bee {

typedef enum LuaHttpCallbackType {
    eLuaHttpCallback_Resolve = 0,
    eLuaHttpCallback_Connect,
    eLuaHttpCallback_Request,
    eLuaHttpCallback_Header,
    eLuaHttpCallback_Redirect,
    eLuaHttpCallback_Body,
    eLuaHttpCallback_Count
}LuaHttpCallbackType;

class HttpSession;
class HttpResponse;
class LuaHttpSession : public HttpSession {
public:
    typedef std::shared_ptr<LuaHttpSession> Ptr;
    typedef std::weak_ptr<LuaHttpSession> WPtr;
    LuaHttpSession(IOSPtr ios);
    virtual ~LuaHttpSession();

public:
    //Called from lua.
    BeeErrorCode get(
        lua_State *main,
        lua_State *co,
        const std::string& url,
        bool manual_read_body = false,
        bool one_off = true,
        const std::string& ref_url = "", 
        int64_t range_start = -1, 
        int64_t range_end = -1,
        int32_t timeout = -1);

    BeeErrorCode async_get(
        lua_State *main,
        int32_t get_callback,
        const std::string& url,
        bool manual_read_body = false,
        bool one_off = true,
        const std::string& ref_url = "",
        int64_t range_start = -1,
        int64_t range_end = -1,
        int32_t timeout = -1);

    BeeErrorCode read(lua_State *co, uint8_t *buff, size_t len, int32_t cond);
    BeeErrorCode scanf(lua_State *co, const std::vector<DataField> &format, size_t total_size);
    void    set_lua_callback(LuaHttpCallbackType type, int32_t callback);
    void    close();

    //Callbacks.
    virtual BeeErrorCode handle_resolve(BeeErrorCode ec1, const boost::system::error_code &ec2, const std::vector<tcp::endpoint> &resolved_addr);
    virtual BeeErrorCode handle_connect(BeeErrorCode ec1, const boost::system::error_code &ec2, const tcp::endpoint &connected_endpoint);
    virtual BeeErrorCode handle_send(BeeErrorCode ec1, const boost::system::error_code &ec2, size_t bytes_send);
    virtual BeeErrorCode handle_rcv_header(BeeErrorCode ec1, const boost::system::error_code &ec2, std::shared_ptr<HttpResponse> header);
    virtual BeeErrorCode handle_redirect(int32_t status_code, const std::string &location);
    virtual BeeErrorCode handle_rcv_content(BeeErrorCode ec1, const boost::system::error_code &ec2, IOBuffer &data);   

    //Called from reader.
    bool    async_load();
    size_t  read_stream(uint8_t *buff, size_t len);
    static  int32_t http_get_result(lua_State *l, int32_t status, lua_KContext ctx);

protected:
    int32_t report_async_get_result(BeeErrorCode ec1, const boost::system::error_code &ec2, const char *body, size_t body_size);
    void    resume(BeeErrorCode ec1, const boost::system::error_code &ec2, const char *body = NULL, size_t body_size = 0);   
    void    load();
    void    read_chunk_size();
    void    load_chunk_body(size_t chunk_size);
    void    output(StateEvent::Ptr ev); 
    void    handle_read_chunk_size(const boost::system::error_code& ec2, size_t trans_bytes);
    void    handle_load_chunk_body(const boost::system::error_code& ec2, size_t trans_bytes);
    bool    push_response(lua_State *l, BeeErrorCode ec1, const boost::system::error_code &ec2, const HttpHeaderTable &header_table, const char *body = NULL, size_t body_size = 0);     
    void    report(BeeErrorCode ec1, const boost::system::error_code &ec2, const char *body, size_t body_size);
    
protected:
    lua_State *main_;
    lua_State *co_;
    bool manual_read_body_;
    bool one_off_;
    int32_t lua_callbacks_[eLuaHttpCallback_Count];
    HttpHeaderTable header_table_;
    uint32_t status_code_;
    bool resumed_;
    bool is_chunk_;
    size_t cur_chunk_size_;
    size_t cur_chunk_left_;
    LuaHttpReader::Ptr reader_;
    Logger logger_;
    int32_t get_callback_ = LUA_REFNIL;
    bool yielding_ = false;
};

} // namespace bee

#endif
