#include "platform/ios/bee_video_source_oc_adapter.h"
#include "platform/ios/screenvideotracksource.h"
#include "internal/video_source_internal.h"
#include "webrtc/api/videosourceproxy.h"
#include "lua/module/lua_webrtc_service.h"

namespace bee {

BeeVideoSourceOcAdapter::BeeVideoSourceOcAdapter(bee_int32_t width, bee_int32_t height, bee_int32_t fps)
    : bee::VideoSource(width, height, fps),
      is_screencast_(false) {
    
}

BeeVideoSourceOcAdapter::~BeeVideoSourceOcAdapter() {
    
}
    
BeeErrorCode BeeVideoSourceOcAdapter::open(bool is_screencast) {
    is_screencast_ = is_screencast;
    return open();
}

BeeErrorCode BeeVideoSourceOcAdapter::open() {
    BeeErrorCode ret = kBeeErrorCode_Success;
    do {
        bee::LuaWebrtcService::FactoryPtr peerconnection_factory = bee::LuaWebrtcService::peer_connection_factory();
        if (peerconnection_factory == NULL) {
            ret = kBeeErrorCode_Null_Pointer;
            break;
        }
        
        rtc::Thread *signalingThread = bee::LuaWebrtcService::signaling_thread();
        rtc::Thread *workerThread = bee::LuaWebrtcService::worker_thread();
        if (signalingThread == NULL || workerThread == NULL) {
            ret = kBeeErrorCode_Service_Not_Started;
            break;
        }
        
        rtc::scoped_refptr<webrtc::ObjcVideoTrackSource> objc_video_track_source;
        if (is_screencast_) {
            objc_video_track_source = new rtc::RefCountedObject<bee::ScreenVideoTrackSource>();
        } else {
            objc_video_track_source = new rtc::RefCountedObject<webrtc::ObjcVideoTrackSource>();
        }
        
        video_source_internal_->rtc_video_track_source_ = webrtc::VideoTrackSourceProxy::Create(signalingThread, workerThread, objc_video_track_source);
    } while (false);
    return ret;
}

} // namespace bee
