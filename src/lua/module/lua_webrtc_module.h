#ifndef __LUA_WEBRTC_MODULE_H__
#define __LUA_WEBRTC_MODULE_H__

#include "lua_module_mgr.h"
#include "lua_webrtc_peer_connection.h"
#include "log/logger.h"

namespace bee {
//////////////////////////////////WebrtcLogSink////////////////////////////////////////
class WebrtcLogSink : public rtc::LogSink {
    void OnLogMessage(const std::string& message) {
        Logger::TR("webrtc", "%s", message.c_str());
    }
};

////////////////////////////////////LuaWebrtcModule//////////////////////////////////////
class LuaWebrtcService;
class LuaWebrtcModule : public LuaObjMgr<LuaRtcPeerConnection>, public LuaModuleBase<LuaWebrtcModule> {
public:
    LuaWebrtcModule();
    virtual ~LuaWebrtcModule();

public:
    static int32_t lua_open_module(lua_State *l);
    static int32_t lua_open(lua_State *l);
    static int32_t lua_close(lua_State *l);
    static int32_t lua_create_offer(lua_State *l);
    static int32_t lua_create_answer(lua_State *l);
    static int32_t lua_set_remote_dec(lua_State *l);
    static int32_t lua_start_video_renderer(lua_State *l);
    static int32_t lua_stop_video_renderer(lua_State *l);
    static int32_t lua_enable_tracing(lua_State *l);
    static int32_t lua_get_stats(lua_State *l);
    static int32_t lua_set_video_source(lua_State *l);
	static int32_t lua_set_audio_source(lua_State *l);
	static int32_t lua_get_audio_input_level(lua_State *l);
	static int32_t lua_get_audio_output_level(lua_State *l);

public:
    void on_close_module();
    bool is_running();

protected:
    std::shared_ptr<LuaWebrtcService> webrtc_service_;
    std::shared_ptr<WebrtcLogSink> log_sink_;
    IOSPtr ios_;
};

} // namespace bee

#endif
