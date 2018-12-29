#include "lua_timer.h"

namespace bee {

LuaTimer::LuaTimer(lua_State *main) : main_(main), logger_("LuaTimer"){

}

LuaTimer::~LuaTimer() {

}

bool LuaTimer::open(IOSPtr ios, int32_t interval, int32_t timeout_count, int32_t callback) {
    bool ret = true;
    do {
        if (ios == NULL) {
            ret = false;
            break;
        }

        timer_ = AsyncWaitTimer::create(*ios);
        timer_->set_wait_millseconds(interval);
        timer_->set_wait_times(timeout_count);
        timer_->set_immediately(false);
        timer_->async_wait(boost::bind(&LuaTimer::handle_timeout, shared_from_this()));

        callback_ = callback;
    } while (0);
    return ret;
}

void LuaTimer::close() {
    if (timer_ != NULL) {
        timer_->cancel();
        timer_.reset();
    }
}

void LuaTimer::handle_timeout() {
    do {
        if (main_ == NULL) {
            break;
        }

        if (callback_ == LUA_REFNIL) {
            break;
        }

        lua_State *co = lua_newthread(main_);
        lua_rawgeti(co, LUA_REGISTRYINDEX, callback_);
        int32_t ret = lua_resume(co, main_, 0);
        lua_pop(main_, 1);

        if (ret != LUA_OK && ret != LUA_YIELD) {
            logger_.Error("Lua timer resume failed.\n");
            int32_t args_len = lua_gettop(co);
            if (args_len > 0 && lua_type(co, -1) == LUA_TSTRING) {
                logger_.Error("Lua timer error %s.\n", lua_tostring(co, -1));
            }
        }
    } while (0);
}

} // namespace bee 
