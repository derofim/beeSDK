#include "lua_webrtc_peer_connection.h"
#include "lua_webrtc_service.h"
#include "lua_webrtc_video_sink.h"
#include "bee/media/audio_source.h"
#include "bee/media/video_source.h"
#include "internal/audio_source_internal.h"
#include "internal/video_source_internal.h"
#include "utility/json/json.h"
#include "webrtc/media/base/videocapturer.h"
#include "webrtc/api/mediastreaminterface.h"
#include "webrtc/api/peerconnectioninterface.h"
#include "webrtc/api/test/fakeconstraints.h"

namespace bee {

////////////////////////////////////LuaRtcPeerConnection//////////////////////////////////////
LuaRtcPeerConnection::LuaRtcPeerConnection(IOSPtr ios, std::shared_ptr<LuaWebrtcService> webrtc_service, lua_State *main, lua_State *co)
    : ios_(ios),
      webrtc_service_(webrtc_service),
      main_(main),
      co_(co),
      logger_("LuaRtcPeerConnection") {
    for (int32_t i = 0; i < eLuaWebrtcCallbackType_Count; ++i) {
        lua_callbacks_[i] = LUA_REFNIL;
    }
    logger_.Debug("[%x] LuaRtcPeerConnection created.\n", (unsigned int)(long)this);
}

LuaRtcPeerConnection::~LuaRtcPeerConnection() {
    if (video_receiver_ != NULL) {
        video_receiver_->stop();
        video_receiver_.reset();
    }
    logger_.Debug("[%x] LuaRtcPeerConnection deleted.\n", (unsigned int)(long)this);
}

BeeErrorCode LuaRtcPeerConnection::create_webrtc_peer_connection(const std::string &stun_uri) {
    BeeErrorCode ret = kBeeErrorCode_Success;
    do {
        if (webrtc_service_ == NULL) {
            ret = kBeeErrorCode_Service_Not_Started;
            break;
        }

        LuaWebrtcService::FactoryPtr factory = webrtc_service_->get_peer_connection_factory();
        if (factory == NULL) {
            ret = kBeeErrorCode_Null_Pointer;
            break;
        }

        webrtc::PeerConnectionInterface::RTCConfiguration config;
        webrtc::PeerConnectionInterface::IceServer server;
        server.uri = stun_uri;
        config.servers.push_back(server);
        //config.type = webrtc::PeerConnectionInterface::kRelay;

        webrtc::FakeConstraints constraints;
        constraints.AddOptional(webrtc::MediaConstraintsInterface::kEnableDtlsSrtp, "true");
#ifdef IOS
        config.enable_dtls_srtp = true;
#endif
        peer_connection_ = factory->CreatePeerConnection(config, &constraints, NULL, NULL, this);
        if (peer_connection_ == NULL) {
            ret = kBeeErrorCode_Webrtc_Create_Peer_Connection_Fail;
            break;
        }
        
        logger_.Info("[%x] PeerConnection created.\n", (unsigned int)(long)peer_connection_.get());
    } while (0);
    return ret;
}

BeeErrorCode LuaRtcPeerConnection::create_offer(AudioSource *audio_source, VideoSource *video_source, int32_t callback) {
    BeeErrorCode ret = kBeeErrorCode_Success;
    do {
        if (webrtc_service_ == NULL) {
            ret = kBeeErrorCode_Service_Not_Started;
            break;
        }

        rtc::Thread *signaling_thread = webrtc_service_->signaling_thread();
        if (signaling_thread == NULL) {
            ret = kBeeErrorCode_Service_Not_Started;
            break;
        }
        
        ret = signaling_thread->Invoke<BeeErrorCode>(
            RTC_FROM_HERE,
            rtc::Bind(&LuaRtcPeerConnection::inner_create_offer, this, audio_source, video_source));
        if (ret != kBeeErrorCode_Success) {
            break;
        }

        lua_callbacks_[eLuaWebrtcCallbackType_OnSdp] = callback;

        if (ios_ != NULL) {
            ios_->post(boost::bind(&LuaRtcPeerConnection::on_media_ready, shared_from_this()));
        }
    } while (0);
    return ret;
}

BeeErrorCode LuaRtcPeerConnection::create_answer(const std::string &jsep, int32_t callback) {
    BeeErrorCode ret = kBeeErrorCode_Success;
    do {
        if (webrtc_service_ == NULL) {
            ret = kBeeErrorCode_Service_Not_Started;
            break;
        }

        rtc::Thread *signaling_thread = webrtc_service_->signaling_thread();
        if (signaling_thread == NULL) {
            ret = kBeeErrorCode_Service_Not_Started;
            break;
        }

        ret = signaling_thread->Invoke<BeeErrorCode>( //Must be called in webrtc's thread(Signaling Thread);
            RTC_FROM_HERE,
            rtc::Bind(&LuaRtcPeerConnection::inner_create_answer, this, jsep));
        if (ret != kBeeErrorCode_Success) {
            break;
        }

        lua_callbacks_[eLuaWebrtcCallbackType_OnSdp] = callback;
    } while (0);
    return ret;
}

BeeErrorCode LuaRtcPeerConnection::set_remote_desc(const std::string &jsep) {
    BeeErrorCode ret = kBeeErrorCode_Success;
    do {
        if (webrtc_service_ == NULL) {
            ret = kBeeErrorCode_Service_Not_Started;
            break;
        }

        rtc::Thread *signaling_thread = webrtc_service_->signaling_thread();
        if (signaling_thread == NULL) {
            ret = kBeeErrorCode_Service_Not_Started;
            break;
        }

        ret = signaling_thread->Invoke<BeeErrorCode>( //Must be called in webrtc's thread(Signaling Thread);
            RTC_FROM_HERE,
            rtc::Bind(&LuaRtcPeerConnection::inner_set_remote_desc, this, jsep));
    } while (0);
    return ret;
}

BeeErrorCode LuaRtcPeerConnection::close() {
    BeeErrorCode ret = kBeeErrorCode_Success;
    do {
        closed_ = true;

        if (webrtc_service_ == NULL) {
            ret = kBeeErrorCode_Service_Not_Started;
            break;
        }

        rtc::Thread *signaling_thread = webrtc_service_->signaling_thread();
        if (signaling_thread == NULL) {
            ret = kBeeErrorCode_Service_Not_Started;
            break;
        }

        ret = signaling_thread->Invoke<BeeErrorCode>( //Must be called in webrtc's thread(Signaling Thread);
            RTC_FROM_HERE,
            rtc::Bind(&LuaRtcPeerConnection::inner_close, this));
        if (ret != kBeeErrorCode_Success) {
            break;
        }

        webrtc_service_.reset();
    } while (0);
    return ret;
}

BeeErrorCode LuaRtcPeerConnection::start_video_render(VideoRenderer *video_renderer) {
    BeeErrorCode ret = kBeeErrorCode_Success;
    do {
        if (video_receiver_ == NULL) {
            ret = kBeeErrorCode_Invalid_State;
            break;
        }

        video_receiver_->start(video_renderer);
    } while (0);
    return ret;
}

BeeErrorCode LuaRtcPeerConnection::stop_video_render() {
    BeeErrorCode ret = kBeeErrorCode_Success;
    do {
        if (video_receiver_ == NULL) {
            ret = kBeeErrorCode_Invalid_State;
            break;
        }

        video_receiver_->stop();
    } while (0);
    return ret;
}

BeeErrorCode LuaRtcPeerConnection::get_stats(webrtc::StatsObserver *observer) {
    BeeErrorCode ret = kBeeErrorCode_Success;
    do {
        if (observer == NULL) {
            ret = kBeeErrorCode_Invalid_Param;
            break;
        }

        if (webrtc_service_ == NULL) {
            ret = kBeeErrorCode_Service_Not_Started;
            break;
        }

        rtc::Thread *signaling_thread = webrtc_service_->signaling_thread();
        if (signaling_thread == NULL) {
            ret = kBeeErrorCode_Service_Not_Started;
            break;
        }

        ret = signaling_thread->Invoke<BeeErrorCode>(
                RTC_FROM_HERE,
                rtc::Bind(&LuaRtcPeerConnection::inner_get_stats, this, observer));
    } while (0);
    return ret;
}

BeeErrorCode LuaRtcPeerConnection::set_video_source(bool internal, int32_t width, int32_t height, int32_t fps, int32_t capturer_index, bool is_screencast) {
	BeeErrorCode ret = kBeeErrorCode_Success;
	do {
		if (webrtc_service_ == NULL) {
			ret = kBeeErrorCode_Service_Not_Started;
			break;
		}

		rtc::Thread *signaling_thread = webrtc_service_->signaling_thread();
		if (signaling_thread == NULL) {
			ret = kBeeErrorCode_Service_Not_Started;
			break;
		}

		signaling_thread->Invoke<void>(
			RTC_FROM_HERE,
			rtc::Bind(&LuaRtcPeerConnection::inner_set_video_source, this, internal, width, height, fps, capturer_index, is_screencast));
	} while (0);
	return ret;
}

BeeErrorCode LuaRtcPeerConnection::set_audio_source(bool no_audio_processing, bool enable_level_control) {
	BeeErrorCode ret = kBeeErrorCode_Success;
	do {
		if (webrtc_service_ == NULL) {
			ret = kBeeErrorCode_Service_Not_Started;
			break;
		}

		rtc::Thread *signaling_thread = webrtc_service_->signaling_thread();
		if (signaling_thread == NULL) {
			ret = kBeeErrorCode_Service_Not_Started;
			break;
		}

		signaling_thread->Invoke<void>(
			RTC_FROM_HERE,
			rtc::Bind(&LuaRtcPeerConnection::inner_set_audio_source, this, no_audio_processing, enable_level_control));
	} while (0);
	return ret;
}

void LuaRtcPeerConnection::set_lua_callback(LuaWebrtcCallbackType type, int32_t callback) {
    lua_callbacks_[type] = callback;
}

int32_t LuaRtcPeerConnection::get_audio_input_level() {
	int32_t ret = -1;
	do {
		if (webrtc_service_ == NULL) {
			break;
		}

		rtc::Thread *signaling_thread = webrtc_service_->signaling_thread();
		if (signaling_thread == NULL) {
			break;
		}

		ret = signaling_thread->Invoke<int32_t>(
			RTC_FROM_HERE,
			rtc::Bind(&LuaRtcPeerConnection::inner_get_audio_input_level, this));
	} while (0);
	return ret;
}

int32_t LuaRtcPeerConnection::get_audio_output_level() {
	int32_t ret = -1;
	do {
		if (webrtc_service_ == NULL) {
			break;
		}

		rtc::Thread *signaling_thread = webrtc_service_->signaling_thread();
		if (signaling_thread == NULL) {
			break;
		}

		ret = signaling_thread->Invoke<int32_t>(
			RTC_FROM_HERE,
			rtc::Bind(&LuaRtcPeerConnection::inner_get_audio_output_level, this));
	} while (0);
	return ret;
}

void LuaRtcPeerConnection::OnSignalingChange(webrtc::PeerConnectionInterface::SignalingState new_state) {

}

void LuaRtcPeerConnection::OnAddStream(rtc::scoped_refptr<webrtc::MediaStreamInterface> stream) {
    if (stream.get() != NULL) {
        webrtc::VideoTrackVector tracks = stream->GetVideoTracks();
        if (!tracks.empty() && video_receiver_ == NULL) {
            webrtc::VideoTrackInterface* track = tracks[0];
            video_receiver_.reset(new LuaWebrtcVideoSink(track, shared_from_this()));
        }

        if (ios_ != NULL) {
            ios_->post(boost::bind(&LuaRtcPeerConnection::on_media_ready, shared_from_this()));
        }
    }
}

void LuaRtcPeerConnection::OnRemoveStream(rtc::scoped_refptr<webrtc::MediaStreamInterface> stream) {
    if (video_receiver_ != NULL) {
        video_receiver_->stop();
        video_receiver_.reset();
    }
}

void LuaRtcPeerConnection::OnDataChannel(rtc::scoped_refptr<webrtc::DataChannelInterface> channel) {

}

void LuaRtcPeerConnection::OnRenegotiationNeeded() {

}

void LuaRtcPeerConnection::OnIceConnectionChange(webrtc::PeerConnectionInterface::IceConnectionState new_state) {
    logger_.Info("OnIceConnectionChange %d\n", new_state);
    if (ios_ != NULL) {
        ios_->post(boost::bind(&LuaRtcPeerConnection::on_local_ice_connection_changed, shared_from_this(), new_state));
    }
}

void LuaRtcPeerConnection::OnIceGatheringChange(webrtc::PeerConnectionInterface::IceGatheringState new_state) {
    if (ios_ != NULL) {
        ios_->post(boost::bind(&LuaRtcPeerConnection::on_local_ice_gathering_changed, shared_from_this(), new_state));
    }
}

void LuaRtcPeerConnection::OnIceCandidate(const webrtc::IceCandidateInterface* candidate) {
    BeeJson::StyledWriter writer;
    BeeJson::Value jmessage;

    jmessage[kCandidateSdpMidName] = candidate->sdp_mid();
    jmessage[kCandidateSdpMlineIndexName] = candidate->sdp_mline_index();
    std::string sdp;
    if (!candidate->ToString(&sdp)) {
        logger_.Error("Failed to serialize candidate.\n");
        return;
    }
    jmessage[kCandidateSdpName] = sdp;
    std::string local_candidate = writer.write(jmessage);

    if (ios_ != NULL) {
        ios_->post(boost::bind(&LuaRtcPeerConnection::on_local_ice_candidate, shared_from_this(), local_candidate));
    }
}

void LuaRtcPeerConnection::OnIceConnectionReceivingChange(bool receiving) {

}

void LuaRtcPeerConnection::OnSuccess(webrtc::SessionDescriptionInterface* desc) {
    logger_.Debug("Description create success\n");

    std::string sdp;
    desc->ToString(&sdp);
    BeeJson::StyledWriter writer;
    BeeJson::Value jmessage;
    jmessage[kSessionDescriptionTypeName] = desc->type();
    jmessage[kSessionDescriptionSdpName] = sdp;
    std::string jsep = writer.write(jmessage);
    ios_->post(boost::bind(&LuaRtcPeerConnection::on_get_sdp, shared_from_this(), jsep));

    peer_connection_->SetLocalDescription(DummySetSessionDescriptionObserver::Create(), desc);

}

void LuaRtcPeerConnection::OnFailure(const std::string& error) {
    if (ios_ != NULL) {
        ios_->post(boost::bind(&LuaRtcPeerConnection::on_get_sdp, shared_from_this(), ""));
    }
}

BeeErrorCode LuaRtcPeerConnection::inner_create_offer(AudioSource *audio_source, VideoSource *video_source) {
    BeeErrorCode ret = kBeeErrorCode_Success;
    do {
        if (webrtc_service_ == NULL) {
            ret = kBeeErrorCode_Service_Not_Started;
            break;
        }

        if (peer_connection_ == NULL) {
            ret = kBeeErrorCode_Null_Pointer;
            break;
        }

        LuaWebrtcService::FactoryPtr factory = webrtc_service_->get_peer_connection_factory();
        if (factory == NULL) {
            ret = kBeeErrorCode_Null_Pointer;
            break;
        }

        //Create media stream.
        rtc::scoped_refptr<webrtc::MediaStreamInterface> stream = factory->CreateLocalMediaStream(kStreamLabel);        
        if (stream == nullptr) {
            ret = kBeeErrorCode_Webrtc_Create_Local_Stream_Fail;
            break;
        }

        //Create audio track source.
        if (audio_source != NULL) {
            rtc::scoped_refptr<webrtc::AudioSourceInterface> rtc_audio_source = audio_source->audio_source_internal_->rtc_audio_track_source_;
            if (rtc_audio_source != NULL) {
                rtc::scoped_refptr<webrtc::AudioTrackInterface> audio_track = factory->CreateAudioTrack(kAudioLabel, rtc_audio_source);
                if (audio_track) {
                    stream->AddTrack(audio_track);
                }
            }
        }

        //Create video track source
        rtc::scoped_refptr<webrtc::VideoTrackInterface> video_track;
        if (video_source != NULL) {
            rtc::scoped_refptr<webrtc::VideoTrackSourceInterface> rtc_video_source = video_source->video_source_internal_->rtc_video_track_source_;
            if (rtc_video_source != NULL) {
                video_track = factory->CreateVideoTrack(kVideoLabel, rtc_video_source);
                if (video_track) {
                    stream->AddTrack(video_track);
                }
            }
        }

        bool add_ret = peer_connection_->AddStream(stream);
        if (!add_ret) {
            ret = kBeeErrorCode_Webrtc_Add_Stream_Fail;
            break;
        }

        role_ = ePartyRole_Offer;

        webrtc::PeerConnectionInterface::RTCOfferAnswerOptions option;
        option.offer_to_receive_audio = 0;
        option.offer_to_receive_video = 0;
        peer_connection_->CreateOffer(this, option);

        if (video_receiver_ == NULL && video_track) {
            video_receiver_.reset(new LuaWebrtcVideoSink(video_track, shared_from_this()));
        }
    } while (0);
    return ret;
}

BeeErrorCode LuaRtcPeerConnection::inner_create_answer(const std::string &jsep) {
    BeeErrorCode ret = kBeeErrorCode_Success;
    do {
        ret = inner_set_remote_desc(jsep);
        if (ret != kBeeErrorCode_Success) {
            break;
        }

        role_ = ePartyRole_Answer;

        webrtc::PeerConnectionInterface::RTCOfferAnswerOptions option;
        option.offer_to_receive_audio = 1;
        option.offer_to_receive_video = 1;
        peer_connection_->CreateAnswer(this, option);
    } while (0);
    return ret;
}

BeeErrorCode LuaRtcPeerConnection::inner_set_remote_desc(const std::string &jsep) {
    BeeErrorCode ret = kBeeErrorCode_Success;
    do {
        if (peer_connection_ == NULL) {
            ret = kBeeErrorCode_Null_Pointer;
            break;
        }

        BeeJson::Reader reader;
        BeeJson::Value jdata;
        if (!reader.parse(jsep, jdata)) {
            ret = kBeeErrorCode_Invalid_Param;
            break;
        }
        
        if (jdata[kTypeName].isNull()) {
            ret = kBeeErrorCode_Invalid_Param;
            break;
        }

        std::string type = jdata[kTypeName].asString();
        if (type.empty()) {
            ret = kBeeErrorCode_Invalid_Param;
            break;
        }

        if (jdata[kSessionDescriptionSdpName].isNull()) {
            ret = kBeeErrorCode_Invalid_Param;
            break;
        }

        std::string sdp = jdata[kSessionDescriptionSdpName].asString();
        if (sdp.empty()) {
            ret = kBeeErrorCode_Invalid_Param;
            break;
        }

        webrtc::SdpParseError error;
        webrtc::SessionDescriptionInterface* session_description(webrtc::CreateSessionDescription(type, sdp, &error));
        if (!session_description) {
            ret = kBeeErrorCode_Invalid_Param;
            break;
        }

        peer_connection_->SetRemoteDescription(DummySetSessionDescriptionObserver::Create(), session_description);
    } while (0);
    return ret;
}

BeeErrorCode LuaRtcPeerConnection::inner_close() {
    BeeErrorCode ret = kBeeErrorCode_Success;
    do {
        if (video_receiver_ != NULL) {
            video_receiver_->stop();
            video_receiver_.reset();
        }

        if (peer_connection_ != NULL) {
            peer_connection_->Close();
            peer_connection_ = NULL;
        }
        
        logger_.Info("PeerConnection closed.\n", (unsigned int)(long)peer_connection_.get());
    } while (0);
    return ret;
}

BeeErrorCode LuaRtcPeerConnection::inner_get_stats(webrtc::StatsObserver *observer) {
    BeeErrorCode ret = kBeeErrorCode_Success;
    do {
        if (peer_connection_ == NULL) {
            ret = kBeeErrorCode_Null_Pointer;
            break;
        }

        if (!peer_connection_->GetStats(observer, NULL, webrtc::PeerConnectionInterface::kStatsOutputLevelStandard)) {
            ret = kBeeErrorCode_Invalid_Param;
            break;
        }
    } while (0);
    return ret;
}

void LuaRtcPeerConnection::inner_set_video_source(bool internal, int32_t width, int32_t height, int32_t fps, int32_t capturer_index, bool is_screencast) {
	use_external_video_source_ = !internal;
	video_source_width_ = width;
	video_source_height_ = height;
	video_source_fps_ = fps;
	video_capturer_index_ = capturer_index;
    video_is_screencast_ = is_screencast;
}

void LuaRtcPeerConnection::inner_set_audio_source(bool no_audio_processing, bool enable_level_control) {
	no_audio_processing_ = no_audio_processing;
	enable_level_control_ = enable_level_control;
}

int32_t LuaRtcPeerConnection::inner_get_audio_input_level() {
	if (peer_connection_ != NULL) {
		return peer_connection_->GetInputAudioLevel();
	} else {
		return -1;
	}

    return -1;
}

int32_t LuaRtcPeerConnection::inner_get_audio_output_level() {
	if (peer_connection_ != NULL) {
		return peer_connection_->GetOutputAudioLevel();
	} else {
		return -1;
	}
}

void LuaRtcPeerConnection::on_get_sdp(const std::string &jsep) {
    do {
        if (closed_) {
            break;
        }

        if (main_ == NULL) {
            break;
        }

        int32_t callback = lua_callbacks_[eLuaWebrtcCallbackType_OnSdp];
        if (callback == LUA_REFNIL) {
            break;
        }
        
        lua_State *co = lua_newthread(main_);
        lua_rawgeti(co, LUA_REGISTRYINDEX, callback);
        lua_pushstring(co, jsep.c_str());
        int32_t ret = lua_resume(co, main_, 1);
        lua_pop(main_, 1);

        if (ret != LUA_OK && ret != LUA_YIELD) {
            logger_.Error("on_get_sdp failed.\n");
            int32_t args_len = lua_gettop(co);
            if (args_len > 0 && lua_type(co, -1) == LUA_TSTRING) {
                logger_.Error("on_get_sdp error %s.\n", lua_tostring(co, -1));
            }
        }
    } while (0);
}

void LuaRtcPeerConnection::on_local_ice_candidate(const std::string &candidate) {
    do {
        if (closed_) {
            break;
        }

        if (main_ == NULL) {
            break;
        }

        int32_t callback = lua_callbacks_[eLuaWebrtcCallbackType_OnIceCandidate];
        if (callback == LUA_REFNIL) {
            break;
        }
        
        lua_State *co = lua_newthread(main_);
        lua_rawgeti(co, LUA_REGISTRYINDEX, callback);
        lua_pushstring(co, candidate.c_str());
        int32_t ret = lua_resume(co, main_, 1);
        lua_pop(main_, 1);

        if (ret != LUA_OK && ret != LUA_YIELD) {
            logger_.Error("on_local_ice_candidate failed.\n");
            int32_t args_len = lua_gettop(co);
            if (args_len > 0 && lua_type(co, -1) == LUA_TSTRING) {
                logger_.Error("on_local_ice_candidate error %s.\n", lua_tostring(co, -1));
            }
        }
    } while (0);
}

void LuaRtcPeerConnection::on_local_ice_gathering_changed(webrtc::PeerConnectionInterface::IceGatheringState new_state) {
    do {
        if (closed_) {
            break;
        }

        if (main_ == NULL) {
            break;
        }

        int32_t callback = lua_callbacks_[eLuaWebrtcCallbackType_OnIceGatheringChange];
        if (callback == LUA_REFNIL) {
            break;
        }

        lua_State *co = lua_newthread(main_);
        lua_rawgeti(co, LUA_REGISTRYINDEX, callback);    
        lua_pushinteger(co, static_cast<int32_t>(new_state));
        int32_t ret = lua_resume(co, main_, 1);
        lua_pop(main_, 1);

        if (ret != LUA_OK && ret != LUA_YIELD) {
            logger_.Error("on_local_ice_gathering_changed failed.\n");
            int32_t args_len = lua_gettop(co);
            if (args_len > 0 && lua_type(co, -1) == LUA_TSTRING) {
                logger_.Error("on_local_ice_gathering_changed error %s.\n", lua_tostring(co, -1));
            }
        }
    } while (0);
}

void LuaRtcPeerConnection::on_local_ice_connection_changed(webrtc::PeerConnectionInterface::IceConnectionState new_state) {
    do {
        if (closed_) {
            break;
        }

        if (main_ == NULL) {
            break;
        }

        int32_t callback = lua_callbacks_[eLuaWebrtcCallbackType_OnIceConnectionChange];
        if (callback == LUA_REFNIL) {
            break;
        }

        lua_State *co = lua_newthread(main_);
        lua_rawgeti(co, LUA_REGISTRYINDEX, callback);
        lua_pushinteger(co, static_cast<int32_t>(new_state));
        int32_t ret = lua_resume(co, main_, 1);
        lua_pop(main_, 1);

        if (ret != LUA_OK && ret != LUA_YIELD) {
            logger_.Error("on_local_ice_connection_changed failed.\n");
            int32_t args_len = lua_gettop(co);
            if (args_len > 0 && lua_type(co, -1) == LUA_TSTRING) {
                logger_.Error("on_local_ice_connection_changed error %s.\n", lua_tostring(co, -1));
            }
        }
    } while (0);
}

void LuaRtcPeerConnection::on_media_ready() {
    do {
        if (closed_) {
            break;
        }

        if (main_ == NULL) {
            break;
        }

        int32_t callback = lua_callbacks_[eLuaWebrtcCallbackType_OnMediaReady];
        if (callback == LUA_REFNIL) {
            break;
        }

        lua_State *co = lua_newthread(main_);
        lua_rawgeti(co, LUA_REGISTRYINDEX, callback);
        int32_t ret = lua_resume(co, main_, 0);
        lua_pop(main_, 1);

        if (ret != LUA_OK && ret != LUA_YIELD) {
            logger_.Error("on_media_ready failed.\n");
            int32_t args_len = lua_gettop(co);
            if (args_len > 0 && lua_type(co, -1) == LUA_TSTRING) {
                logger_.Error("on_media_ready error %s.\n", lua_tostring(co, -1));
            }
        }
    } while (0);
}

} // namespace bee
