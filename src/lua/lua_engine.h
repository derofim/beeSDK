#ifndef __LUA_ENGINE_H__
#define __LUA_ENGINE_H__

#include "utility/common.h"
#include "bee/base/bee.h"
#include "log/logger.h"

extern "C" {
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
}

namespace bee {

/////////////////////////////////////LuaEngine/////////////////////////////////////
class BeeSession;
class LuaEngine {
public:
    typedef std::shared_ptr<LuaEngine> Ptr;
    LuaEngine(BeeSession *session);
    virtual ~LuaEngine();

public:
    BeeErrorCode   open(const std::string &lua_name, const std::string &lua_buff, const SystemParam &param, const std::string &live_session_id, std::vector<BeeCapability> &capability);
    BeeErrorCode   close();
    BeeErrorCode   execute(bee_int32_t svc_code, const std::string &cmd, const std::string &args);
    void           register_function();
    void           lua_open_extra_libs(lua_State *L);

public:
    static int32_t lua_bee_log(lua_State *L);
    static int32_t lua_get_tick_count32(lua_State *L);
    static int32_t lua_xxtea_encrypt(lua_State *L);
    static int32_t lua_xxtea_decrypt(lua_State *L);
    static int32_t lua_base64_encode(lua_State *L);
    static int32_t lua_base64_decode(lua_State *L);
    static int32_t lua_url_encode(lua_State *L);
    static int32_t lua_forward_pointer(lua_State *L);
    static int32_t lua_configure_net_log(lua_State *L);
    static int32_t lua_svc_notify(lua_State *L);

protected:
    void reset();
    void check_and_create_bee_table(const char *table_name);
    bool store_session_to_lua();
    bool set_sys_info(const SystemParam &param, const std::string &live_session_id);
    bool get_capability(std::vector<BeeCapability> &capability);

protected:
    lua_State *L_;
    BeeSession *session_;
    std::string lua_name_;
    std::string lua_buff_;
    bool bee_table_created_ = false;
    Logger logger_;
};

} // namespace bee

#endif
