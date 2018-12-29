#include "lua_video_cache.h"
#include "session/bee_session.h"
#include "utility/algorithm.h"
#include "utility/iobuffer.h"

namespace bee {
/////////////////////////////////////LuaVideoCache/////////////////////////////////////
LuaVideoCache::LuaVideoCache(): 
    buffer_(NULL),
    use_(0),
    tail_(0),
    size_(0) {
    size_ = 2097152;
    buffer_ = new uint8_t[2097152];
}

LuaVideoCache::~LuaVideoCache() {
    if ( buffer_ ) {
        delete [] buffer_;
        buffer_ = NULL;
    }
}
  
uint32_t LuaVideoCache::WriteData(const uint8_t *buf, uint32_t len) {
    uint32_t tail = tail_ % size_;
    uint32_t use = use_ % size_;
    uint32_t space = GetSpaceSize((uint32_t)(-1));
    if ( len > space )
        len = space;
    tail_ += len;

    if ( use > tail ) {
        memcpy(buffer_+tail, buf, len);
        return len;
    } 

    uint32_t l = size_ - tail;
    if ( l >= len ) {
        memcpy(buffer_+tail, buf, len);
        return len;
    }

    memcpy(buffer_+tail, buf, l);
    memcpy(buffer_, buf+l, len-l);
    return len;
}

uint32_t LuaVideoCache::ReadData(uint8_t *buf, uint32_t len) {
    uint32_t tail = tail_ % size_;
    uint32_t use = use_ % size_;
    uint32_t data_size = tail_ - use_;
    if ( len > data_size )
        len = data_size;
    use_ += len;

    if ( use > tail ) {  
        uint32_t l = size_ - use;
        if ( l >= len ) {
          memcpy(buf, buffer_+use, len);
          return len;
        }

        memcpy(buf, buffer_+use, l);
        memcpy(buf+l, buffer_, len-l);
        return len;
    }

    memcpy(buf, buffer_+use, len);
    return len;
}

uint32_t LuaVideoCache::PromiseWriteData(const uint8_t *buf, uint32_t len) {
    uint32_t write_len = WriteData(buf, len);
    if (data_promice_ != NULL && data_promice_->completed(this)) {
        DataPromise::Ptr old_promise = data_promice_;
        old_promise->honor(this); //honor可能立刻触发新的Promise
        if (old_promise == data_promice_) {
            data_promice_.reset();
        }
    }

    return write_len;
}

void LuaVideoCache::PromiseReadData(DataPromise::Ptr promise) {
    if (promise->completed(this)) {
        promise->honor(this);
    } else {
        MakePromise(promise);
    }
}

void LuaVideoCache::MakePromise(DataPromise::Ptr promise) {
    if (data_promice_ != NULL) {
        data_promice_->give_up();
        data_promice_.reset();
    }
    data_promice_ = promise;
}

void LuaVideoCache::ClearData() {
    use_ = 0;
    tail_ = 0;
}

uint32_t LuaVideoCache::SkipData(uint32_t size) {
    uint32_t data_size = tail_ - use_;
    if ( data_size > size ) {
        use_ += size;
        return size;
    }

    use_ = 0;
    tail_ = 0;
    return data_size;
}

void LuaVideoCache::ResetSize(uint32_t size) {
    uint8_t *tmp = new uint8_t[size];
    uint32_t len = ReadData(tmp, size);
    delete [] buffer_;
    use_ = 0;
    tail_ = len;
    size_ = size;
    buffer_ = tmp;
}

uint32_t LuaVideoCache::GetSpaceSize(uint32_t expect_size) {
    uint32_t space =  size_ - (tail_ - use_);
    if ( expect_size > space ) 
        return space;
    return expect_size;
}

uint32_t LuaVideoCache::GetDataSize() {
    return tail_ - use_;
}

int32_t LuaVideoCache::Scanf(lua_State *l, const std::vector<DataField> &format) {
    int32_t ret = -1;
    do {
        if (l == NULL || format.empty()) {
            break;
        }

        ret = 0;
        lua_settop(l, 0);

        std::vector<DataField>::const_iterator iter = format.begin();
        for (;iter != format.end();++iter,++ret) {
            switch (iter->type) {
            case eDataType_int8_t:
            case eDataType_uint8_t:
                {
                    uint8_t v = 0;
                    for (size_t i = 0;i < iter->length;++i) {
                        ReadData((uint8_t*)&v, kDataTypeSize[iter->type]);
                        lua_pushinteger(l, v);
                    }
                }
                break;
            case eDataType_int16_t:
            case eDataType_uint16_t:
                {
                    uint16_t v = 0;
                    for (size_t i = 0;i < iter->length;++i) {
                        ReadData((uint8_t*)&v, kDataTypeSize[iter->type]);
                        if (iter->hton) {
                            v = ntohs(v);
                        }
                        lua_pushinteger(l, v);
                    }
                }
                break;
            case eDataType_int32_t:
            case eDataType_uint32_t:
                {
                    uint32_t v = 0;
                    for (size_t i = 0;i < iter->length;++i) {
                        ReadData((uint8_t*)&v, kDataTypeSize[iter->type]);
                        if (iter->hton) {
                            v = ntohl(v);
                        }
                        lua_pushinteger(l, v);
                    }
                }
                break;
            case eDataType_int64_t:
            case eDataType_uint64_t:
                {
                    uint64_t v = 0;
                    for (size_t i = 0;i < iter->length;++i) {
                        ReadData((uint8_t*)&v, kDataTypeSize[iter->type]);
                        if (iter->hton) {
                            v = _ntohll(v);
                        }
                        lua_pushinteger(l, (uint32_t)v);
                    }
                }
                break;
            case eDataType_double:
                {
                    double v = 0;
                    for (size_t i = 0;i < iter->length;++i) {
                        ReadData((uint8_t*)&v, kDataTypeSize[iter->type]);
                        lua_pushnumber(l, v);
                    }
                }
                break;
            case eDataType_byte_array:
                {
                    IOBuffer data(iter->length);
                    ReadData((uint8_t*)data.data(), data.size());
                    lua_pushlstring(l, (char*)data.data(), data.size());
                }
                break;
            case eDataType_string:
                {
                    char *str = new char[iter->length + 1];
                    str[iter->length] = '\0';
                    ReadData((uint8_t*)str, iter->length);
                    lua_pushstring(l, str);
                    delete [] str;
                }
                break;
            default:
                break;
            }
        }
    } while (0);
    return ret;
}

int32_t LuaVideoCache::LuaOpenVideoCache(lua_State *l) {
    luaL_Reg reg[] = {
        { "WriteData",      LuaVideoCache::LuaWriteData },
        { "ReadData",       LuaVideoCache::LuaReadData },
        { "ClearData",      LuaVideoCache::LuaClearData },
        { "SkipData",       LuaVideoCache::LuaSkipData },
        { "ResetSize",      LuaVideoCache::LuaResetSize },
        { "GetSpaceSize",   LuaVideoCache::LuaGetSpaceSize },
        { "GetDataSize",    LuaVideoCache::LuaGetDataSize },
        { "Version",        LuaVideoCache::LuaVersion },
        { "GetCache",       LuaVideoCache::LuaGetCache },
        { "SyncReadData",   LuaVideoCache::LuaSyncReadData },
        { "Scanf",          LuaVideoCache::LuaScanf },
        { "WaitUntil",      LuaVideoCache::LuaWaitUntil },
        { NULL,             NULL }
    };

    std::string name = "videocache";
    std::string version = "1.0.1";
    return LuaModuleBase::open_module(l, reg, name, version);
}

int32_t LuaVideoCache::LuaWriteData(lua_State *l) {
    ModuleWrapper<LuaVideoCache> *wrapper = get_module_wrapper(l);
    if ( !wrapper ) {
        return -1;
    }

    size_t len = 0;
    const char *data = lua_tolstring(l, -1, &len);
    uint32_t size = wrapper->module->WriteData((const uint8_t *)data, len); 
    lua_settop(l, 0);
    lua_pushinteger(l, size);
    return 1;
}

int32_t LuaVideoCache::LuaReadData(lua_State *l) {
    ModuleWrapper<LuaVideoCache> *wrapper = get_module_wrapper(l);
    uint32_t read_len = 0;
    if ( wrapper && 2 == lua_gettop(l) ) {
        uint8_t *buf = (uint8_t *)lua_topointer(l, -2);
        uint32_t len = (uint32_t)lua_tointeger(l, -1);
        read_len = wrapper->module->ReadData(buf, len);
    } else {
        lua_settop(l, 0);
        return -1;
    }
    lua_settop(l, 0);
    lua_pushinteger(l, read_len);
    return 1;
}


int32_t LuaVideoCache::LuaClearData(lua_State *l) {
    ModuleWrapper<LuaVideoCache> *wrapper = get_module_wrapper(l);
    if ( !wrapper ) {
        return -1;
    }

    wrapper->module->ClearData();
    return 0;
}

int32_t LuaVideoCache::LuaSkipData(lua_State *l) {
    ModuleWrapper<LuaVideoCache> *wrapper = get_module_wrapper(l);
    uint32_t skip_len = 0;
    if ( wrapper && 1 == lua_gettop(l) )  {
        uint32_t len = (uint32_t)lua_tointeger(l, -1);
        skip_len = wrapper->module->SkipData(len);
    } else {
        lua_settop(l, 0);
        return -1;
    }
    lua_settop(l, 0);
    lua_pushinteger(l, skip_len);
    return 1;
}

int32_t LuaVideoCache::LuaResetSize(lua_State *l) {
    ModuleWrapper<LuaVideoCache> *wrapper = get_module_wrapper(l);
    if ( wrapper && 1 == lua_gettop(l) ) {
        uint32_t len = (uint32_t)lua_tointeger(l, -1);
        wrapper->module->ResetSize(len);
    } else {
        return -1;
    }
    lua_settop(l, 0);
    return 0;
}

int32_t LuaVideoCache::LuaGetSpaceSize(lua_State *l) {
    ModuleWrapper<LuaVideoCache> *wrapper = get_module_wrapper(l);
    uint32_t space_len = 0;
    if ( wrapper && 1 == lua_gettop(l) ) {
        uint32_t expect_size = (uint32_t)lua_tointeger(l, -1);
        space_len = wrapper->module->GetSpaceSize(expect_size);
    } else {
        lua_settop(l, 0);
        return -1;
    }
    lua_settop(l, 0);
    lua_pushinteger(l, space_len);
    return 1;
}

int32_t LuaVideoCache::LuaGetDataSize(lua_State *l) {
    ModuleWrapper<LuaVideoCache> *wrapper = get_module_wrapper(l);
    uint32_t data_size = 0;
    if ( wrapper ) {
        data_size = wrapper->module->GetDataSize();
    } else {
        lua_settop(l, 0);
        return -1;
    }
    lua_settop(l, 0);
    lua_pushinteger(l, data_size);
    return 1;
}

int32_t LuaVideoCache::LuaVersion(lua_State *l) {
    ModuleWrapper<LuaVideoCache> *wrapper = get_module_wrapper(l);
    lua_settop(l, 0); 
    if ( wrapper ) {
        lua_pushstring(l, wrapper->module->ver_.c_str());
    } else {
        lua_pushstring(l, "null");
    }

    return 1;
}

int32_t LuaVideoCache::LuaGetCache(lua_State *l) {
    ModuleWrapper<LuaVideoCache> *wrapper = get_module_wrapper(l);
    lua_settop(l, 0); 
    if ( wrapper ) {
        lua_pushlightuserdata(l, wrapper->module);
    } else {
        lua_pushnil(l);
    }

    return 1;
}

int32_t LuaVideoCache::LuaSyncReadData(lua_State *l) {
    do {
        ModuleWrapper<LuaVideoCache> *wrapper = get_module_wrapper(l);
        if (wrapper == NULL) {
            return -1;
        }

        int32_t args_len = lua_gettop(l);
        if (3 != args_len) {
            return -1;
        }

        if (!lua_islightuserdata(l, -3) || !lua_isinteger(l, -2) || !lua_isinteger(l, -1)) {
            return -1;
        }

        uint8_t *buf = (uint8_t *)lua_topointer(l, -3);
        uint32_t len = (uint32_t)lua_tointeger(l, -2);
        if (buf == NULL || len == 0) {
            return -1;
        }

        int32_t cond = (int32_t)lua_tointeger(l, -1);
        DataCompletionCondition::Ptr condition = DataCompCondFactory::create(cond, len);

        //Cache enough,read directly.
        int32_t read_len = 0;
        if ((*condition)(wrapper->module->GetDataSize()) && (read_len = wrapper->module->ReadData(buf, len)) > 0) {
            //Return LUA_OK to resume.
            lua_settop(l, 0);
            lua_pushinteger(l, read_len);
            return 1;
        }

        //Wait for data,return LUA_YIELD to resume.
        LuaReadPromise::Ptr promise(new LuaReadPromise(buf, len, condition));
        promise->set_coroutine(l);
        promise->set_main_thread(wrapper->module->GetMainThread());
        wrapper->module->MakePromise(promise);

        int32_t ret = (int32_t)promise->wait();
        if (ret != 0) {
            return ret;
        }
    } while (0);
    
    lua_settop(l, 0);
    return lua_yieldk(l, 0, 0, LuaReadPromise::on_wait_result); //Pass nothing to lua_resume
}

int32_t LuaVideoCache::LuaScanf(lua_State *l)
{
    int32_t ret = -1;
    do {
        ModuleWrapper<LuaVideoCache> *wrapper = get_module_wrapper(l);
        if (wrapper == NULL) {
            break;
        }

        int32_t args_len = lua_gettop(l);
        if (1 != args_len) {
            break;
        }

        if (!lua_istable(l, -1)) {
            break;
        }

        DataField field;
        std::vector<DataField> format;
        bool valid = true;
        int32_t total_size = 0;
        
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
                } else if (strcmp(key, "length") == 0) {
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
                } else if (strcmp(key, "hton") == 0) {
                    if (!lua_isboolean(l, -1)) {
                        valid = false;
                        break;
                    }
                    field.hton = lua_toboolean(l, -1)==0?false:true;
                } else {
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
            break;
        }
     
        DataCompletionCondition::Ptr condition = DataCompCondFactory::create(eReadCompletionConditon_Enough, total_size);
        if ((*condition)(wrapper->module->GetDataSize()) && (ret = wrapper->module->Scanf(l, format)) > 0) {//Return LUA_OK to resume.
            break;
        }

        //Wait for data,return LUA_YIELD to resume.
        LuaScanfPromise::Ptr promise(new LuaScanfPromise(NULL, total_size, condition));
        promise->set_coroutine(l);
        promise->set_main_thread(wrapper->module->GetMainThread());
        promise->set_format(format);
        wrapper->module->MakePromise(promise);

        ret = (int32_t)promise->wait();
    } while (0);
    return ret;
}

int32_t LuaVideoCache::LuaWaitUntil(lua_State *l) {
    int32_t ret = -1;
    do {
        ModuleWrapper<LuaVideoCache> *wrapper = get_module_wrapper(l);
        if (wrapper == NULL) {
            break;
        }

        int32_t args_len = lua_gettop(l);
        if (1 != args_len) {
            break;
        }

        if (!lua_isinteger(l, -1)) {
            break;
        }

        size_t len = (size_t)lua_tointeger(l, -1);
        DataCompletionCondition::Ptr condition = READ_ENOUGH(len);

        //Cache enough
        if ((*condition)(wrapper->module->GetDataSize())) {
            lua_settop(l, 0);
            lua_pushboolean(l, true);
            ret = 1;
            break;
        }

        //Wait for data,return LUA_YIELD to resume.
        WaitUntilPromise::Ptr promise(new WaitUntilPromise(NULL, len, condition));
        promise->set_coroutine(l);
        promise->set_main_thread(wrapper->module->GetMainThread());
        wrapper->module->MakePromise(promise);

        ret = (int32_t)promise->wait();
    } while (0);
    return ret;
}

} // namespace bee
