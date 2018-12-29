#include "lua_data_promise.h"
#include "lua_video_cache.h"
#include "session/bee_session.h"

namespace bee {

/////////////////////////////////////DataPromise/////////////////////////////////////
DataPromise::DataPromise(uint8_t *buffer, size_t length, DataCompletionCondition::Ptr condition):
    buffer_(buffer),
    length_(length),
    condition_(condition),
    honored_(false) {

}

DataPromise::~DataPromise() {

}

bool DataPromise::honor(LuaVideoCache *cache) {
    bool ret = true;
    do {
        if (honored_) {
            break;
        }

        ret = do_honor(cache);
        if (!ret) {
            break;
        }

        honored_ = true;
    } while (0);
    return ret;
}

bool DataPromise::honor(size_t len) {
    bool ret = true;
    do {
        if (honored_) {
            break;
        }

        ret = do_honor(len);
        if (!ret) {
            break;
        }

        honored_ = true;
    } while (0);
    return ret;
}

bool DataPromise::do_honor(size_t len) {
    return false;
}

bool DataPromise::give_up() {
    bool ret = true;
    do {
        if (honored_) {
            break;
        }

        do_give_up();
        honored_ = true;
    } while (0);
    return ret;
}

bool DataPromise::completed(LuaVideoCache *cache) {
    bool ret = true;
    do {
        if (cache == NULL) {
            ret = false;
            break;
        }

        ret = completed((size_t)cache->GetDataSize());
    } while (0);
    return ret;
}

bool DataPromise::completed(size_t len) {
    bool ret = true;
    do {
        if (condition_ == NULL) {
            ret = false;
            break;
        }

        ret = (*condition_)(len);
    } while (0);
    return ret;
}

///////////////////////////////////SyncPromise///////////////////////////////////////
SyncPromise::SyncPromise(uint8_t *buffer, size_t length, DataCompletionCondition::Ptr condition):DataPromise(buffer, length, condition) {

}

SyncPromise::~SyncPromise() {

}

bool SyncPromise::do_honor(LuaVideoCache *cache) {
    bool ret = true;
    do {
        if (cache == NULL || buffer_ == NULL || length_ == 0) {
            ret = false;
            break;
        }

        size_t read_len = cache->ReadData(buffer_, length_);
        promise_.set_value(read_len);
    } while (0);
    return ret;
}

bool SyncPromise::do_honor(size_t len) {
    promise_.set_value(len);
    return true;
}

uint32_t SyncPromise::wait() {
    std::shared_future<uint32_t> future = promise_.get_future();
    return future.get();
}

void SyncPromise::do_give_up() {
    promise_.set_value(0);
}

///////////////////////////////////LuaReadPromise///////////////////////////////////////
LuaReadPromise::LuaReadPromise(uint8_t *buffer, size_t length, DataCompletionCondition::Ptr condition):DataPromise(buffer, length, condition), main_(NULL), co_(NULL) {

}

LuaReadPromise::~LuaReadPromise() {

}

bool LuaReadPromise::do_honor(LuaVideoCache *cache) {
    bool ret = true;
    do {
        if (cache == NULL || buffer_ == NULL || length_ == 0) {
            ret = false;
            break;
        }

        if (co_ == NULL || main_ == NULL) {
            ret = false;
            break;
        }

        size_t read_len = cache->ReadData(buffer_, length_);

        lua_settop(co_, 0);
        lua_pushinteger(co_, (lua_Integer)read_len);

        lua_resume(co_, main_, 1);
    } while (0);
    return ret;
}

uint32_t LuaReadPromise::wait() {
    uint32_t ret = -1;
    do {
        if (co_ == NULL) {
            break;
        }

        ret = 0;
    } while (0);
    return ret;
}

void LuaReadPromise::do_give_up() {
    do {
        if (co_ == NULL || main_ == NULL) {
            break;
        }

        lua_settop(co_, 0);
        lua_resume(co_, main_, 0); //Push nothing to stack, so lua get nil.
    } while (0);
}

int32_t LuaReadPromise::on_wait_result(lua_State *l, int32_t status, lua_KContext ctx) {
    int32_t nresult = lua_gettop(l);
    return nresult;
}

///////////////////////////////////WaitUntilPromise///////////////////////////////////////
WaitUntilPromise::WaitUntilPromise(uint8_t *buffer, size_t length, DataCompletionCondition::Ptr condition):LuaReadPromise(buffer, length, condition) {

}

WaitUntilPromise::~WaitUntilPromise() {

}

bool WaitUntilPromise::do_honor(LuaVideoCache *cache) {
    bool ret = true;
    do {
        if (cache == NULL || length_ == 0) {
            ret = false;
            break;
        }

        if (co_ == NULL || main_ == NULL) {
            ret = false;
            break;
        }

        size_t len = cache->GetDataSize();
        lua_settop(co_, 0);
        lua_pushboolean(co_, len >= length_);
        lua_resume(co_, main_, 1);
    } while (0);
    return ret;
}

///////////////////////////////////LuaScanfPromise///////////////////////////////////////
LuaScanfPromise::LuaScanfPromise(uint8_t *buffer, size_t length, DataCompletionCondition::Ptr condition):LuaReadPromise(buffer, length, condition) {

}

LuaScanfPromise::~LuaScanfPromise() {

}

bool LuaScanfPromise::do_honor(LuaVideoCache *cache) {
    bool ret = true;
    do {
        if (cache == NULL || co_ == NULL || main_ == NULL) {
            ret = false;
            break;
        }

        int32_t rs = cache->Scanf(co_, format_);
        if (rs != (int32_t)format_.size()) {
            ret = false;
            break;
        }

        lua_resume(co_, main_, rs);
    } while (0);
    return ret;
}

} // namespace bee
