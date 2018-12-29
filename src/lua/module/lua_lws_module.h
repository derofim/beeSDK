#ifndef __LUA_LWS_MODULE_H__
#define __LUA_LWS_MODULE_H__

#include "lua_module_mgr.h"
#include "lua_lws_session.h"
namespace bee {

////////////////////////////////////LuaLwsModule//////////////////////////////////////
class LuaLwsService;
class LuaLwsModule : public LuaObjMgr<LuaLwsSession>, public LuaModuleBase<LuaLwsModule> {
public:
    LuaLwsModule();
    virtual ~LuaLwsModule();

public:
    static int32_t lua_open_module(lua_State *l);
    static int32_t lua_start(lua_State *l);
    static int32_t lua_open(lua_State *l);
    static int32_t lua_close(lua_State *l);
    static int32_t lua_connect(lua_State *l);
    static int32_t lua_write(lua_State *l);

public:
    void on_close_module();
    bool is_running();

protected:
    std::shared_ptr<LuaLwsService> lws_service_;
    IOSPtr ios_;
};

} // namespace bee

#endif
