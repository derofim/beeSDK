#ifndef __LUA_VIDEO_CACHE_H__
#define __LUA_VIDEO_CACHE_H__

#include "lua_data_promise.h"
#include "lua_module_mgr.h"

namespace bee {

////////////////////////////////////LuaVideoCache//////////////////////////////////////
class LuaVideoCache : public LuaModuleBase<LuaVideoCache>{
public:
    LuaVideoCache();
    ~LuaVideoCache();

public:
    uint32_t        WriteData(const uint8_t *buf, uint32_t len);
    uint32_t        ReadData(uint8_t *buf, uint32_t len);
    uint32_t        PromiseWriteData(const uint8_t *buf, uint32_t len);
    void            PromiseReadData(DataPromise::Ptr promise);
    void            MakePromise(DataPromise::Ptr promise);
    void            ClearData();
    uint32_t        SkipData(uint32_t size);
    void            ResetSize(uint32_t size);
    uint32_t        GetSpaceSize(uint32_t expect_size);
    uint32_t        GetDataSize();
    lua_State*      GetMainThread(){return main_thread_;}
    int32_t         Scanf(lua_State *l, const std::vector<DataField> &format);

    static int32_t  LuaOpenVideoCache(lua_State *l);
    static int32_t  LuaWriteData(lua_State *l);
    static int32_t  LuaReadData(lua_State *l);
    static int32_t  LuaClearData(lua_State *l);
    static int32_t  LuaSkipData(lua_State *l);
    static int32_t  LuaResetSize(lua_State *l);
    static int32_t  LuaGetSpaceSize(lua_State *l);
    static int32_t  LuaGetDataSize(lua_State *l);
    static int32_t  LuaVersion(lua_State *l);
    static int32_t  LuaGetCache(lua_State *l);
    static int32_t  LuaSyncReadData(lua_State *l);
    static int32_t  LuaScanf(lua_State *l);
    static int32_t  LuaWaitUntil(lua_State *l);

public:
    uint8_t  *buffer_;
    uint32_t use_;
    uint32_t tail_;
    uint32_t size_;

    const std::string ver_;
    DataPromise::Ptr  data_promice_;
};

} // namespace bee

#endif // __LUA_VIDEO_CACHE_H__
