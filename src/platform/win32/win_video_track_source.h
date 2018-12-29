#ifndef __WIN_VIDEO_TRACK_SOURCE_H__
#define __WIN_VIDEO_TRACK_SOURCE_H__

#include "bee/base/bee_define.h"
#include "webrtc/rtc_base/asyncinvoker.h"
#include "webrtc/rtc_base/checks.h"
#include "webrtc/rtc_base/thread_checker.h"
#include "webrtc/rtc_base/timestampaligner.h"
#include "webrtc/common_video/include/i420_buffer_pool.h"
#include "webrtc/common_video/libyuv/include/webrtc_libyuv.h"
#include "webrtc/media/base/adaptedvideotracksource.h"
#include "webrtc/api/videosourceproxy.h"

namespace bee {

class WinVideoTrackSource : public rtc::AdaptedVideoTrackSource {
 public:
     WinVideoTrackSource(rtc::Thread* signaling_thread, bool is_screencast = true);
     ~WinVideoTrackSource();

public:
     bool is_screencast() const override { return is_screencast_; }

    // Indicates that the encoder should denoise video before encoding it.
    // If it is not set, the default configuration is used which is different
    // depending on video codec.
    rtc::Optional<bool> needs_denoising() const override {
        return rtc::Optional<bool>(false);
    }

    void SetState(SourceState state);

    SourceState state() const override { return state_; }

    bool remote() const override { return false; }

    void OnByteBufferFrameCaptured(
        const void* frame_data,
        int length,
        int width,
        int height,
        int rotation,
        int64_t timestamp_ns);

    void OnOutputFormatRequest(int width, int height, int fps);

    static rtc::scoped_refptr<webrtc::VideoTrackSourceInterface> create(
        rtc::Thread *signaling_thread,
        rtc::Thread *worker_thread,
        int32_t width,
        int32_t height,
        int32_t fps);

 private:
    rtc::Thread* signaling_thread_;
    rtc::AsyncInvoker invoker_;
    rtc::ThreadChecker camera_thread_checker_;
    SourceState state_;
    rtc::VideoBroadcaster broadcaster_;
    rtc::TimestampAligner timestamp_aligner_;
    webrtc::NV12ToI420Scaler nv12toi420_scaler_;
    webrtc::I420BufferPool buffer_pool_;
    const bool is_screencast_;
};

}  // namespace bee

#endif  // __WIN_VIDEO_TRACK_SOURCE_H__
