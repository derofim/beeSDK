
#include "lua_engine.h"
#include "lua_default_http_session.h"
#include "module/lua_video_cache.h"
#include "module/lua_crypto_module.h"
#include "module/lua_iobuffer_module.h"
#include "module/lua_http_module.h"
#include "module/lua_lws_module.h"
#include "module/lua_webrtc_module.h"
#include "module/lua_timer_module.h"
#include "session/bee_session.h"
#include "utility/crypto.h"
#include "bee/base/bee_define.h"
#include "bee/base/bee_version.h"
#include "log/logger.h"
#include "service/bee_entrance.h"

#ifdef IOS
#include "platform/ios/ios_adapter.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif
#include "module/cjson/lua_cjson.h"
#ifdef __cplusplus
} // extern "C"
#endif

namespace bee {

const std::string kSvcCodeString = "svc";
const std::string kSvcDescString = "desc";

///////////////////////////////////LuaEngine///////////////////////////////////////
LuaEngine::LuaEngine(BeeSession *session):L_(NULL), session_(session), logger_("LuaEngine"){

}

LuaEngine::~LuaEngine() {

}

BeeErrorCode LuaEngine::open(const std::string &lua_name, const std::string &lua_buff, const SystemParam &param, const std::string &live_session_id, std::vector<BeeCapability> &capability) {
    BeeErrorCode ret = kBeeErrorCode_Success;
    do {
        if (lua_name.empty() || lua_buff.empty()) {
            ret = kBeeErrorCode_Invalid_Param;
            break;
        }

        reset();

        lua_name_ = lua_name;
        lua_buff_ = lua_buff;

        if (lua_buff_.empty()) {
            ret = kBeeErrorCode_Engine_Not_Loaded;
            break;
        }        

        L_ = luaL_newstate();
        luaL_openlibs(L_);
        register_function();
        lua_open_extra_libs(L_);

        int32_t rs = (luaL_loadbuffer(L_, lua_buff_.c_str(), lua_buff_.length(), lua_name_.c_str()) || lua_pcall(L_, 0, LUA_MULTRET, 0));
        if  (rs != LUA_OK) {
            ret = kBeeErrorCode_Engine_Script_Error;
            int32_t args_len = lua_gettop(L_);
            if (args_len > 0 && lua_type(L_, -1) == LUA_TSTRING) {
                logger_.Error("Lua open error %s.\n", lua_tostring(L_, -1));
            }
            break;
        }

        if (!store_session_to_lua()) {
            ret = kBeeErrorCode_Invalid_Param;
            break;
        }

        if (!set_sys_info(param, live_session_id)) {
            ret = kBeeErrorCode_Engine_Script_Error;
            break;
        }

        if (!get_capability(capability)) {
            ret = kBeeErrorCode_Service_Not_Supported;
            break;
        }
    } while (0);

    if (ret != kBeeErrorCode_Success) {
        reset();
    }
    return ret;
}

BeeErrorCode LuaEngine::close() {
    reset();
    return kBeeErrorCode_Success;
}

BeeErrorCode LuaEngine::execute(bee_int32_t svc_code, const std::string &cmd, const std::string &args) {
    BeeErrorCode ret = kBeeErrorCode_Success;
    do {
        if (cmd.empty() || args.empty()) {
            ret = kBeeErrorCode_Invalid_Param;
            break;
        }

        if (L_ == NULL) {
            ret = kBeeErrorCode_Engine_Not_Loaded;
            break;
        }

        lua_State *co = lua_newthread(L_);
        lua_getglobal(co, cmd.c_str());
        lua_pushinteger(co, svc_code);
        lua_pushlstring(co, args.c_str(), args.size());

        //Clear main thread stack.
        lua_settop(L_, 0);

        int32_t nret = lua_resume(co, L_, 2);
        if (LUA_YIELD == nret) {
            //logger_.Debug("Execute resume LUA_YIELD.\n");
        } else if (LUA_OK == nret) {
            //logger_.Debug("Execute resume LUA_OK.\n");
        } else {
            logger_.Error("Execute (%d %s) resume failed.\n", svc_code, cmd.c_str());
            ret = kBeeErrorCode_Engine_Script_Error;
            int32_t args_len = lua_gettop(co);
            if (args_len > 0 && lua_type(co, -1) == LUA_TSTRING) {
                logger_.Error("Execute error %s.\n", lua_tostring(co, -1));
            }
            break;
        }
    } while (0);
    return ret;
}

void LuaEngine::register_function() {
    lua_register(L_, "sdk_BeeLog",          LuaEngine::lua_bee_log);
    lua_register(L_, "sdk_GetTickGount32",  LuaEngine::lua_get_tick_count32);
    lua_register(L_, "sdk_xxteaEncrypt",    LuaEngine::lua_xxtea_encrypt);
    lua_register(L_, "sdk_xxteaDecrypt",    LuaEngine::lua_xxtea_decrypt);
    lua_register(L_, "sdk_Base64Encode",    LuaEngine::lua_base64_encode);
    lua_register(L_, "sdk_Base64Decode",    LuaEngine::lua_base64_decode);
    lua_register(L_, "sdk_UrlEncode",       LuaEngine::lua_url_encode);
    lua_register(L_, "sdk_ForwardPointer",  LuaEngine::lua_forward_pointer);
    lua_register(L_, "sdk_ConfigureNetLog", LuaEngine::lua_configure_net_log);
    lua_register(L_, "sdk_SvcNotify",       LuaEngine::lua_svc_notify);
}

void LuaEngine::lua_open_extra_libs(lua_State *L) {
    luaL_Reg ext[] = {
        { "cjson",      luaopen_cjson },
        { "videocache", LuaVideoCache::LuaOpenVideoCache },
        { "crypto",     LuaCryptoModule::lua_open_module },
        { "iobuffer",   LuaIOBufferModule::lua_open_module },
        { "http",       LuaHttpModule::lua_open_module },
        { "lws",        LuaLwsModule::lua_open_module },
        { "webrtc",     LuaWebrtcModule::lua_open_module },
        { "timer",      LuaTimerModule::lua_open_module },
        { NULL, NULL }
    };

    luaL_Reg *reg = ext;
    lua_getglobal(L, "package");
    lua_getfield(L, -1, "preload");

    for ( ; reg->name; reg++ ) {
        lua_pushcfunction(L, reg->func);
        lua_setfield(L, -2, reg->name);
    }

    lua_pop(L, 2);
}

void LuaEngine::reset() {
    if (L_ != NULL) {
        lua_close(L_);
        L_ = NULL;
    }

    lua_name_   = "";
    lua_buff_   = "";
}

void LuaEngine::check_and_create_bee_table(const char *table_name) {
    if (bee_table_created_) {
        lua_getglobal(L_, table_name);  //Get table on stack top.
    } else {
        lua_newtable(L_);               //Create a table on stack top.
        lua_setglobal(L_, table_name);  //Pop the table on stack top and set it to global.
        lua_getglobal(L_, table_name);  //Push back the table from global on stack top.
        bee_table_created_ = true;      //Set flag.
    }
}

bool LuaEngine::store_session_to_lua() {
    bool ret = true;
    do {
        if (L_ == NULL || session_ == NULL) {
            ret = false;
            break;
        }

        const char *table_name = "BEE_TABLE";
        check_and_create_bee_table(table_name);

        //Set BEE_TABLE["bee_session"] = session_
        lua_pushstring(L_, "bee_session");
        lua_pushlightuserdata(L_, session_);
        lua_settable(L_, -3);
    } while (0);
    return ret;
}

bool LuaEngine::set_sys_info(const SystemParam &param, const std::string &live_session_id) {
    bool ret = true;
    do {
        if (L_ == NULL) {
            ret = false;
            break;
        }

        //Pass system info to script.
        lua_settop(L_, 0);
        lua_getglobal(L_, "SetSysInfo");
        if (lua_gettop(L_) != 1) { //Script may not accept these parameters, so SetSysInfo is not mandatory.
            break;
        }

        if (lua_isnil(L_, -1) || !lua_isfunction(L_, -1)) {
            ret = false;
            break;
        }

        lua_pushinteger(L_, (lua_Integer)param.platform_type);
        lua_pushinteger(L_, (lua_Integer)param.net_type);
        lua_pushstring(L_, param.app_name.c_str());
        lua_pushstring(L_, param.app_version.c_str());
        lua_pushstring(L_, BEE_VERSION);
        lua_pushstring(L_, param.system_info.c_str());
        lua_pushstring(L_, param.machine_code.c_str());
        lua_pushstring(L_, live_session_id.c_str());

        int32_t rs = lua_pcall(L_, 8, 0, 0);
        if (rs != LUA_OK) {
            int32_t args_len = lua_gettop(L_);
            if (args_len > 0 && lua_type(L_, -1) == LUA_TSTRING) {
                logger_.Error("Lua SetSysInfo error %s.\n", lua_tostring(L_, -1));
            }
            ret = false;
        }
    } while (0);
    return ret;
}

bool LuaEngine::get_capability(std::vector<BeeCapability> &capability) {
    bool ret = true;
    do {
        if (L_ == NULL) {
            ret = false;
            break;
        }

        lua_settop(L_, 0);
        lua_getglobal(L_, "GetCapability");
        if (lua_gettop(L_) != 1) {
            ret = false;
            break;
        }

        if (lua_isnil(L_, -1) || !lua_isfunction(L_, -1)) {
            ret = false;
            break;
        }

        int32_t rs = lua_pcall(L_, 0, 1, 0);
        if (rs != LUA_OK) {
            int32_t args_len = lua_gettop(L_);
            if (args_len > 0 && lua_type(L_, -1) == LUA_TSTRING) {
                logger_.Error("Lua GetCapability error %s.\n", lua_tostring(L_, -1));
            }
            ret = false;
            break;
        }

        if (lua_gettop(L_) != 1) {
            ret = false;
            break;
        }

        if (!lua_istable(L_, -1)) {
            ret = false;
            break;
        }

        bool valid1 = true;
        lua_pushnil(L_);
        while (lua_next(L_, -2) != 0) {
            do {
                if (!lua_istable(L_, -1)) {
                    valid1 = false;
                    break;
                }

                int32_t svc_code = -1;
                std::string svc_desc;
                bool valid2 = true;
                lua_pushnil(L_);
                while (lua_next(L_, -2) != 0) {
                    do {
                        if (!lua_isstring(L_, -2)) {
                            valid2 = false;
                            break;
                        }

                        const char *name = lua_tostring(L_, -2);
                        if (name == NULL) {
                            valid2 = false;
                            break;
                        }

                        if (0 == strncmp(name, kSvcCodeString.c_str(), kSvcCodeString.size())) {
                            if (!lua_isinteger(L_, -1)) {
                                valid2 = false;
                                break;
                            }
                            svc_code = (int32_t)lua_tointeger(L_, -1);
                            break;
                        }

                        if (0 == strncmp(name, kSvcDescString.c_str(), kSvcDescString.size())) {
                            if (!lua_isstring(L_, -1)) {
                                valid2 = false;
                                break;
                            }

                            const char *sd = (const char*)lua_tostring(L_, -1);
                            if (sd == NULL) {
                                valid2 = false;
                                break;
                            }

                            svc_desc = sd;
                            break;
                        }
                    } while (0);

                    lua_pop(L_, 1);

                    if (!valid2) {
                        valid1 = false;
                        break;
                    }                 
                }
                if (svc_code != -1 && !svc_desc.empty()) {
                    capability.emplace_back(svc_code, svc_desc);
                }
            } while (0);

            lua_pop(L_, 1);

            if (!valid1) {
                ret = false;
                break;
            }
        }
    } while (0);

    if (capability.empty()) {
        ret = false;
    }
    return ret;
}

int32_t LuaEngine::lua_bee_log(lua_State *L) {
    int32_t args_len = lua_gettop(L);
    if (4 != args_len) {
        return luaL_error(L, "Invalid args.");
    }

    if (!lua_isstring(L, -1)) {
        return luaL_error(L, "Stack pos -1 is not string.");
    }

    const char *log = lua_tostring(L, -1);
    if (log == NULL) {
        return luaL_error(L, "Log NULL.");
    }

    if (!lua_isinteger(L, -2)) {
        return luaL_error(L, "Stack pos -2 is not int.");
    }
    int32_t line_number = (int32_t)lua_tointeger(L, -2);

    if (!lua_isstring(L, -3)) {
        return luaL_error(L, "Stack pos -3 is not string.");
    }
    const char *source = lua_tostring(L, -3);
    if (source == NULL) {
        return luaL_error(L, "Log source NULL.");
    }

    if (!lua_isinteger(L, -4)) {
        return luaL_error(L, "Stack pos -4 is not int.");
    }
    int32_t level = (int32_t)lua_tointeger(L, -4);
    if (level < 0 || level >= kLogLevel_All) {
        level = kLogLevel_Debug;
    }
    BeeLogLevel lv = static_cast<BeeLogLevel>(level);
    Logger::Log("lua", lv, source, line_number, "%s\n", log);
    lua_settop(L, 0);

    return 0;
}

int32_t LuaEngine::lua_get_tick_count32(lua_State *L) {
    TimeType tick_count =  MillisecTimer::get_tickcount();
    lua_pushinteger(L, (int32_t)(tick_count & 0xFFFFFFFF));
    return 1;
}

int32_t LuaEngine::lua_xxtea_encrypt(lua_State *L) {
    int32_t n = lua_gettop(L);
    if (2 != n) {
        return luaL_error(L, "Invalid args.");
    }

    const char *key = lua_tostring(L, -2);
    if (key == NULL) {
        return luaL_error(L, "Encrypt key NULL.");
    }

    size_t input_len = 0;
    const char *input = lua_tolstring(L, -1, &input_len);
    if (input == NULL || input_len == 0) {
        return luaL_error(L, "Encrypt input NULL.");
    }

    size_t output_len = 0;
    char *output = xxtea_encrypt(input, input_len, key, &output_len);
    if (output == NULL || output_len == 0) {
        return luaL_error(L, "Encrypt output NULL.");
    }

    lua_settop(L, 0);
    lua_pushlstring(L, output, output_len);
    free(output);

    return 1;
}

int32_t LuaEngine::lua_xxtea_decrypt(lua_State *L) {
    int32_t n = lua_gettop(L);
    if (2 != n) {
        return luaL_error(L, "Invalid args.");
    }

    const char *key = lua_tostring(L, -2);
    if (key == NULL) {
        return luaL_error(L, "Decrypt key NULL.");
    }

    size_t input_len = 0;
    const char *input = lua_tolstring(L, -1, &input_len);
    if (input == NULL || input_len == 0) {
        return luaL_error(L, "Decrypt input NULL.");
    }

    size_t output_len = 0;
    char *output = xxtea_decrypt(input, input_len, key, &output_len);
    if (output == NULL || output_len == 0) {
        return luaL_error(L, "Decrypt output NULL.");
    }

    lua_settop(L, 0);
    lua_pushlstring(L, output, output_len);
    free(output);

    return 1;
}

int32_t LuaEngine::lua_base64_encode(lua_State *L) {
    int32_t n = lua_gettop(L);
    if (1 != n) {
        return luaL_error(L, "Invalid args.");
    }

    size_t input_len = 0;
    const char *input = lua_tolstring(L, -1, &input_len);
    if (input == NULL || input_len == 0) {
        return luaL_error(L, "Base64 encode input NULL.");
    }

    std::string output = base64_encode((unsigned char*)input, input_len);
    if (output.empty()) {
        return luaL_error(L, "Base64 encode output empty.");
    }

    lua_settop(L, 0);
    lua_pushlstring(L, output.c_str(), output.size());

    return 1;
}

int32_t LuaEngine::lua_base64_decode(lua_State *L) {
    int32_t n = lua_gettop(L);
    if (1 != n) {
        return luaL_error(L, "Invalid args.");
    }

    size_t input_len = 0;
    const char *input = lua_tolstring(L, -1, &input_len);
    if (input == NULL || input_len == 0) {
        return luaL_error(L, "Base64 decode input NULL.");
    }

    size_t output_len = 0;
    std::string output = base64_decode((unsigned char*)input, input_len);
    if (output.empty()) {
        return luaL_error(L, "Base64 decode output empty.");
    }

    lua_settop(L, 0);
    lua_pushlstring(L, output.c_str(), output.size());

    return 1;
}

int32_t LuaEngine::lua_url_encode(lua_State *L) {
    int32_t n = lua_gettop(L);
    if (1 != n) {
        return luaL_error(L, "Invalid args.");
    }

    size_t input_len = 0;
    const char *input = lua_tolstring(L, -1, &input_len);
    if (input == NULL || input_len == 0) {
        return luaL_error(L, "Url encode input NULL.");
    }

    char *output = new char[3 * input_len];
    size_t i = 0;
    size_t j = 0;
    const char characters[] = "0123456789ABCDEF";
    for (i = 0; i < input_len; i++, j++) {
        char c = input[i];
        if ((c < '0' && c != '-' && c != '.') ||
            (c < 'A' && c > '9') ||
            (c > 'Z' && c < 'a' && c != '_') ||
            (c > 'z')) {
            output[j++] = '%';
            output[j++] = characters[(input[i] >> 4) & 0x0f];
            output[j] = characters[input[i] & 0x0f];
        }
        else {
            output[j] = input[i];
        }
    }

    output[j] = '\0';
    lua_settop(L, 0);
    lua_pushlstring(L, output, j);
    delete[] output;

    return 1;
}

int32_t LuaEngine::lua_forward_pointer(lua_State *L) {
    int32_t args_len = lua_gettop(L);
    if (2 != args_len) {
        return luaL_error(L, "Invalid args.");
    }

    int32_t ptr_offset = 1;
    int32_t off_offset = 2;
    if (!lua_islightuserdata(L, ptr_offset) || !lua_isinteger(L, off_offset)) {
        return luaL_error(L, "Wrong param data type.");
    }

    char *ptr = (char *)lua_topointer(L, ptr_offset);
    int32_t offset = (int32_t)lua_tointeger(L, off_offset);
    char *fptr = ptr + offset;

    lua_settop(L, 0);
    lua_pushlightuserdata(L, fptr);

    return 1;
}

int32_t LuaEngine::lua_configure_net_log(lua_State *L) {
    int32_t args_len = lua_gettop(L);
    if (2 != args_len) {
        return luaL_error(L, "Invalid args.");
    }

    int32_t log_level_offset = 1;
    int32_t time_base_offset = 2;
    if (!lua_isnumber(L, log_level_offset) || !lua_isnumber(L, time_base_offset)) {
        return luaL_error(L, "Wrong param data type.");
    }

    int32_t log_level = (int32_t)lua_tointeger(L, log_level_offset);
    TimeType time_base = (TimeType)lua_tonumber(L, time_base_offset);
    BeeEntrance::instance()->configure_net_log(log_level, time_base);
    return 0;
}

int32_t LuaEngine::lua_svc_notify(lua_State *L) {
    int32_t args_len = lua_gettop(L);
    if (2 != args_len) {
        return luaL_error(L, "Invalid args.");
    }

    int32_t svc_offset = 1;
    int32_t data_offset = 2;
    if (!lua_isinteger(L, svc_offset) || !lua_isstring(L, data_offset)) {
        return luaL_error(L, "Wrong param data type.");
    }

    int32_t svc_code = (int32_t)lua_tointeger(L, svc_offset);

    size_t size = 0;
    char *data = (char *)lua_tolstring(L, data_offset, &size);
    if (data == NULL || size == 0) {
        return luaL_error(L, "Data NULL.");
    }

    lua_getglobal(L, "BEE_TABLE");
    lua_pushstring(L, "bee_session");
    lua_gettable(L, -2);
    BeeSession *session = (BeeSession *)lua_topointer(L, -1);
    if (session != NULL) { //Honor promise whatever lua return.
        session->on_svc_notify(svc_code, data, size);
    }
    return 0;
}

} // namespace bee
