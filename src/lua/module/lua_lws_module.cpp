#include "lua_lws_module.h"
#include "lua_lws_service.h"
#include "bee/base/bee_define.h"
#include "session/bee_session.h"

namespace bee {

LuaLwsModule::LuaLwsModule() {

}

LuaLwsModule::~LuaLwsModule() {

}

int32_t LuaLwsModule::lua_open_module(lua_State *l) {
    luaL_Reg reg[] = {
        { "start",          LuaLwsModule::lua_start }, 
        { "open",           LuaLwsModule::lua_open },     
        { "close",          LuaLwsModule::lua_close },
        { "connect",        LuaLwsModule::lua_connect },
        { "write",          LuaLwsModule::lua_write },
        { NULL,             NULL }
    };

    std::string name = "lws";
    std::string version = "2.3.0";
    return LuaModuleBase::open_module(l, reg, name, version);
}

int32_t LuaLwsModule::lua_start(lua_State *l) {
    if (!check_args(l, 1)) {
        return luaL_error(l, "Invalid args.");
    }

    ModuleWrapper<LuaLwsModule> *wrapper = get_module_wrapper(l);
    if (wrapper == NULL) {
        return luaL_error(l, "Wrapper NULL.");
    }

    if (wrapper->module->is_running()) {
        return 0;
    }

    if (!lua_istable(l, -1)) {
        return luaL_error(l, "Param is not table.");
    }

    std::vector<std::string> protocol_vec;
    lua_pushnil(l);
    while (lua_next(l, -2) != 0) {
        const char *value = lua_tostring(l, -1);
        if (value == NULL) {
            break;
        }
        protocol_vec.push_back(value);
        lua_pop(l, 1);
    }

    wrapper->module->lws_service_.reset(new LuaLwsService(wrapper->module));
    BeeErrorCode ret = wrapper->module->lws_service_->start(protocol_vec);
    if (ret != kBeeErrorCode_Success) {
        return luaL_error(l, "Lws service start error %d.", ret);
    }

    lua_getglobal(l, "BEE_TABLE");
    lua_pushstring(l, "bee_session");
    lua_gettable(l, -2);

    BeeSession *s = (BeeSession *)lua_topointer(l, -1);
    if (s == NULL) {
        return luaL_error(l, "No session in table.");
    }

    wrapper->module->ios_ = s->get_ios();
    return 0;
}

int32_t LuaLwsModule::lua_open(lua_State *l) {
    if (!check_args(l, 4)) {
        return luaL_error(l, "Invalid args.");
    }

    ModuleWrapper<LuaLwsModule> *wrapper = get_module_wrapper(l);
    if (wrapper == NULL) {
        return luaL_error(l, "Wrapper NULL.");
    }

    if (!wrapper->module->is_running()) {
        return luaL_error(l, "Lws service not running.");
    }

    int32_t protocol_offset = 1;
    int32_t json_completed_offset = 2;
    if (!lua_isstring(l, protocol_offset) || !lua_isboolean(l, json_completed_offset)) {
        return luaL_error(l, "Wrong param data type.");
    }

    std::string protocol;
    const char *proto = lua_tostring(l, protocol_offset);
    if (proto != NULL) {
        protocol = proto;
    }

    bool json_completed = lua_toboolean(l, json_completed_offset) == 0 ? false : true;

    //Lua push parameters into stack from left to right, so error_callback on stack top with index -1(5).
    int32_t error_callback = (int32_t)luaL_ref(l, LUA_REGISTRYINDEX);
    int32_t read_callback = (int32_t)luaL_ref(l, LUA_REGISTRYINDEX);

    uint32_t handle = wrapper->module->new_lua_obj(
        std::make_shared<LuaLwsSession>(
            wrapper->module->ios_, 
            wrapper->module->lws_service_, 
            protocol, 
            json_completed,
            read_callback, 
            error_callback, 
            wrapper->module->get_main_thread(), 
            l));
    lua_settop(l, 0);
    lua_pushinteger(l, (lua_Integer)handle);
    return 1;
}

int32_t LuaLwsModule::lua_close(lua_State *l) {
    if (!check_args(l, 1)) {
        return luaL_error(l, "Invalid args.");
    }

    ModuleWrapper<LuaLwsModule> *wrapper = get_module_wrapper(l);
    if (wrapper == NULL) {
        return luaL_error(l, "Wrapper NULL.");
    }

    if (!wrapper->module->is_running()) {
        return luaL_error(l, "Lws service not running.");
    }

    int32_t handle_offset = -1;
    if (!lua_isinteger(l, handle_offset)) {
        return luaL_error(l, "Wrong param data type.");
    }

    int32_t handle = (int32_t)lua_tointeger(l, handle_offset);
    LuaLwsSession::Ptr lws_session = wrapper->module->delete_lua_obj(handle);
    if (lws_session == NULL) {
        return luaL_error(l, "Can't find lws session from handle %d.", handle);
    }

    lws_session->close();
    lws_session.reset();
    return 0;
}

int32_t LuaLwsModule::lua_connect(lua_State *l) {
    if (!check_args(l, 4)) {
        return luaL_error(l, "Invalid args.");
    }

    ModuleWrapper<LuaLwsModule> *wrapper = get_module_wrapper(l);
    if (wrapper == NULL) {
        return luaL_error(l, "Wrapper NULL.");
    }

    if (!wrapper->module->is_running()) {
        return luaL_error(l, "Lws service not running.");
    }

    int32_t handle_offset = 1;
    int32_t url_offset = 2;
    int32_t timeout_offset = 3;
    if (!lua_isinteger(l, handle_offset) || !lua_isstring(l, url_offset) || !lua_isinteger(l, timeout_offset)) {
        return luaL_error(l, "Wrong param data type.");
    }

    const char *url = (const char*)lua_tostring(l, url_offset);
    int32_t timeout = (int32_t)lua_tointeger(l, timeout_offset);
    int32_t handle = (int32_t)lua_tointeger(l, handle_offset);
    int32_t connect_callback = (int32_t)luaL_ref(l, LUA_REGISTRYINDEX);
    LuaLwsSession::Ptr lws_session = wrapper->module->get_obj(handle);
    if (lws_session == NULL) {
        return luaL_error(l, "Can't find lws session from handle %d.", handle);
    }

    BeeErrorCode ret = lws_session->connect(url, timeout, connect_callback);
    if (ret != kBeeErrorCode_Success) {
        lws_session.reset();
        return luaL_error(l, "Lws session connect fail, error %d.", ret);
    }

    lws_session.reset();
    return 0;
}

int32_t LuaLwsModule::lua_write(lua_State *l) {
    if (!check_args(l, 2)) {
        return luaL_error(l, "Invalid args.");
    }

    ModuleWrapper<LuaLwsModule> *wrapper = get_module_wrapper(l);
    if (wrapper == NULL) {
        return luaL_error(l, "Wrapper NULL.");
    }

    if (!wrapper->module->is_running()) {
        return luaL_error(l, "Lws service not running.");
    }

    int32_t handle_offset = 1;
    int32_t data_offset = 2;
    if (!lua_isinteger(l, handle_offset) || !lua_isstring(l, data_offset)) {
        return luaL_error(l, "Wrong param data type.");
    }

    const char *data = (const char*)lua_tostring(l, data_offset);
    int32_t handle = (int32_t)lua_tointeger(l, handle_offset);
    LuaLwsSession::Ptr lws_session = wrapper->module->get_obj(handle);
    if (lws_session == NULL) {
        return luaL_error(l, "Can't find lws session from handle %d.", handle);
    }

    BeeErrorCode ret = lws_session->write(handle, data);
    if (ret != kBeeErrorCode_Success) {
        lws_session.reset();
        return luaL_error(l, "Lws session write fail, error %d.", ret);
    }

    lws_session.reset();
    return 0;
}

void LuaLwsModule::on_close_module() {
    if (!obj_table_.empty()) {
        auto iter = obj_table_.begin();
        for (; iter != obj_table_.end(); ++iter) {
            iter->second->obj->close();
        }
    }

    if (lws_service_ != NULL) {
        lws_service_->stop();
        lws_service_.reset();
    }

    if (ios_ != NULL) {
        ios_->reset();
    }
}

bool LuaLwsModule::is_running() {
    return lws_service_ != NULL && lws_service_->is_running();
}

} // namespace bee
