#include "lua_timer_module.h"
#include "session/bee_session.h"

namespace bee {

LuaTimerModule::LuaTimerModule() {

}

LuaTimerModule::~LuaTimerModule() {

}

int32_t LuaTimerModule::lua_open_module(lua_State *l) {
    luaL_Reg reg[] = {
        { "open",           LuaTimerModule::lua_open },
        { "close",          LuaTimerModule::lua_close },
        { NULL,             NULL }
    };

    std::string name = "timer";
    std::string version = "1.0.0.1";
    return LuaModuleBase::open_module(l, reg, name, version);
}

int32_t LuaTimerModule::lua_open(lua_State *l) {
    if (!check_args(l, 3)) {
        return luaL_error(l, "Invalid args.");
    }

    ModuleWrapper<LuaTimerModule> *wrapper = get_module_wrapper(l);
    if (wrapper == NULL) {
        return luaL_error(l, "Wrapper NULL.");
    }

    int32_t interval_offset = 1;
    int32_t timeout_count_offset = 2;
    if (!lua_isinteger(l, interval_offset) || !lua_isinteger(l, timeout_count_offset)) {
        return luaL_error(l, "Wrong param data type.");
    }
    int32_t interval = (int32_t)lua_tointeger(l, interval_offset);
    int32_t timeout_count = (int32_t)lua_tointeger(l, timeout_count_offset);

    int32_t callback = (int32_t)luaL_ref(l, LUA_REGISTRYINDEX);
    if (callback == LUA_REFNIL) {
        return luaL_error(l, "Timer callback NULL.");
    }

    lua_getglobal(l, "BEE_TABLE");
    lua_pushstring(l, "bee_session");
    lua_gettable(l, -2);

    BeeSession *s = (BeeSession *)lua_topointer(l, -1);
    if (s == NULL) {
        return luaL_error(l, "No session in table.");
    }

    std::shared_ptr<LuaTimer> timer(new LuaTimer(wrapper->module->get_main_thread()));
    if (!timer->open(s->get_ios(), interval, timeout_count, callback)) {
        return luaL_error(l, "Inner timer open fail.");
    }

    uint32_t handle = wrapper->module->new_lua_obj(timer);
    lua_settop(l, 0);
    lua_pushinteger(l, (lua_Integer)handle);

    return 1;
}
int32_t LuaTimerModule::lua_close(lua_State *l) {
    if (!check_args(l, 1)) {
        return luaL_error(l, "Invalid args.");
    }

    ModuleWrapper<LuaTimerModule> *wrapper = get_module_wrapper(l);
    if (wrapper == NULL) {
        return luaL_error(l, "Wrapper NULL.");
    }

    int32_t handle_offset = -1;
    if (!lua_isinteger(l, handle_offset)) {
        return luaL_error(l, "Wrong param data type.");
    }

    int32_t handle = (int32_t)lua_tointeger(l, handle_offset);
    LuaTimer::Ptr timer = wrapper->module->delete_lua_obj(handle);
    if (timer == NULL) {
        return luaL_error(l, "Can't find http session from handle %d.", handle);
    }

    timer->close();
    return 0;
}

void LuaTimerModule::on_close_module() {
    if (!obj_table_.empty()) {
        auto iter = obj_table_.begin();
        for (; iter != obj_table_.end(); ++iter) {
            iter->second->obj->close();
        }
    }
}

} // namespace bee
