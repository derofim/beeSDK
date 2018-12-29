#include "lua_iobuffer_module.h"

namespace bee {

LuaIOBufferModule::LuaIOBufferModule() {

}

LuaIOBufferModule::~LuaIOBufferModule() {

}

int32_t LuaIOBufferModule::lua_open_module(lua_State *l) {
    luaL_Reg reg[] = {
        { "new",            LuaIOBufferModule::lua_new },     
        { "delete",         LuaIOBufferModule::lua_delete },  
        { "get",            LuaIOBufferModule::lua_get_buffer },
        { "ptr",            LuaIOBufferModule::lua_get_ptr },
        { "size",           LuaIOBufferModule::lua_get_size },
        { "capacity",       LuaIOBufferModule::lua_get_capacity },
        { "freespace",      LuaIOBufferModule::lua_get_free_space },
        { "empty",          LuaIOBufferModule::lua_is_empty },
        { "full",           LuaIOBufferModule::lua_is_full },
        { "resize",         LuaIOBufferModule::lua_resize },
        { "reset",          LuaIOBufferModule::lua_reset },
        { "clear",          LuaIOBufferModule::lua_clear },
        { "consume",        LuaIOBufferModule::lua_consume },
        { "produce",        LuaIOBufferModule::lua_produce },
        { "align",          LuaIOBufferModule::lua_align },
        { "read",           LuaIOBufferModule::lua_read },
        { NULL,             NULL }
    };

    std::string name = "iobuffer";
    std::string version = "1.0.1";
    return LuaModuleBase::open_module(l, reg, name, version);
}

int32_t LuaIOBufferModule::lua_new(lua_State *l) {
    int32_t ret = -1;
    do {
        if (!check_args(l, 1)) {
            break;
        }

        ModuleWrapper<LuaIOBufferModule> *wrapper = get_module_wrapper(l);
        if (wrapper == NULL) {
            break;
        }

        int32_t size_offset = -1;
        if (!lua_isinteger(l, size_offset)) {
            break;
        }

        int32_t capacity = (int32_t)lua_tointeger(l, size_offset);
        if (capacity <= 0) {
            break;
        }

        uint32_t handle = wrapper->module->new_lua_obj(std::make_shared<IOBuffer>(capacity));
        lua_settop(l, 0);
        lua_pushinteger(l, handle);
        ret = 1;
    } while (0);
    return ret;
}

int32_t LuaIOBufferModule::lua_delete(lua_State *l) {
    int32_t ret = -1;
    do {
        if (!check_args(l, 1)) {
            break;
        }

        ModuleWrapper<LuaIOBufferModule> *wrapper = get_module_wrapper(l);
        if (wrapper == NULL) {
            break;
        }

        int32_t handle_offset = -1;
        if (!lua_isinteger(l, handle_offset)) {
            break;
        }

        uint32_t handle = (uint32_t)lua_tointeger(l, handle_offset);
        wrapper->module->delete_lua_obj(handle);
        ret = 0;
    } while (0);
    return ret;
}

int32_t LuaIOBufferModule::lua_get_buffer(lua_State *l) {
    int32_t ret = -1;
    do {
        if (!check_args(l, 1)) {
            break;
        }

        std::shared_ptr<IOBuffer> buffer = get_iobuffer(l);
        if (buffer == NULL) {
            break;
        }

        lua_settop(l, 0);
        lua_pushlightuserdata(l, (void*)buffer->data());
        lua_pushinteger(l, buffer->size());
        ret = 2;
    } while (0);
    return ret;
}

int32_t LuaIOBufferModule::lua_get_ptr(lua_State *l) {
    int32_t ret = -1;
    do {
        if (!check_args(l, 1)) {
            break;
        }

        std::shared_ptr<IOBuffer> buffer = get_iobuffer(l);
        if (buffer == NULL) {
            break;
        }

        lua_settop(l, 0);
        lua_pushlightuserdata(l, (void*)buffer->data());
        ret = 1;
    } while (0);
    return ret;
}

int32_t LuaIOBufferModule::lua_get_size(lua_State *l) {
    int32_t ret = -1;
    do {
        if (!check_args(l, 1)) {
            break;
        }

        std::shared_ptr<IOBuffer> buffer = get_iobuffer(l);
        if (buffer == NULL) {
            break;
        }

        lua_settop(l, 0);
        lua_pushinteger(l, buffer->size());
        ret = 1;
    } while (0);
    return ret;
}

int32_t LuaIOBufferModule::lua_get_capacity(lua_State *l) {
    int32_t ret = -1;
    do {
        if (!check_args(l, 1)) {
            break;
        }

        std::shared_ptr<IOBuffer> buffer = get_iobuffer(l);
        if (buffer == NULL) {
            break;
        }

        lua_settop(l, 0);
        lua_pushinteger(l, buffer->capacity());
        ret = 1;
    } while (0);
    return ret;
}

int32_t LuaIOBufferModule::lua_get_free_space(lua_State *l) {
    int32_t ret = -1;
    do {
        if (!check_args(l, 1)) {
            break;
        }

        std::shared_ptr<IOBuffer> buffer = get_iobuffer(l);
        if (buffer == NULL) {
            break;
        }

        lua_settop(l, 0);
        lua_pushinteger(l, buffer->freespace());
        ret = 1;
    } while (0);
    return ret;
}

int32_t LuaIOBufferModule::lua_is_empty(lua_State *l) {
    int32_t ret = -1;
    do {
        if (!check_args(l, 1)) {
            break;
        }

        std::shared_ptr<IOBuffer> buffer = get_iobuffer(l);
        if (buffer == NULL) {
            break;
        }

        lua_settop(l, 0);
        lua_pushboolean(l, buffer->empty());
        ret = 1;
    } while (0);
    return ret;
}

int32_t LuaIOBufferModule::lua_is_full(lua_State *l) {
    int32_t ret = -1;
    do {
        if (!check_args(l, 1)) {
            break;
        }

        std::shared_ptr<IOBuffer> buffer = get_iobuffer(l);
        if (buffer == NULL) {
            break;
        }

        lua_settop(l, 0);
        lua_pushboolean(l, buffer->full());
        ret = 1;
    } while (0);
    return ret;
}

int32_t LuaIOBufferModule::lua_resize(lua_State *l) {
    int32_t ret = -1;
    do {
        if (!check_args(l, 2)) {
            break;
        }

        ModuleWrapper<LuaIOBufferModule> *wrapper = get_module_wrapper(l);
        if (wrapper == NULL) {
            break;
        }

        int32_t handle_offset = 1;
        if (!lua_isinteger(l, handle_offset)) {
            break;
        }

        uint32_t handle = (uint32_t)lua_tointeger(l, handle_offset);
        std::shared_ptr<IOBuffer> buffer = wrapper->module->get_obj(handle);
        if (buffer == NULL) {
            break;
        }

        int32_t size_offset = 2;
        if (!lua_isinteger(l, size_offset)) {
            break;
        }

        int32_t new_size = (int32_t)lua_tointeger(l, size_offset);
        if (new_size < 0) {
            break;
        }

        size_t unew_size = (size_t)new_size;
        size_t cur_capacity = buffer->capacity();
        if (unew_size > cur_capacity) {
            std::shared_ptr<IOBuffer> new_buff(new IOBuffer(unew_size));
            memcpy(new_buff->data(), buffer->data(), buffer->size());
            new_buff->resize(unew_size);
            wrapper->module->set_lua_obj(handle, new_buff);
        } else {
            buffer->resize(unew_size);
        }

        ret = 0;
    } while (0);
    return ret;
}

int32_t LuaIOBufferModule::lua_reset(lua_State *l) {
    int32_t ret = -1;
    do {
        if (!check_args(l, 2)) {
            break;
        }

        ModuleWrapper<LuaIOBufferModule> *wrapper = get_module_wrapper(l);
        if (wrapper == NULL) {
            break;
        }

        int32_t handle_offset = 1;
        if (!lua_isinteger(l, handle_offset)) {
            break;
        }

        uint32_t handle = (uint32_t)lua_tointeger(l, handle_offset);
        std::shared_ptr<IOBuffer> buffer = wrapper->module->get_obj(handle);
        if (buffer == NULL) {
            break;
        }

        int32_t size_offset = 2;
        if (!lua_isinteger(l, size_offset)) {
            break;
        }

        int32_t new_size = (int32_t)lua_tointeger(l, size_offset);
        if (new_size < 0) {
            break;
        }

        size_t unew_size = (size_t)new_size;
        size_t cur_capacity = buffer->capacity();
        if (unew_size > cur_capacity) {
            std::shared_ptr<IOBuffer> new_buff(new IOBuffer(unew_size));
            wrapper->module->set_lua_obj(handle, new_buff);
        } else {
            buffer->clear();
            buffer->resize(unew_size);
        }

        ret = 0;
    } while (0);
    return ret;
}

int32_t LuaIOBufferModule::lua_clear(lua_State *l) {
    int32_t ret = -1;
    do {
        if (!check_args(l, 1)) {
            break;
        }

        std::shared_ptr<IOBuffer> buffer = get_iobuffer(l);
        if (buffer == NULL) {
            break;
        }

        buffer->clear();
        ret = 0;
    } while (0);
    return ret;
}

int32_t LuaIOBufferModule::lua_consume(lua_State *l) {
    int32_t ret = -1;
    do {
        if (!check_args(l, 2)) {
            break;
        }

        std::shared_ptr<IOBuffer> buffer = get_iobuffer(l);
        if (buffer == NULL) {
            break;
        }

        int32_t size_offset = 2;
        if (!lua_isinteger(l, size_offset)) {
            break;
        }

        int32_t size = (int32_t)lua_tointeger(l, size_offset);
        if (size < 0 || size > (int32_t)buffer->size()) {
            break;
        }

        buffer->consume((size_t)size);
        ret = 0;
    } while (0);
    return ret;
}

int32_t LuaIOBufferModule::lua_produce(lua_State *l) {
    int32_t ret = -1;
    do {
        if (!check_args(l, 2)) {
            break;
        }

        std::shared_ptr<IOBuffer> buffer = get_iobuffer(l);
        if (buffer == NULL) {
            break;
        }

        int32_t size_offset = 2;
        if (!lua_isinteger(l, size_offset)) {
            break;
        }

        int32_t size = (int32_t)lua_tointeger(l, size_offset);
        if (size < 0 || size > (int32_t)buffer->freespace()) {
            break;
        }

        buffer->produce((size_t)size);
        ret = 0;
    } while (0);
    return ret;
}

int32_t LuaIOBufferModule::lua_align(lua_State *l) {
    int32_t ret = -1;
    do {
        if (!check_args(l, 2)) {
            break;
        }

        std::shared_ptr<IOBuffer> buffer = get_iobuffer(l);
        if (buffer == NULL) {
            break;
        }

        buffer->align();
        ret = 0;
    } while (0);
    return ret;
}

int32_t LuaIOBufferModule::lua_read(lua_State *l) {
    int32_t ret = -1;
    do {
        if (!check_args(l, 3)) {
            break;
        }

        std::shared_ptr<IOBuffer> buffer = get_iobuffer(l);
        if (buffer == NULL) {
            break;
        }

        int32_t buffer_offset = 2;
        int32_t bufflen_offset = 3;
        if (!lua_islightuserdata(l, buffer_offset) || !lua_isinteger(l, bufflen_offset)) {
            break;
        }

        uint8_t *dst = (uint8_t *)lua_topointer(l, buffer_offset);
        if (dst == NULL) {
            break;
        }

        uint32_t len = (uint32_t)lua_tointeger(l, bufflen_offset);
        if (len == 0) {
            break;
        }

        size_t read_len = std::min<size_t>(len, buffer->size());
        memcpy(dst, buffer->data(), read_len);
        buffer->consume(read_len);

        lua_settop(l, 0);
        lua_pushinteger(l, read_len);
        ret = 1;
    } while (0);
    return ret;
}

std::shared_ptr<IOBuffer> LuaIOBufferModule::get_iobuffer(lua_State *l) {
    std::shared_ptr<IOBuffer> ret;
    do {
        if (l == NULL) {
            break;
        }

        ModuleWrapper<LuaIOBufferModule> *wrapper = get_module_wrapper(l);
        if (wrapper == NULL) {
            break;
        }

        int32_t handle_offset = 1;
        if (!lua_isinteger(l, handle_offset)) {
            break;
        }

        uint32_t handle = (uint32_t)lua_tointeger(l, handle_offset);
        ret = wrapper->module->get_obj(handle);
    } while (0);
    return ret;
}

} // namespace bee
