#ifndef __LUA_TIMER_MODULE_H__
#define __LUA_TIMER_MODULE_H__

#include "lua_module_mgr.h"
#include "lua_timer.h"

namespace bee {

////////////////////////////////////LuaTimerModule//////////////////////////////////////
class LuaTimerModule : public LuaObjMgr<LuaTimer>, public LuaModuleBase<LuaTimerModule> {
public:
    LuaTimerModule();
    ~LuaTimerModule();

public:
    static int32_t lua_open_module(lua_State *l);
    static int32_t lua_open(lua_State *l);
    static int32_t lua_close(lua_State *l);

public:
    void on_close_module();
};

} // namespace bee

#endif
