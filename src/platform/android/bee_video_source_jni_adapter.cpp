#ifdef ANDROID
#include "platform/android/bee_video_source_jni_adapter.h"
#include "internal/video_source_internal.h"
#include "lua/module/lua_webrtc_service.h"
#include "webrtc/api/videosourceproxy.h"
#include "platform/android/androidvideotracksource.h"

namespace bee {

BeeVideoSourceJniAdapter::BeeVideoSourceJniAdapter(bee_int32_t width, bee_int32_t height, bee_int32_t fps, bool is_screencast)
    : bee::VideoSource(width, height, fps, is_screencast), logger_("BeeVideoSourceJniAdapter") {
    
}

BeeVideoSourceJniAdapter::~BeeVideoSourceJniAdapter() {
    logger_.Info("BeeVideoSourceJniAdapter::~BeeVideoSourceJniAdapter.");
}

BeeErrorCode BeeVideoSourceJniAdapter::open(JNIEnv *jni, jobject j_egl_context) {
    jni_ = jni;
    j_egl_context_ = j_egl_context;
    return open();
}

long BeeVideoSourceJniAdapter::getRtcVideoSource() {
    return (long)video_source_internal_->rtc_video_track_source_.get();
}

BeeErrorCode BeeVideoSourceJniAdapter::open() {
    BeeErrorCode ret = kBeeErrorCode_Success;
    do {
        if (jni_ == NULL || j_egl_context_ == NULL) {
            ret = kBeeErrorCode_Invalid_Param;
            break;
        }

        bee::LuaWebrtcService::FactoryPtr peerconnection_factory = bee::LuaWebrtcService::peer_connection_factory();
        if (peerconnection_factory == NULL) {
            ret = kBeeErrorCode_Null_Pointer;
            break;
        }
        
        rtc::Thread *signaling_thread = bee::LuaWebrtcService::signaling_thread();
        rtc::Thread *worker_thread = bee::LuaWebrtcService::worker_thread();
        if (signaling_thread == NULL || worker_thread == NULL) {
            ret = kBeeErrorCode_Service_Not_Started;
            break;
        }

        rtc::scoped_refptr<bee::AndroidVideoTrackSource> source(new rtc::RefCountedObject<bee::AndroidVideoTrackSource>(
                signaling_thread, jni_, j_egl_context_, is_screencast_));
        video_source_internal_->rtc_video_track_source_ =
                webrtc::VideoTrackSourceProxy::Create(signaling_thread, worker_thread, source);
    } while (false);
    return ret;
}

} // namespace bee
#endif // #ifdef ANDROID
