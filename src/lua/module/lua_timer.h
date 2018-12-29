#ifndef __LUA_TIMER_H__
#define __LUA_TIMER_H__

#include "utility/timer.h"
#include "log/logger.h"

extern "C" {
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
}

namespace bee {

////////////////////////////////////LuaTimer//////////////////////////////////////
class LuaTimer : public std::enable_shared_from_this<LuaTimer> {
public:
    typedef std::shared_ptr<LuaTimer> Ptr;
    LuaTimer(lua_State *main);
    ~LuaTimer();

public:
    bool open(IOSPtr ios, int32_t interval, int32_t timeout_count, int32_t callback);
    void close();
    void handle_timeout();

protected:
    AsyncWaitTimer::Ptr timer_;
    lua_State *main_ = NULL;
    int32_t callback_ = LUA_REFNIL;
    int32_t interval_ = -1; //ms
    Logger logger_;
};

} // namespace bee 

#endif
