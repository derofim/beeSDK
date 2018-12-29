#include "lua_webrtc_module.h"
#include "lua_webrtc_service.h"
#include "lua_webrtc_stats_log_observer.h"
#include "session/bee_session.h"
#include "bee/media/audio_source.h"
#include "bee/media/video_source.h"
#include "bee/media/video_renderer.h"

#ifdef IOS
#include "webrtc/rtc_base/ssladapter.h"
#include "webrtc/rtc_base/bind.h"
#include "webrtc/system_wrappers/include/field_trial_default.h"
#elif defined(WIN32)
#include "webrtc/rtc_base/ssladapter.h"
#else
#include "webrtc/base/ssladapter.h"
#endif

namespace bee {

LuaWebrtcModule::LuaWebrtcModule() {

}

LuaWebrtcModule::~LuaWebrtcModule() {

}

int32_t LuaWebrtcModule::lua_open_module(lua_State *l) {
    luaL_Reg reg[] = {
        { "open",                       LuaWebrtcModule::lua_open },
        { "close",                      LuaWebrtcModule::lua_close },
        { "create_offer",               LuaWebrtcModule::lua_create_offer },
        { "create_answer",              LuaWebrtcModule::lua_create_answer },
        { "set_remote_desc",            LuaWebrtcModule::lua_set_remote_dec },
        { "start_video_renderer",       LuaWebrtcModule::lua_start_video_renderer },
        { "stop_video_renderer",        LuaWebrtcModule::lua_stop_video_renderer },
        { "enable_tracing",             LuaWebrtcModule::lua_enable_tracing },
        { "get_stats",                  LuaWebrtcModule::lua_get_stats },
        { "set_video_source",			LuaWebrtcModule::lua_set_video_source },
		{ "set_audio_source",			LuaWebrtcModule::lua_set_audio_source },
		{ "get_audio_input_level",		LuaWebrtcModule::lua_get_audio_input_level },
		{ "get_audio_output_level",		LuaWebrtcModule::lua_get_audio_output_level },
        { NULL,                         NULL }
    };
#ifdef IOS
    static const std::string filed_trial = "WebRTC-H264HighProfile/Enabled/";
    webrtc::field_trial::InitFieldTrialsFromString(filed_trial.c_str());
#endif
    
    rtc::InitializeSSL();

    std::string name = "webrtc";
    std::string version = "57";
    ModuleWrapper<LuaWebrtcModule> *wrapper = NULL;
    int32_t ret = LuaModuleBase::open_module(l, reg, name, version, &wrapper);
    if (ret != -1 && wrapper != NULL) {
        wrapper->module->webrtc_service_.reset(new LuaWebrtcService());   
    }

    return ret;
}

int32_t LuaWebrtcModule::lua_open(lua_State *l) {
    if (!check_args(l, 2)) {
        return luaL_error(l, "Invalid args.");
    }

    ModuleWrapper<LuaWebrtcModule> *wrapper = get_module_wrapper(l);
    if (wrapper == NULL) {
        return luaL_error(l, "Wrapper NULL.");
    }

    if (!wrapper->module->is_running()) {
        return luaL_error(l, "Webrtc service not running.");
    }
       
    int32_t stun_offset = 1;
    int32_t callbacks_offset = 2;
    if (!lua_isstring(l, stun_offset) || 
        !lua_istable(l, callbacks_offset)) {
        return luaL_error(l, "Wrong param data type.");
    }

    const char *stun_uri = lua_tostring(l, stun_offset);
    if (stun_uri == NULL) {
        return luaL_error(l, "Stun uri not set.");
    }       

    int32_t callbacks[eLuaWebrtcCallbackType_Count] = { LUA_REFNIL };
    lua_pushnil(l);
    while (lua_next(l, -2) != 0) {
        bool valid = true;
        do {
            if (!lua_isstring(l, -2)) {
                valid = false;
                break;
            }
            const char *key = lua_tostring(l, -2);
            if (key == NULL) {
                valid = false;
                break;
            }

            int callback = luaL_ref(l, LUA_REGISTRYINDEX); //This function will pop stack top, so do not call lua_pop.
            if (callback == LUA_REFNIL) {
                break;
            }

            if (strcmp(key, "on_ice_candidate") == 0) {
                callbacks[eLuaWebrtcCallbackType_OnIceCandidate] = callback;
            } else if (strcmp(key, "on_ice_gathering_change") == 0) {
                callbacks[eLuaWebrtcCallbackType_OnIceGatheringChange] = callback;
            } else if (strcmp(key, "on_ice_connection_change") == 0) {
                callbacks[eLuaWebrtcCallbackType_OnIceConnectionChange] = callback;
            } else if (strcmp(key, "on_media_ready") == 0) {
                callbacks[eLuaWebrtcCallbackType_OnMediaReady] = callback;
            } else if (strcmp(key, "on_video_frame") == 0) {
                callbacks[eLuaWebrtcCallbackType_OnVideoFrame] = callback;
            }
        } while (0);
            
        if (!valid) {
            lua_pop(l, 1);
        }
    }

    if (wrapper->module->ios_ == NULL) {
        lua_getglobal(l, "BEE_TABLE");
        lua_pushstring(l, "bee_session");
        lua_gettable(l, -2);

        BeeSession *s = (BeeSession *)lua_topointer(l, -1);
        if (s == NULL) {
            return luaL_error(l, "No session in table.");
        } else {
            wrapper->module->webrtc_service_->set_session_id(s->get_session_id());
            wrapper->module->ios_ = s->get_ios();
            if (wrapper->module->ios_ == NULL) {
                return luaL_error(l, "Webrtc ios not set.");
            }
        }
    }

    std::shared_ptr<LuaRtcPeerConnection> lua_peer_connection(new LuaRtcPeerConnection(wrapper->module->ios_, wrapper->module->webrtc_service_, wrapper->module->get_main_thread(), l));
    rtc::Thread *signaling_thread = wrapper->module->webrtc_service_->signaling_thread();
    if (signaling_thread == NULL) {
        lua_peer_connection.reset();
        return luaL_error(l, "Signaling thread not started.");
    }

    BeeErrorCode ec = signaling_thread->Invoke<BeeErrorCode>(
        RTC_FROM_HERE,
        rtc::Bind(&LuaRtcPeerConnection::create_webrtc_peer_connection, lua_peer_connection.get(), stun_uri));
    if (ec != kBeeErrorCode_Success) {
        lua_peer_connection.reset();
        return luaL_error(l, "Create peerconnection fail, error %d.", ec);
    }

    for (int32_t i = 0; i < eLuaWebrtcCallbackType_Count && callbacks[i] != LUA_REFNIL; ++i) {
        lua_peer_connection->set_lua_callback(static_cast<LuaWebrtcCallbackType>(i), callbacks[i]);
    }

    uint32_t handle = wrapper->module->new_lua_obj(lua_peer_connection);
    lua_settop(l, 0);
    lua_pushinteger(l, (lua_Integer)handle);
    lua_peer_connection.reset();
    return 1;
}

int32_t LuaWebrtcModule::lua_close(lua_State *l) {
    if (!check_args(l, 1)) {
        return luaL_error(l, "Invalid args.");
    }

    ModuleWrapper<LuaWebrtcModule> *wrapper = get_module_wrapper(l);
    if (wrapper == NULL) {
        return luaL_error(l, "Wrapper NULL.");
    }

    if (!wrapper->module->is_running()) {
        return luaL_error(l, "Webrtc service not running.");
    }

    int32_t handle_offset = 1;
    if (!lua_isinteger(l, handle_offset)) {
        return luaL_error(l, "Wrong param data type.");
    }

    int32_t handle = (int32_t)lua_tointeger(l, handle_offset);
    LuaRtcPeerConnection::Ptr lua_peer_connection = wrapper->module->delete_lua_obj(handle);
    if (lua_peer_connection == NULL) {
        return luaL_error(l, "Can't find peerconnection from handle %d.", handle);
    }
    
    lua_peer_connection->close();
    lua_peer_connection.reset();
    return 0;
}

int32_t LuaWebrtcModule::lua_create_offer(lua_State *l) {
    if (!check_args(l, 4)) {
        return luaL_error(l, "Invalid args.");
    }

    ModuleWrapper<LuaWebrtcModule> *wrapper = get_module_wrapper(l);
    if (wrapper == NULL) {
        return luaL_error(l, "Wrapper NULL.");
    }

    if (!wrapper->module->is_running()) {
        return luaL_error(l, "Webrtc service not running.");
    }

    int32_t handle_offset = 1;
    int32_t audio_source_offset = 2;
    int32_t video_source_offset = 3;
    if (!lua_isinteger(l, handle_offset) ||
        !lua_isnumber(l, audio_source_offset) ||
        !lua_isnumber(l, video_source_offset)) {
        return luaL_error(l, "Wrong param data type.");
    }

    int32_t handle = (int32_t)lua_tointeger(l, handle_offset);
    LuaRtcPeerConnection::Ptr lua_peer_connection = wrapper->module->get_obj(handle);
    if (lua_peer_connection == NULL) {
        return luaL_error(l, "Can't find peerconnection from handle %d.", handle);
    }

    AudioSource *audio_source = (AudioSource*)(int64_t)lua_tonumber(l, audio_source_offset);
    VideoSource *video_source = (VideoSource*)(int64_t)lua_tonumber(l, video_source_offset);

    int32_t callback = (int32_t)luaL_ref(l, LUA_REGISTRYINDEX);
    if (callback == LUA_REFNIL) {
        lua_peer_connection.reset();
        return luaL_error(l, "create_offer callback null.");
    }

    BeeErrorCode ec = lua_peer_connection->create_offer(audio_source, video_source, callback);
    Logger::TR("LuaWebrtcModule", "create_offer return %d.\n", ec);
    if (ec != kBeeErrorCode_Success) {
        lua_peer_connection.reset();
        return luaL_error(l, "Inner create_offer fail, error %d.", ec);
    }

    lua_peer_connection.reset();
    return 0;
}

int32_t LuaWebrtcModule::lua_create_answer(lua_State *l) {
    if (!check_args(l, 3)) {
        return luaL_error(l, "Invalid args.");
    }

    ModuleWrapper<LuaWebrtcModule> *wrapper = get_module_wrapper(l);
    if (wrapper == NULL) {
        return luaL_error(l, "Wrapper NULL.");
    }

    if (!wrapper->module->is_running()) {
        return luaL_error(l, "Webrtc service not running.");
    }

    int32_t handle_offset = 1;
    int32_t jsep_offset = 2;
    if (!lua_isinteger(l, handle_offset) || !lua_isstring(l, jsep_offset)) {
        return luaL_error(l, "Wrong param data type.");
    }

    int32_t handle = (int32_t)lua_tointeger(l, handle_offset);
    LuaRtcPeerConnection::Ptr lua_peer_connection = wrapper->module->get_obj(handle);
    if (lua_peer_connection == NULL) {
        return luaL_error(l, "Can't find peerconnection from handle %d.", handle);
    }

    const char *jsep = lua_tostring(l, jsep_offset);
    if (jsep == NULL) {
        lua_peer_connection.reset();
        return luaL_error(l, "create_answer fail, no jsep.");
    }

    int32_t callback = (int32_t)luaL_ref(l, LUA_REGISTRYINDEX);
    if (callback == LUA_REFNIL) {
        lua_peer_connection.reset();
        return luaL_error(l, "create_answer callback null.");
    }

    BeeErrorCode ec = lua_peer_connection->create_answer(jsep, callback);
    if (ec != kBeeErrorCode_Success) {
        lua_peer_connection.reset();
        return luaL_error(l, "Inner create_answer fail, error %d.", ec);
    }

    lua_peer_connection.reset();
    return 0;
}

int32_t LuaWebrtcModule::lua_set_remote_dec(lua_State *l) {
    if (!check_args(l, 2)) {
        return luaL_error(l, "Invalid args.");
    }

    ModuleWrapper<LuaWebrtcModule> *wrapper = get_module_wrapper(l);
    if (wrapper == NULL) {
        return luaL_error(l, "Wrapper NULL.");
    }

    if (!wrapper->module->is_running()) {
        return luaL_error(l, "Webrtc service not running.");
    }

    int32_t handle_offset = 1;
    int32_t jsep_offset = 2;
    if (!lua_isinteger(l, handle_offset) || !lua_isstring(l, jsep_offset)) {
        return luaL_error(l, "Wrong param data type.");
    }

    int32_t handle = (int32_t)lua_tointeger(l, handle_offset);
    LuaRtcPeerConnection::Ptr lua_peer_connection = wrapper->module->get_obj(handle);
    if (lua_peer_connection == NULL) {
        return luaL_error(l, "Can't find peerconnection from handle %d.", handle);
    }

    const char *jsep = lua_tostring(l, jsep_offset);
    if (jsep == NULL) {
        lua_peer_connection.reset();
        return luaL_error(l, "lua_set_remote_dec fail, no jsep.");
    }

    BeeErrorCode ec = lua_peer_connection->set_remote_desc(jsep);
    if (ec != kBeeErrorCode_Success) {
        lua_peer_connection.reset();
        return luaL_error(l, "Inner set_remote_desc fail, error %d.", ec);
    }

    lua_peer_connection.reset();
    return 0;
}

int32_t LuaWebrtcModule::lua_start_video_renderer(lua_State *l) {
    if (!check_args(l, 2)) {
        return luaL_error(l, "Invalid args.");
    }

    ModuleWrapper<LuaWebrtcModule> *wrapper = get_module_wrapper(l);
    if (wrapper == NULL) {
        return luaL_error(l, "Wrapper NULL.");
    }

    if (!wrapper->module->is_running()) {
        return luaL_error(l, "Webrtc service not running.");
    }

    int32_t handle_offset = 1;
    int32_t renderer_offset = 2;
    if (!lua_isinteger(l, handle_offset) || !lua_isnumber(l, renderer_offset)) {
        return luaL_error(l, "Wrong param data type.");
    }

    int32_t handle = (int32_t)lua_tointeger(l, handle_offset);
    LuaRtcPeerConnection::Ptr lua_peer_connection = wrapper->module->get_obj(handle);
    if (lua_peer_connection == NULL) {
        return luaL_error(l, "Can't find peerconnection from handle %d.", handle);
    }

    VideoRenderer *video_renderer = (VideoRenderer*)(int64_t)lua_tonumber(l, renderer_offset);    
    if (video_renderer == NULL) {
        lua_peer_connection.reset();
        return luaL_error(l, "StartVideoRender video renderer null.");
    }

    BeeErrorCode ec = lua_peer_connection->start_video_render(video_renderer);
    if (ec != kBeeErrorCode_Success) {
        lua_peer_connection.reset();
        return luaL_error(l, "Inner StartVideoRender fail, error %d.", ec);
    }

    lua_peer_connection.reset();
    return 0;
}

int32_t LuaWebrtcModule::lua_stop_video_renderer(lua_State *l) {
    if (!check_args(l, 1)) {
        return luaL_error(l, "Invalid args.");
    }

    ModuleWrapper<LuaWebrtcModule> *wrapper = get_module_wrapper(l);
    if (wrapper == NULL) {
        return luaL_error(l, "Wrapper NULL.");
    }

    if (!wrapper->module->is_running()) {
        return luaL_error(l, "Webrtc service not running.");
    }

    int32_t handle_offset = 1;
    if (!lua_isinteger(l, handle_offset)) {
        return luaL_error(l, "Wrong param data type.");
    }

    int32_t handle = (int32_t)lua_tointeger(l, handle_offset);
    LuaRtcPeerConnection::Ptr lua_peer_connection = wrapper->module->get_obj(handle);
    if (lua_peer_connection == NULL) {
        return luaL_error(l, "Can't find peerconnection from handle %d.", handle);
    }

    BeeErrorCode ec = lua_peer_connection->stop_video_render();
    if (ec != kBeeErrorCode_Success) {
        lua_peer_connection.reset();
        return luaL_error(l, "Inner StopVideoRender fail, error %d.", ec);
    }

    lua_peer_connection.reset();
    return 0;
}

int32_t LuaWebrtcModule::lua_enable_tracing(lua_State *l) {
    if (!check_args(l, 1)) {
        return luaL_error(l, "Invalid args.");
    }

    ModuleWrapper<LuaWebrtcModule> *wrapper = get_module_wrapper(l);
    if (wrapper == NULL) {
        return luaL_error(l, "Wrapper NULL.");
    }

    if (!wrapper->module->is_running()) {
        return luaL_error(l, "Webrtc service not running.");
    }

    int32_t level_offset = 1;
    if (!lua_isinteger(l, level_offset)) {
        return luaL_error(l, "Wrong param data type.");
    }

    int32_t level = (int32_t)lua_tointeger(l, level_offset);
    if (level <= rtc::LS_NONE && level >= rtc::LS_SENSITIVE) {
        rtc::LoggingSeverity rtc_log_level = static_cast<rtc::LoggingSeverity>(level);
        wrapper->module->log_sink_.reset(new WebrtcLogSink);
        rtc::LogMessage::AddLogToStream(wrapper->module->log_sink_.get(), rtc_log_level);
    }

    return 0;
}

int32_t LuaWebrtcModule::lua_get_stats(lua_State *l) {
    if (!check_args(l, 2)) {
        return luaL_error(l, "Invalid args.");
    }

    ModuleWrapper<LuaWebrtcModule> *wrapper = get_module_wrapper(l);
    if (wrapper == NULL) {
        return luaL_error(l, "Wrapper NULL.");
    }

    if (!wrapper->module->is_running()) {
        return luaL_error(l, "Webrtc service not running.");
    }

    int32_t handle_offset = 1;
    int32_t obsvr_offset = 2;
    if (!lua_isinteger(l, handle_offset) || !lua_isnumber(l, obsvr_offset)) {
        return luaL_error(l, "Wrong param data type.");
    }

    int32_t handle = (int32_t)lua_tointeger(l, handle_offset);
    LuaRtcPeerConnection::Ptr lua_peer_connection = wrapper->module->get_obj(handle);
    if (lua_peer_connection == NULL) {
        return luaL_error(l, "Can't find peerconnection from handle %d.", handle);
    }

    webrtc::StatsObserver *observer = (webrtc::StatsObserver*)(int64_t)lua_tonumber(l, obsvr_offset);
    if (observer == NULL) {
        observer = new rtc::RefCountedObject<LuaWebRTCStatsLogObserver>();
    }

    rtc::scoped_refptr<webrtc::StatsObserver> observer_ptr(observer);
    BeeErrorCode ec = lua_peer_connection->get_stats(observer);
    if (ec != kBeeErrorCode_Success) {
        lua_peer_connection.reset();
        return luaL_error(l, "Inner GetStats fail, error %d.", ec);
    }

    lua_peer_connection.reset();
    return 0;
}

int32_t LuaWebrtcModule::lua_set_video_source(lua_State *l) {
    if (!check_args(l, 7)) {
        return luaL_error(l, "Invalid args.");
    }

    ModuleWrapper<LuaWebrtcModule> *wrapper = get_module_wrapper(l);
    if (wrapper == NULL) {
        return luaL_error(l, "Wrapper NULL.");
    }

    if (!wrapper->module->is_running()) {
        return luaL_error(l, "Webrtc service not running.");
    }

    int32_t handle_offset = 1;
    int32_t internal_offset = 2;
    int32_t width_offset = 3;
    int32_t height_offset = 4;
    int32_t fps_offset = 5;
    int32_t capturer_index_offset = 6;
    int32_t is_screencast_offset = 7;

    if (!lua_isinteger(l, handle_offset) ||
        !lua_isboolean(l, internal_offset) ||
        !lua_isnumber(l, width_offset) ||
        !lua_isnumber(l, height_offset) ||
        !lua_isnumber(l, fps_offset) ||
        !lua_isnumber(l, capturer_index_offset) ||
        !lua_isboolean(l, is_screencast_offset)) {
        return luaL_error(l, "Wrong param data type.");
    }

    int32_t handle = (int32_t)lua_tointeger(l, handle_offset);
    LuaRtcPeerConnection::Ptr lua_peer_connection = wrapper->module->get_obj(handle);
    if (lua_peer_connection == NULL) {
        return luaL_error(l, "Can't find peerconnection from handle %d.", handle);
    }

    bool internal = lua_toboolean(l, internal_offset) == 0 ? false : true;
    int32_t width = (int32_t)lua_tointeger(l, width_offset);
    int32_t height = (int32_t)lua_tonumber(l, height_offset);
    int32_t fps = (int32_t)lua_tonumber(l, fps_offset);
    int32_t captuer_index = (int32_t)lua_tonumber(l, capturer_index_offset);
    bool is_screencast = lua_toboolean(l, is_screencast_offset) == 0 ? false : true;

    BeeErrorCode ec = lua_peer_connection->set_video_source(internal, width, height, fps, captuer_index, is_screencast);
    if (ec != kBeeErrorCode_Success) {
        lua_peer_connection.reset();
        return luaL_error(l, "Inner set video source fail, error %d.", ec);
    }

    lua_peer_connection.reset();
    return 0;
}

int32_t LuaWebrtcModule::lua_set_audio_source(lua_State *l) {
	if (!check_args(l, 3)) {
		return luaL_error(l, "Invalid args.");
	}

	ModuleWrapper<LuaWebrtcModule> *wrapper = get_module_wrapper(l);
	if (wrapper == NULL) {
		return luaL_error(l, "Wrapper NULL.");
	}

	if (!wrapper->module->is_running()) {
		return luaL_error(l, "Webrtc service not running.");
	}

	int32_t handle_offset = 1;
	int32_t no_audio_processing_offset = 2;
	int32_t enable_level_control_offset = 3;

	if (!lua_isinteger(l, handle_offset) ||
		!lua_isboolean(l, no_audio_processing_offset) ||
		!lua_isboolean(l, enable_level_control_offset)) {
		return luaL_error(l, "Wrong param data type.");
	}

	int32_t handle = (int32_t)lua_tointeger(l, handle_offset);
	LuaRtcPeerConnection::Ptr lua_peer_connection = wrapper->module->get_obj(handle);
	if (lua_peer_connection == NULL) {
		return luaL_error(l, "Can't find peerconnection from handle %d.", handle);
	}

	bool no_audio_processing = lua_toboolean(l, no_audio_processing_offset) == 0 ? false : true;
	bool enable_level_control = lua_toboolean(l, enable_level_control_offset) == 0 ? false : true;

	BeeErrorCode ec = lua_peer_connection->set_audio_source(no_audio_processing, enable_level_control);
	if (ec != kBeeErrorCode_Success) {
		lua_peer_connection.reset();
		return luaL_error(l, "Inner set audio source fail, error %d.", ec);
	}

	lua_peer_connection.reset();
	return 0;
}

int32_t LuaWebrtcModule::lua_get_audio_input_level(lua_State *l) {
	if (!check_args(l, 1)) {
		return luaL_error(l, "Invalid args.");
	}

	ModuleWrapper<LuaWebrtcModule> *wrapper = get_module_wrapper(l);
	if (wrapper == NULL) {
		return luaL_error(l, "Wrapper NULL.");
	}

	if (!wrapper->module->is_running()) {
		return luaL_error(l, "Webrtc service not running.");
	}

	int32_t handle_offset = 1;

	if (!lua_isinteger(l, handle_offset)) {
		return luaL_error(l, "Wrong param data type.");
	}

	int32_t handle = (int32_t)lua_tointeger(l, handle_offset);
	LuaRtcPeerConnection::Ptr lua_peer_connection = wrapper->module->get_obj(handle);
	if (lua_peer_connection == NULL) {
		return luaL_error(l, "Can't find peerconnection from handle %d.", handle);
	}

	int32_t audio_level = lua_peer_connection->get_audio_input_level();
	lua_settop(l, 0);
	lua_pushinteger(l, (lua_Integer)audio_level);
	lua_peer_connection.reset();
	return 1;
}

int32_t LuaWebrtcModule::lua_get_audio_output_level(lua_State *l) {
	if (!check_args(l, 1)) {
		return luaL_error(l, "Invalid args.");
	}

	ModuleWrapper<LuaWebrtcModule> *wrapper = get_module_wrapper(l);
	if (wrapper == NULL) {
		return luaL_error(l, "Wrapper NULL.");
	}

	if (!wrapper->module->is_running()) {
		return luaL_error(l, "Webrtc service not running.");
	}

	int32_t handle_offset = 1;

	if (!lua_isinteger(l, handle_offset)) {
		return luaL_error(l, "Wrong param data type.");
	}

	int32_t handle = (int32_t)lua_tointeger(l, handle_offset);
	LuaRtcPeerConnection::Ptr lua_peer_connection = wrapper->module->get_obj(handle);
	if (lua_peer_connection == NULL) {
		return luaL_error(l, "Can't find peerconnection from handle %d.", handle);
	}

	int32_t audio_level = lua_peer_connection->get_audio_output_level();
	lua_settop(l, 0);
	lua_pushinteger(l, (lua_Integer)audio_level);
	lua_peer_connection.reset();
	return 1;
}

void LuaWebrtcModule::on_close_module() {
    if (!obj_table_.empty()) {
        auto iter = obj_table_.begin();
        for (; iter != obj_table_.end(); ++iter) {
            iter->second->obj->close();
        }
    }

    if (is_running()) {
        //Must remove here or next logging in webrtc will crash because log sink already released.
        if (log_sink_ != NULL) {
            rtc::LogMessage::RemoveLogToStream(log_sink_.get());
        }
        webrtc_service_.reset();
    }
    
    rtc::CleanupSSL();
}

bool LuaWebrtcModule::is_running() {
    return webrtc_service_ != NULL;
}

} // namespace bee
