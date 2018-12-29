#ifndef __LUA_IOBUFFER_MODULE_H__
#define __LUA_IOBUFFER_MODULE_H__

#include "lua_module_mgr.h"
#include "utility/iobuffer.h"

namespace bee {

////////////////////////////////////LuaIOBufferModule//////////////////////////////////////
class LuaIOBufferModule : public LuaObjMgr<IOBuffer>, public LuaModuleBase<LuaIOBufferModule>{
public:
    LuaIOBufferModule();
    ~LuaIOBufferModule();

public:
    static int32_t lua_open_module(lua_State *l);
    static int32_t lua_new(lua_State *l);
    static int32_t lua_delete(lua_State *l);
    static int32_t lua_get_buffer(lua_State *l);
    static int32_t lua_get_ptr(lua_State *l);
    static int32_t lua_get_size(lua_State *l);
    static int32_t lua_get_capacity(lua_State *l);
    static int32_t lua_get_free_space(lua_State *l);
    static int32_t lua_is_empty(lua_State *l);
    static int32_t lua_is_full(lua_State *l);
    static int32_t lua_resize(lua_State *l); //Reserve data
    static int32_t lua_reset(lua_State *l); //Clear data
    static int32_t lua_clear(lua_State *l);
    static int32_t lua_consume(lua_State *l);
    static int32_t lua_produce(lua_State *l);
    static int32_t lua_align(lua_State *l);
    static int32_t lua_read(lua_State *l);

protected:
    static std::shared_ptr<IOBuffer> get_iobuffer(lua_State *l);
};

} // namespace bee

#endif
