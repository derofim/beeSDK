#ifndef __LUA_HTTP_MODULE_H__
#define __LUA_HTTP_MODULE_H__

#include "lua_module_mgr.h"
#include "lua_http_session.h"

namespace bee {

////////////////////////////////////LuaHttpModule//////////////////////////////////////
class LuaHttpModule : public LuaObjMgr<LuaHttpSession>, public LuaModuleBase<LuaHttpModule> {
public:
    LuaHttpModule();
    virtual ~LuaHttpModule();

public:
    static int32_t lua_open_module(lua_State *l);
    static int32_t lua_open(lua_State *l);
    static int32_t lua_close(lua_State *l); 
    static int32_t lua_set_callback(lua_State *l);
    static int32_t lua_get(lua_State *l);
    static int32_t lua_async_get(lua_State *l);
    static int32_t lua_post(lua_State *l);
    static int32_t lua_read_header(lua_State *l);
    static int32_t lua_read_body(lua_State *l);
    static int32_t lua_read(lua_State *l);
    static int32_t lua_scanf(lua_State *l);

protected:
    void clear();
};

} // namespace bee

#endif
