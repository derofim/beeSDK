#include "platform/android/bee_video_renderer_jni_adapter.h"
#include "internal/video_renderer_internal.h"
#include "internal/video_frame_internal.h"
#include "webrtc/sdk/android/src/jni/javavideorendererwrapper.h"

namespace bee {

BeeVideoRendererJniAdapter::BeeVideoRendererJniAdapter():
        logger_("BeeVideoRendererJniAdapter"){
    
}

BeeVideoRendererJniAdapter::~BeeVideoRendererJniAdapter() {
    logger_.Info("BeeVideoRendererJniAdapter::~BeeVideoRendererJniAdapter");
}

BeeErrorCode BeeVideoRendererJniAdapter::open(long nativeRtcVideoRenderer) {
    BeeErrorCode ret = kBeeErrorCode_Success;
    do {
        if (nativeRtcVideoRenderer == 0) {
            ret = kBeeErrorCode_Invalid_Param;
            break;
        }

        //Make javaVideoRenderer owned by BeeVideoRendererJniAdapter, so not to dispose Jave VideoRenderer to delete more than once.
        webrtc_jni::JavaVideoRendererWrapper *javaVideoRenderer = reinterpret_cast<webrtc_jni::JavaVideoRendererWrapper*>(nativeRtcVideoRenderer);
        video_renderer_internal_->rtc_video_renderer_ = std::shared_ptr<webrtc_jni::JavaVideoRendererWrapper>(javaVideoRenderer);
    } while (0);
    return ret;
}

void BeeVideoRendererJniAdapter::on_frame(const VideoFrame &frame) {
    if (video_renderer_internal_->rtc_video_renderer_ != NULL) {
        video_renderer_internal_->rtc_video_renderer_->OnFrame(frame.video_frame_internal_->rtc_video_frame_);
    }
}

} // namespace bee
