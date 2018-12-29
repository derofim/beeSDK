#ifndef __LUA_DATA_PROMISE_H__
#define __LUA_DATA_PROMISE_H__

#include "utility/common.h"
#include "utility/algorithm.h"

extern "C" {
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
}

namespace bee {

class BeeSession;
class LuaVideoCache;

////////////////////////////////////DataTypeId//////////////////////////////////////
typedef enum DataTypeId {
    eDataType_int8_t = 0,
    eDataType_uint8_t,
    eDataType_int16_t,
    eDataType_uint16_t,
    eDataType_int32_t,
    eDataType_uint32_t,
    eDataType_int64_t,  //Should not used
    eDataType_uint64_t, //Should not used
    eDataType_double,
    eDataType_byte_array,
    eDataType_string,
    eDataType_count
}DataTypeId;

const size_t kDataTypeSize[eDataType_count] = {
    sizeof(int8_t),
    sizeof(uint8_t),
    sizeof(int16_t),
    sizeof(uint16_t),
    sizeof(int32_t),
    sizeof(uint32_t),
    sizeof(int64_t),
    sizeof(uint64_t),
    sizeof(double),
    sizeof(uint8_t),
    sizeof(int8_t)
};

///////////////////////////////////DataField///////////////////////////////////////
typedef struct DataField {
    DataTypeId  type;
    size_t      length;
    bool        hton;
    DataField() {
        type    = eDataType_int32_t;
        length  = 1;
        hton    = false;
    }
}DataField;

/////////////////////////////////////ReadCompletionConditon/////////////////////////////////////
typedef enum ReadCompletionConditon {
    eReadCompletionConditon_Some = 0,
    eReadCompletionConditon_Exactly,
    eReadCompletionConditon_Enough,
    eReadCompletionConditon_Count
}ReadCompletionConditon;

///////////////////////////////////DataCompletionCondition///////////////////////////////////////
class DataCompletionCondition {
public:
    typedef std::shared_ptr<DataCompletionCondition> Ptr;
    DataCompletionCondition(size_t size = 0):size_(size){}
    virtual bool operator()(size_t len) = 0;

protected:
    size_t size_;
};

/////////////////////////////////////SomeCondition/////////////////////////////////////
class SomeCondition : public DataCompletionCondition {
public:
    typedef std::shared_ptr<SomeCondition> Ptr;
    SomeCondition(size_t size = 0):DataCompletionCondition(size){}
    virtual bool operator()(size_t len){return len > 0;}
};

/////////////////////////////////////ExactlyCondition/////////////////////////////////////
class ExactlyCondition : public DataCompletionCondition {
public:
    typedef std::shared_ptr<ExactlyCondition> Ptr;
    ExactlyCondition(size_t size = 0):DataCompletionCondition(size){}
    virtual bool operator()(size_t len){return len == size_;}
};

/////////////////////////////////////EnoughCondition/////////////////////////////////////
class EnoughCondition : public DataCompletionCondition {
public:
    typedef std::shared_ptr<EnoughCondition> Ptr;
    EnoughCondition(size_t size = 0):DataCompletionCondition(size){}
    virtual bool operator()(size_t len){return len >= size_;}
};

#define READ_SOME(len)     SomeCondition::Ptr(new SomeCondition(len))
#define READ_EXACTLY(len)  ExactlyCondition::Ptr(new ExactlyCondition(len))
#define READ_ENOUGH(len)   EnoughCondition::Ptr(new EnoughCondition(len))

//////////////////////////////////DataCompCondFactory////////////////////////////////////////
class DataCompCondFactory {
public:
    static DataCompletionCondition::Ptr create(int32_t conditon, size_t len) {
        switch (conditon) {
        case eReadCompletionConditon_Some:
            return READ_SOME(len);
        case eReadCompletionConditon_Exactly:
            return READ_EXACTLY(len);
        case eReadCompletionConditon_Enough:
        default:
            return READ_ENOUGH(len);
        }
    }
};

////////////////////////////////////DataPromise//////////////////////////////////////
class DataPromise {
public:
    typedef std::shared_ptr<DataPromise> Ptr;
    DataPromise(uint8_t *buffer, size_t length, DataCompletionCondition::Ptr condition);
    virtual ~DataPromise();

public:
    virtual bool     do_honor(LuaVideoCache *cache) = 0;
    virtual uint32_t wait() = 0;
    virtual void     do_give_up() = 0;

    virtual bool     honor(LuaVideoCache *cache);
    virtual bool     honor(size_t len);
    virtual bool     do_honor(size_t len);
    virtual bool     give_up();
    virtual bool     completed(LuaVideoCache *cache);
    virtual bool     completed(size_t len);
    uint8_t          *get_buffer_ptr(){return buffer_;}
    size_t           get_buffer_length(){return length_;}

protected:
    uint8_t *buffer_;
    size_t length_;
    DataCompletionCondition::Ptr condition_;
    bool honored_;
};

////////////////////////////////////SyncPromise//////////////////////////////////////
class SyncPromise : public DataPromise {
public:
    typedef std::shared_ptr<SyncPromise> Ptr;
    SyncPromise(uint8_t *buffer, size_t length, DataCompletionCondition::Ptr condition);
    virtual ~SyncPromise();

public:
    virtual bool     do_honor(LuaVideoCache *cache); //For bypass read,data must be directly read from cache.
    virtual bool     do_honor(size_t len); //for lua read,when notified,data already in buffer,just pass read length.
    virtual uint32_t wait();
    virtual void     do_give_up();

protected:
    std::promise<uint32_t> promise_;
};

////////////////////////////////////LuaReadPromise//////////////////////////////////////
class LuaReadPromise : public DataPromise {
public:
    typedef std::shared_ptr<LuaReadPromise> Ptr;
    LuaReadPromise(uint8_t *buffer, size_t length, DataCompletionCondition::Ptr condition);
    virtual ~LuaReadPromise();

public:
    virtual bool     do_honor(LuaVideoCache *cache);
    virtual uint32_t wait();
    virtual void     do_give_up();
    void             set_main_thread(lua_State *L){main_ = L;}
    void             set_coroutine(lua_State *L){co_ = L;}
    static int32_t   on_wait_result(lua_State *l, int32_t status, lua_KContext ctx);

protected:
    lua_State *main_;
    lua_State *co_;
};

////////////////////////////////////WaitUntilPromise//////////////////////////////////////
class WaitUntilPromise : public LuaReadPromise {
public:
    typedef std::shared_ptr<WaitUntilPromise> Ptr;
    WaitUntilPromise(uint8_t *buffer, size_t length, DataCompletionCondition::Ptr condition);
    virtual ~WaitUntilPromise();

public:
    virtual bool do_honor(LuaVideoCache *cache);
};

////////////////////////////////////LuaScanfPromise//////////////////////////////////////
class LuaScanfPromise : public LuaReadPromise {
public:
    typedef std::shared_ptr<LuaScanfPromise> Ptr;
    LuaScanfPromise(uint8_t *buffer, size_t length, DataCompletionCondition::Ptr condition);
    virtual ~LuaScanfPromise();

public:
    virtual bool do_honor(LuaVideoCache *cache);
    void set_format(const std::vector<DataField> &format){format_ = format;}

protected:
    std::vector<DataField> format_;
};

} // namespace bee

#endif
