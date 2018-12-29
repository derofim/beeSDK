#include "lua_http_module.h"
#include "session/bee_session.h"

namespace bee {

LuaHttpModule::LuaHttpModule() {

}

LuaHttpModule::~LuaHttpModule() {
    clear();
}

int32_t LuaHttpModule::lua_open_module(lua_State *l) {
    luaL_Reg reg[] = {
        { "open",           LuaHttpModule::lua_open },     
        { "close",          LuaHttpModule::lua_close },  
        { "set_callback",   LuaHttpModule::lua_set_callback },
        { "get",            LuaHttpModule::lua_get },
        { "async_get",      LuaHttpModule::lua_async_get },
        { "post",           LuaHttpModule::lua_post },
        { "read_header",    LuaHttpModule::lua_read_header },
        { "read_body",      LuaHttpModule::lua_read_body },
        { "read",           LuaHttpModule::lua_read },
        { "scanf",          LuaHttpModule::lua_scanf },
        { NULL,             NULL }
    };

    std::string name = "http";
    std::string version = "1.0.1";
    return LuaModuleBase::open_module(l, reg, name, version);
}

int32_t LuaHttpModule::lua_open(lua_State *l) {
    if (!check_args(l, 0)) {
        return luaL_error(l, "Invalid args.");
    }

    ModuleWrapper<LuaHttpModule> *wrapper = get_module_wrapper(l);
    if (wrapper == NULL) {
        return luaL_error(l, "Wrapper NULL.");
    }

    lua_getglobal(l, "BEE_TABLE");
    lua_pushstring(l, "bee_session");
    lua_gettable(l, -2);

    BeeSession *s = (BeeSession *)lua_topointer(l, -1);
    if (s == NULL) {
        return luaL_error(l, "No session in table.");
    }

    uint32_t handle = wrapper->module->new_lua_obj(std::make_shared<LuaHttpSession>(s->get_ios()));
    lua_settop(l, 0);
    lua_pushinteger(l, (lua_Integer)handle);
    return 1;
}

int32_t LuaHttpModule::lua_close(lua_State *l) {
    if (!check_args(l, 1)) {
        return luaL_error(l, "Invalid args.");
    }

    ModuleWrapper<LuaHttpModule> *wrapper = get_module_wrapper(l);
    if (wrapper == NULL) {
        return luaL_error(l, "Wrapper NULL.");
    }

    int32_t handle_offset = -1;
    if (!lua_isinteger(l, handle_offset)) {
        return luaL_error(l, "Wrong param data type.");
    }

    int32_t handle = (int32_t)lua_tointeger(l, handle_offset);
    LuaHttpSession::Ptr http_session = wrapper->module->delete_lua_obj(handle);
    if (http_session == NULL) {
        return luaL_error(l, "Can't find http session from handle %d.", handle);
    }

    http_session->close();
    http_session.reset();
    return 0;
}

int32_t LuaHttpModule::lua_set_callback(lua_State *l) {
    if (!check_args(l, 3)) {
        return luaL_error(l, "Invalid args.");
    }

    ModuleWrapper<LuaHttpModule> *wrapper = get_module_wrapper(l);
    if (wrapper == NULL) {
        return luaL_error(l, "Wrapper NULL.");
    }

    int32_t handle_offset = -3;
    int32_t type_offset = -2;

    if (!lua_isinteger(l, handle_offset) || !lua_isinteger(l, type_offset)) {
        return luaL_error(l, "Wrong param data type.");
    }

    int32_t handle = (int32_t)lua_tointeger(l, handle_offset);
    LuaHttpSession::Ptr http_session = wrapper->module->get_obj(handle);
    if (http_session == NULL) {
        return luaL_error(l, "Can't find http session from handle %d.", handle);
    }

    int32_t type = (int32_t)lua_tointeger(l, type_offset);
    if (type < 0 || type >= eLuaHttpCallback_Count) {
        http_session.reset();
        return luaL_error(l, "Invalid http callback type %d.", type);
    }
    LuaHttpCallbackType etype = static_cast<LuaHttpCallbackType>(type);

    int32_t callback = (int32_t)luaL_ref(l, LUA_REGISTRYINDEX);
    if (callback == LUA_REFNIL) {
        http_session.reset();
        return luaL_error(l, "Http callback type %d NULL.", type);
    }

    http_session->set_lua_callback(etype, callback);
    http_session.reset();
    return 0;
}

int32_t LuaHttpModule::lua_get(lua_State *l) {
    if (!check_args(l, 3)) {
        return luaL_error(l, "Invalid args.");
    }

    ModuleWrapper<LuaHttpModule> *wrapper = get_module_wrapper(l);
    if (wrapper == NULL) {
        return luaL_error(l, "Wrapper NULL.");
    }

    int32_t handle_offset = -3;
    int32_t url_offset = -2;
    int32_t param_offset = -1;

    if (!lua_isinteger(l, handle_offset) || !lua_isstring(l, url_offset) || !lua_istable(l, param_offset)) {
        return luaL_error(l, "Wrong param data type.");
    }

    int32_t handle = (int32_t)lua_tointeger(l, handle_offset);
    LuaHttpSession::Ptr http_session = wrapper->module->get_obj(handle);
    if (http_session == NULL) {
        return luaL_error(l, "Can't find http session from handle %d.", handle);
    }

    const char *url = lua_tostring(l, url_offset);
    if (url == NULL) {
        http_session.reset();
        return luaL_error(l, "URL is NULL.");
    }

    std::string ref_url;
    int64_t range_start = -1;
    int64_t range_end = -1;
    int32_t timeout = -1;
    bool manual_read_body = false;
    bool one_off = true;
    bool valid = true;

    lua_pushnil(l);
    while (lua_next(l, -2) != 0) {
        do {
            if (!lua_isstring(l, -2)) {
                valid = false;
                break;
            }

            const char *name = lua_tostring(l, -2);
            if (name == NULL) {
                valid = false;
                break;
            }

            if (0 == strncmp(name, kHttpRangeStr.c_str(), kHttpRangeStr.size())) {
                if (!lua_isstring(l, -1)) {
                    valid = false;
                    break;
                }
                const char *value = lua_tostring(l, -1);
                if (value == NULL) {
                    valid = false;
                    break;
                }

                int32_t start = -1;
                int32_t end = -1;
                sscanf(value, "%d-%d", &start, &end);
                range_start = start;
                range_end = end;
                break;
            }

            if (0 == strncmp(name, kHttpTimeoutStr.c_str(), kHttpTimeoutStr.size())) {
                if (!lua_isinteger(l, -1)) {
                    valid = false;
                    break;
                }
                timeout = (int32_t)lua_tointeger(l, -1);
                break;
            }

            if (0 == strncmp(name, kManualReadBody.c_str(), kManualReadBody.size())) {
                if (!lua_isinteger(l, -1)) {
                    valid = false;
                    break;
                }
                manual_read_body = lua_tointeger(l, -1) == 0 ? false : true;
                break;
            }

            if (0 == strncmp(name, kOneOff.c_str(), kOneOff.size())) {
                if (!lua_isinteger(l, -1)) {
                    valid = false;
                    break;
                }
                one_off = lua_tointeger(l, -1) == 0 ? false : true;
                break;
            }
        } while (0);

        lua_pop(l, 1);

        if (!valid) {
            break;
        }
    }

    if (!valid) {
        http_session.reset();
        return luaL_error(l, "Invalid get param.");
    }

    BeeErrorCode ret = http_session->get(
        wrapper->module->get_main_thread(),
        l,
        url,
        manual_read_body,
        one_off,
        "",
        range_start,
        range_end);
    if (ret != kBeeErrorCode_Success) {
        http_session.reset();
        return luaL_error(l, "Http session get return error %d.", ret);
    }
    http_session.reset();

    //Before calling lua_yieldk, everything on stack must be released, as lua_yieldk will unwind stack by
    //calling longjump and everything on stack will be lost, object destructor may not be called.
    lua_settop(l, 0);
    return lua_yieldk(l, 0, 0, LuaHttpSession::http_get_result);
}

int32_t LuaHttpModule::lua_async_get(lua_State *l) {
    if (!check_args(l, 4)) {
        return luaL_error(l, "Invalid args.");
    }

    ModuleWrapper<LuaHttpModule> *wrapper = get_module_wrapper(l);
    if (wrapper == NULL) {
        return luaL_error(l, "Wrapper NULL.");
    }

    //Get callback at stack top first, luaL_ref will pop stack top.
    int32_t get_callback = (int32_t)luaL_ref(l, LUA_REGISTRYINDEX);

    int32_t handle_offset = 1;
    int32_t url_offset = 2;
    int32_t param_offset = 3;

    if (!lua_isinteger(l, handle_offset) || !lua_isstring(l, url_offset) || !lua_istable(l, param_offset)) {
        return luaL_error(l, "Wrong param data type.");
    }

    int32_t handle = (int32_t)lua_tointeger(l, handle_offset);
    LuaHttpSession::Ptr http_session = wrapper->module->get_obj(handle);
    if (http_session == NULL) {
        return luaL_error(l, "Can't find http session from handle %d.", handle);
    }

    const char *url = lua_tostring(l, url_offset);
    if (url == NULL) {
        http_session.reset();
        return luaL_error(l, "URL is NULL.");
    }

    std::string ref_url;
    int64_t range_start = -1;
    int64_t range_end = -1;
    int32_t timeout = -1;
    bool manual_read_body = false;
    bool one_off = true;
    bool valid = true;

    lua_pushnil(l);
    while (lua_next(l, -2) != 0) {
        do {
            if (!lua_isstring(l, -2)) {
                valid = false;
                break;
            }

            const char *name = lua_tostring(l, -2);
            if (name == NULL) {
                valid = false;
                break;
            }

            if (0 == strncmp(name, kHttpRangeStr.c_str(), kHttpRangeStr.size())) {
                if (!lua_isstring(l, -1)) {
                    valid = false;
                    break;
                }
                const char *value = lua_tostring(l, -1);
                if (value == NULL) {
                    valid = false;
                    break;
                }

                int32_t start = -1;
                int32_t end = -1;
                sscanf(value, "%d-%d", &start, &end);
                range_start = start;
                range_end = end;
                break;
            }

            if (0 == strncmp(name, kHttpTimeoutStr.c_str(), kHttpTimeoutStr.size())) {
                if (!lua_isinteger(l, -1)) {
                    valid = false;
                    break;
                }
                timeout = (int32_t)lua_tointeger(l, -1);
                break;
            }

            if (0 == strncmp(name, kManualReadBody.c_str(), kManualReadBody.size())) {
                if (!lua_isinteger(l, -1)) {
                    valid = false;
                    break;
                }
                manual_read_body = lua_tointeger(l, -1) == 0 ? false : true;
                break;
            }

            if (0 == strncmp(name, kOneOff.c_str(), kOneOff.size())) {
                if (!lua_isinteger(l, -1)) {
                    valid = false;
                    break;
                }
                one_off = lua_tointeger(l, -1) == 0 ? false : true;
                break;
            }
        } while (0);

        lua_pop(l, 1);

        if (!valid) {
            break;
        }
    }

    if (!valid) {
        http_session.reset();
        return luaL_error(l, "Invalid get param.");
    }

    BeeErrorCode ret = http_session->async_get(
        wrapper->module->get_main_thread(), 
        get_callback,
        url, 
        manual_read_body,
        one_off,
        "",
        range_start,
        range_end);
    http_session.reset();
    if (ret != kBeeErrorCode_Success) {
        return luaL_error(l, "Http session async_get return error %d.", ret);
    } else {
        return 0;
    }
}

int32_t LuaHttpModule::lua_post(lua_State *l) {
    return -1;
}

int32_t LuaHttpModule::lua_read_header(lua_State *l) {
    return -1;
}

int32_t LuaHttpModule::lua_read_body(lua_State *l) {
    return -1;
}

int32_t LuaHttpModule::lua_read(lua_State *l) {
    if (!check_args(l, 4)) {
        return luaL_error(l, "Invalid args.");
    }

    ModuleWrapper<LuaHttpModule> *wrapper = get_module_wrapper(l);
    if (wrapper == NULL) {
        return luaL_error(l, "Wrapper NULL.");
    }

    int32_t handle_offset = -4;
    int32_t buffer_offset = -3;
    int32_t bufflen_offset = -2;
    int32_t condition_offset = -1;

    if (!lua_isinteger(l, handle_offset) ||
        !lua_islightuserdata(l, buffer_offset) ||
        !lua_isinteger(l, bufflen_offset) ||
        !lua_isinteger(l, condition_offset)) {
        return luaL_error(l, "Wrong param data type.");
    }

    int32_t handle = (int32_t)lua_tointeger(l, handle_offset);
    LuaHttpSession::Ptr http_session = wrapper->module->get_obj(handle);
    if (http_session == NULL) {
        return luaL_error(l, "Can't find http session from handle %d.", handle);
    }

    uint8_t *buf = (uint8_t *)lua_topointer(l, buffer_offset);
    uint32_t len = (uint32_t)lua_tointeger(l, bufflen_offset);
    int32_t cond = (int32_t)lua_tointeger(l, condition_offset);

    BeeErrorCode ret = http_session->read(l, buf, len, cond);
    if (ret != kBeeErrorCode_Success) {
        http_session.reset();
        return luaL_error(l, "Http session read fail, error %d.", ret);
    }
    http_session.reset();
    lua_settop(l, 0);
    return lua_yieldk(l, 0, 0, LuaHttpReader::on_wait_result); //Pass nothing to lua_resume
}


int32_t LuaHttpModule::lua_scanf(lua_State *l) {
    if (!check_args(l, 2)) {
        return luaL_error(l, "Invalid args.");
    }

    ModuleWrapper<LuaHttpModule> *wrapper = get_module_wrapper(l);
    if (wrapper == NULL) {
        return luaL_error(l, "Wrapper NULL.");
    }

    int32_t handle_offset = -2;
    int32_t format_offset = -1;

    if (!lua_isinteger(l, handle_offset) || !lua_istable(l, format_offset)) {
        return luaL_error(l, "Wrong param data type.");
    }

    int32_t handle = (int32_t)lua_tointeger(l, handle_offset);
    LuaHttpSession::Ptr http_session = wrapper->module->get_obj(handle);
    if (http_session == NULL) {
        return luaL_error(l, "Can't find http session from handle %d.", handle);
    }

    DataField field;
    std::vector<DataField> format;
    bool valid = true;
    size_t total_size = 0;

    lua_pushnil(l);
    while (lua_next(l, -2) != 0) {
        if (!lua_istable(l, -1)) {
            valid = false;
            break;
        }

        lua_pushnil(l);
        while (lua_next(l, -2) != 0) {
            if (!lua_isstring(l, -2)) {
                valid = false;
                break;
            }

            const char *key = lua_tostring(l, -2);
            if (key == NULL) {
                valid = false;
                break;
            }

            if (strcmp(key, "typeid") == 0) {
                if (!lua_isinteger(l, -1)) {
                    valid = false;
                    break;
                }
                int32_t type = (int32_t)lua_tointeger(l, -1);
                if (type < 0 || type >= eDataType_count) {
                    valid = false;
                    break;
                }

                field.type = static_cast<DataTypeId>(type);
            }
            else if (strcmp(key, "length") == 0) {
                if (!lua_isinteger(l, -1)) {
                    valid = false;
                    break;
                }
                size_t length = (size_t)lua_tointeger(l, -1);
                if (length == 0) {
                    valid = false;
                    break;
                }
                field.length = length;
            }
            else if (strcmp(key, "hton") == 0) {
                if (!lua_isboolean(l, -1)) {
                    valid = false;
                    break;
                }
                field.hton = lua_toboolean(l, -1) == 0 ? false : true;
            }
            else {
                valid = false;
                break;
            }

            lua_pop(l, 1);
        }

        total_size += (kDataTypeSize[field.type] * field.length);
        format.push_back(field);
        lua_pop(l, 1);
    }

    if (!valid) {
        http_session.reset();
        return luaL_error(l, "Invalid get param.");
    }

    BeeErrorCode ret = http_session->scanf(l, format, total_size);
    if (ret != kBeeErrorCode_Success) {
        http_session.reset();
        return luaL_error(l, "Http session read fail, error %d.", ret);
    }
    http_session.reset();
    lua_settop(l, 0);
    return lua_yieldk(l, 0, 0, LuaHttpReader::on_wait_result); //Pass nothing to lua_resume
}

void LuaHttpModule::clear() {
    LuaObjTable::iterator iter = obj_table_.begin();
    for (;iter != obj_table_.end();++iter) {
        std::shared_ptr<LuaObj<LuaHttpSession> > lua_obj = iter->second;
        if (lua_obj != NULL) {
            LuaHttpSession::Ptr http_session = lua_obj->obj;
            if (http_session != NULL) {
                http_session->close();
            }
        }
    }
    obj_table_.clear();
}

} // namespace bee
