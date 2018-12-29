#ifndef __LUA_HTTP_READER_H__
#define __LUA_HTTP_READER_H__

#include "lua_data_promise.h"
#include "utility/common.h"
#include "bee/base/bee_define.h"

extern "C" {
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
}

namespace bee {

////////////////////////////////////LuaHttpReader//////////////////////////////////////
class LuaHttpSession;
class LuaHttpReader {
public:
    typedef std::shared_ptr<LuaHttpReader> Ptr;
    LuaHttpReader(std::shared_ptr<LuaHttpSession> http_session, uint8_t *buff, size_t size, lua_State *main, lua_State *co);
    virtual ~LuaHttpReader();

public:
    virtual bool    read();
    virtual void    close();
    virtual bool    notify(BeeErrorCode e1, const boost::system::error_code &ec2, size_t size);
    virtual int32_t push_return_value();
    virtual size_t  get_read_len(){return pos_;}
    virtual bool    completed() = 0;
    static  int32_t on_wait_result(lua_State *l, int32_t status, lua_KContext ctx);

protected:
    virtual bool resume(int32_t count);
    virtual bool give_up();

protected:
    std::weak_ptr<LuaHttpSession> http_session_;    
    uint8_t   *buff_;
    size_t    size_;
    size_t    pos_;
    lua_State *main_;
    lua_State *co_;
};

////////////////////////////////////LuaHttpSomeReader//////////////////////////////////////
class LuaHttpSomeReader : public LuaHttpReader {
public:
    LuaHttpSomeReader(std::shared_ptr<LuaHttpSession> http_session, uint8_t *buff, size_t size, lua_State *main, lua_State *co);
    virtual ~LuaHttpSomeReader();

public:
    virtual bool completed();
};

////////////////////////////////////LuaHttpExactlyReader//////////////////////////////////////
class LuaHttpExactlyReader : public LuaHttpReader {
public:
    LuaHttpExactlyReader(std::shared_ptr<LuaHttpSession> http_session, uint8_t *buff, size_t size, lua_State *main, lua_State *co);
    virtual ~LuaHttpExactlyReader();

public:
    virtual bool completed();
};

////////////////////////////////////LuaHttpScanfReader//////////////////////////////////////
class IOBuffer;
class LuaHttpScanfReader : public LuaHttpExactlyReader {
public:
    LuaHttpScanfReader(std::shared_ptr<LuaHttpSession> http_session, uint8_t *buff, size_t size, lua_State *main, lua_State *co);
    virtual ~LuaHttpScanfReader();

public:
    virtual  int32_t push_return_value();
    bool     init(size_t size, const std::vector<DataField> &format);

protected:
    int32_t  scanf();
    uint32_t read_data(uint8_t *dst, uint32_t len);

protected:
    size_t read_pos_;
    std::vector<DataField> format_;
    std::shared_ptr<IOBuffer> io_buffer_;
};

} // namespace bee

#endif
