#ifndef __LUA_WEBRTC_PEER_CONNECTION_H__
#define __LUA_WEBRTC_PEER_CONNECTION_H__

#include "bee/base/bee_define.h"
#include "utility/common.h"
#include "log/logger.h"
#include "webrtc/api/peerconnectioninterface.h"
#include "webrtc/media/engine/webrtcvideocapturer.h"

#ifdef IOS
#include "webrtc/rtc_base/bind.h"
#endif

extern "C" {
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
}

namespace bee {

typedef enum LuaWebrtcCallbackType {
    eLuaWebrtcCallbackType_OnIceCandidate = 0,
    eLuaWebrtcCallbackType_OnIceGatheringChange,
    eLuaWebrtcCallbackType_OnIceConnectionChange,
    eLuaWebrtcCallbackType_OnMediaReady,
    eLuaWebrtcCallbackType_OnVideoFrame,
    eLuaWebrtcCallbackType_OnSdp,
    eLuaWebrtcCallbackType_Count
}LuaWebrtcCallbackType;

///////////////////////////////////Constants///////////////////////////////////////
const char kCandidateSdpMidName[] = "sdpMid";
const char kCandidateSdpMlineIndexName[] = "sdpMLineIndex";
const char kCandidateSdpName[] = "candidate";
const char kSessionDescriptionTypeName[] = "type";
const char kSessionDescriptionSdpName[] = "sdp";
const char kTransactionName[] = "transaction";
const char kTransactionRspDataName[] = "data";
const char kJanusName[] = "janus";
const char kSessionIdName[] = "session_id";
const char kIdName[] = "id";
const char kPrivateIdName[] = "private_id";
const char kDataName[] = "data";
const char kErrorName[] = "error";
const char kErrorCodeName[] = "code";
const char kErrorReasonName[] = "reason";
const char kHintName[] = "hint";
const char kSenderName[] = "sender";
const char kPluginName[] = "plugin";
const char kPluginDataName[] = "plugindata";
const char kResultName[] = "result";
const char kJsepName[] = "jsep";
const char kTypeName[] = "type";
const char kSdpName[] = "sdp";
const char kVideoRoomName[] = "videoroom";
const char kPublishersName[] = "publishers";
const char kDisplayName[] = "display";
const char kAudioName[] = "audio";
const char kVideoName[] = "video";
const char kAudioLabel[] = "audio_label";
const char kVideoLabel[] = "video_label";
const char kStreamLabel[] = "stream_label";

typedef enum PartyRole {
    ePartyRole_Offer = 0,
    ePartyRole_Answer
}PartyRole;

const int32_t kBeeDefaultVideoWidth = 640;
const int32_t kBeeDefaultVideoHeight = 480;
const int32_t kBeeDefaultVideoFps = 30;
const int32_t kBeeDefaultCapturerIndex = 0;
const bool kBeeDefaultIsScreencast = false;
const bool kBeeDefaultNoAudioProcessing = false;
const bool kBeeDefaultEnableLevelControl = true;

//////////////////////////////////DummySetSessionDescriptionObserver////////////////////////////////////////
class DummySetSessionDescriptionObserver : public webrtc::SetSessionDescriptionObserver {
public:
    static DummySetSessionDescriptionObserver* Create() {
        return new rtc::RefCountedObject<DummySetSessionDescriptionObserver>();
    }
    virtual void OnSuccess() {
        Logger::IF("webrtc", "%s\n", __FUNCTION__);
    }
    virtual void OnFailure(const std::string& error) {
        Logger::IF("webrtc", "%s\n", __FUNCTION__);
    }

protected:
    DummySetSessionDescriptionObserver() {}
    ~DummySetSessionDescriptionObserver() {}
};

////////////////////////////////////LuaRtcPeerConnection//////////////////////////////////////
class LuaWebrtcService;
class LuaWebrtcVideoSink;
class AudioSource;
class VideoSource;
class VideoRenderer;
class LuaRtcPeerConnection
    : public webrtc::PeerConnectionObserver,
      public webrtc::CreateSessionDescriptionObserver,
      public std::enable_shared_from_this<LuaRtcPeerConnection> {
public:
    typedef std::shared_ptr<LuaRtcPeerConnection> Ptr;
    typedef rtc::scoped_refptr<webrtc::PeerConnectionInterface> PeerConnectionPtr;
    LuaRtcPeerConnection(IOSPtr ios, std::shared_ptr<LuaWebrtcService> webrtc_service, lua_State *main, lua_State *co);
    ~LuaRtcPeerConnection();

public:
    BeeErrorCode create_webrtc_peer_connection(const std::string &stun_uri);
    BeeErrorCode create_offer(AudioSource *audio_source, VideoSource *video_source, int32_t callback);
    BeeErrorCode create_answer(const std::string &jsep, int32_t callback);
    BeeErrorCode set_remote_desc(const std::string &jsep);
    BeeErrorCode close();
    BeeErrorCode start_video_render(VideoRenderer *video_renderer);
    BeeErrorCode stop_video_render();
    BeeErrorCode get_stats(webrtc::StatsObserver *observer);
    BeeErrorCode set_video_source(bool internal, int32_t width, int32_t height, int32_t fps, int32_t capturer_index, bool is_screencast);
	BeeErrorCode set_audio_source(bool no_audio_processing, bool enable_level_control);
    void set_lua_callback(LuaWebrtcCallbackType type, int32_t callback);
	int32_t get_audio_input_level();
	int32_t get_audio_output_level();

protected:
    //PeerConnectionObserver implementation.
    virtual void OnSignalingChange(webrtc::PeerConnectionInterface::SignalingState new_state) override;
    virtual void OnAddStream(rtc::scoped_refptr<webrtc::MediaStreamInterface> stream) override;
    virtual void OnRemoveStream(rtc::scoped_refptr<webrtc::MediaStreamInterface> stream) override;
    virtual void OnDataChannel(rtc::scoped_refptr<webrtc::DataChannelInterface> channel) override;
    virtual void OnRenegotiationNeeded() override;
    virtual void OnIceConnectionChange(webrtc::PeerConnectionInterface::IceConnectionState new_state) override;
    virtual void OnIceGatheringChange(webrtc::PeerConnectionInterface::IceGatheringState new_state) override;
    virtual void OnIceCandidate(const webrtc::IceCandidateInterface* candidate) override;
    virtual void OnIceConnectionReceivingChange(bool receiving) override;

    //CreateSessionDescriptionObserver implementation.
    virtual void OnSuccess(webrtc::SessionDescriptionInterface* desc) override;
    virtual void OnFailure(const std::string& error) override;

    //RefCountInterface, ignore webrtc refcount.
#if defined(ANDROID) || defined(LINUX)
    virtual int AddRef() const override { return 1; }
    virtual int Release() const override { return 1; }
#else
    virtual void AddRef() const override {}
    virtual rtc::RefCountReleaseStatus Release() const override { return rtc::RefCountReleaseStatus::kOtherRefsRemained; }
#endif

protected:
    BeeErrorCode inner_create_offer(AudioSource *audio_source, VideoSource *video_source);
    BeeErrorCode inner_create_answer(const std::string &jsep);
    BeeErrorCode inner_set_remote_desc(const std::string &jsep);
    BeeErrorCode inner_close();
    BeeErrorCode inner_get_stats(webrtc::StatsObserver *observer);
	void inner_set_video_source(bool internal, int32_t width, int32_t height, int32_t fps, int32_t capturer_index, bool is_screencast);
	void inner_set_audio_source(bool no_audio_processing, bool enable_level_control);
	int32_t inner_get_audio_input_level();
	int32_t inner_get_audio_output_level();
    void on_get_sdp(const std::string &jsep);
    void on_local_ice_candidate(const std::string &candidate);
    void on_local_ice_gathering_changed(webrtc::PeerConnectionInterface::IceGatheringState new_state);
    void on_local_ice_connection_changed(webrtc::PeerConnectionInterface::IceConnectionState new_state);
    void on_media_ready();

protected:
    IOSPtr ios_;
    std::shared_ptr<LuaWebrtcService> webrtc_service_;
    lua_State *main_ = NULL;
    lua_State *co_ = NULL;
    PeerConnectionPtr peer_connection_;
    int32_t lua_callbacks_[eLuaWebrtcCallbackType_Count];
    PartyRole role_ = ePartyRole_Offer;
    std::shared_ptr<LuaWebrtcVideoSink> video_receiver_;
    bool closed_ = false;
    Logger logger_;
#ifdef WIN32
    bool use_external_video_source_ = false;
#else
    bool use_external_video_source_ = true;
#endif
    int32_t video_source_width_ = kBeeDefaultVideoWidth;
    int32_t video_source_height_ = kBeeDefaultVideoHeight;
    int32_t video_source_fps_ = kBeeDefaultVideoFps;
	int32_t video_capturer_index_ = kBeeDefaultCapturerIndex;
    bool video_is_screencast_ = kBeeDefaultIsScreencast;
	bool no_audio_processing_ = kBeeDefaultNoAudioProcessing;
	bool enable_level_control_ = kBeeDefaultEnableLevelControl;
};

} // namespace bee 

#endif
