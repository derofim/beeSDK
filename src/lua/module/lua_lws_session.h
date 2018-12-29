#ifndef __LUA_LWS_SESSION_H__
#define __LUA_LWS_SESSION_H__

#include "lws_config.h"
#include "libwebsockets.h"
#include "utility/common.h"
#include "utility/json/json.h"
#include "utility/timer.h"
#include "log/logger.h"

extern "C" {
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
}

namespace bee {

typedef enum LuaLwsCallbackType {
    eLuaLwsCallback_Connect = 0,
    eLuaLwsCallback_Read,
    eLuaLwsCallback_Error,
    eLuaLwsCallback_Count
}LuaLwsCallbackType;

class LuaLwsService;
class LuaLwsSession : public std::enable_shared_from_this<LuaLwsSession> {
public:
    typedef std::shared_ptr<LuaLwsSession> Ptr;
    LuaLwsSession(
        IOSPtr ios, 
        std::shared_ptr<LuaLwsService> lws_service, 
        const std::string &protocol, 
        bool json_completed,
        int32_t read_callback, 
        int32_t error_callback, 
        lua_State *main, 
        lua_State *co);
    ~LuaLwsSession();

public:
    BeeErrorCode connect(const char *url, int32_t timeout, int32_t connect_callback);
    void         close();
    BeeErrorCode write(int32_t handle, const char *data);
    void         report_event(struct lws *wsi, lws_callback_reasons reason, void *in, size_t len);
    void         on_connected(lws *wsi);
    static       int32_t on_wait_result(lua_State *l, int32_t status, lua_KContext ctx);
    void         set_closing(bool closing) { closing_ = closing; }
    bool         is_closing() { return closing_; }
    void         set_closed(bool closed) { closed_ = closed; }
    bool         is_closed() {return closed_;}

protected:
    void    report_connection_result(struct lws *wsi, bool success, const std::string &data);
    void    report_disconnected(struct lws *wsi);
    void    report_read_data(const std::string &data);
    bool    get_completed_json_message(const std::string &data, std::string *&msg);
    void    on_event(struct lws *wsi, lws_callback_reasons reason, const std::string &data);
    void    handle_connect_timeout();

protected:
    IOSPtr ios_;
    struct lws *wsi_ = NULL;
    std::shared_ptr<LuaLwsService> lws_service_;
    std::string protocol_;
    bool check_json_completed_ = false;
    std::string current_msg_;
    lua_State *main_ = NULL;
    lua_State *co_ = NULL;
    bool waiting_ = false;
    int32_t lua_callbacks_[eLuaLwsCallback_Count];
    bool closing_ = false;
    bool closed_ = false;
    bool connected_ = false;
    Logger logger_;
    AsyncWaitTimer::Ptr connect_timer_;
    bool connect_reported = false;
};

} // namespace bee

#endif
