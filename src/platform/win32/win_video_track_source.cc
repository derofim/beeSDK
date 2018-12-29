#include "win_video_track_source.h"
#include "service/bee_entrance.h"
#include <utility>

namespace {
// MediaCodec wants resolution to be divisible by 2.
const int kRequiredResolutionAlignment = 2;
}

namespace bee {

WinVideoTrackSource::WinVideoTrackSource(rtc::Thread* signaling_thread, bool is_screencast)
    : AdaptedVideoTrackSource(kRequiredResolutionAlignment),
      signaling_thread_(signaling_thread),
      is_screencast_(is_screencast) {
    camera_thread_checker_.DetachFromThread();
}

WinVideoTrackSource::~WinVideoTrackSource() {

}

void WinVideoTrackSource::SetState(SourceState state) {
    if (signaling_thread_ == NULL) {
        return;
    }

    if (rtc::Thread::Current() != signaling_thread_) {
        invoker_.AsyncInvoke<void>(
            RTC_FROM_HERE, signaling_thread_,
            rtc::Bind(&WinVideoTrackSource::SetState, this, state));
        return;
    }

    if (state_ != state) {
        state_ = state;
        FireOnChanged();
    }
}

void WinVideoTrackSource::OnByteBufferFrameCaptured(
    const void* frame_data,
    int length,
    int width,
    int height,
    int rotation,
    int64_t timestamp_ns) {
    RTC_DCHECK(camera_thread_checker_.CalledOnValidThread());
    RTC_DCHECK(rotation == 0 || rotation == 90 || rotation == 180 || rotation == 270);

    int64_t camera_time_us = timestamp_ns / rtc::kNumNanosecsPerMicrosec;
    int64_t translated_camera_time_us = timestamp_aligner_.TranslateTimestamp(camera_time_us, rtc::TimeMicros());

    int adapted_width;
    int adapted_height;
    int crop_width;
    int crop_height;
    int crop_x;
    int crop_y;

    if (!AdaptFrame(
        width, 
        height, 
        camera_time_us, 
        &adapted_width,
        &adapted_height, 
        &crop_width, 
        &crop_height, 
        &crop_x,
        &crop_y)) {
        return;
    }

    const uint8_t* y_plane = static_cast<const uint8_t*>(frame_data);
    const uint8_t* uv_plane = y_plane + width * height;
    const int uv_width = (width + 1) / 2;

    RTC_CHECK_GE(length, width * height + 2 * uv_width * ((height + 1) / 2));

    // Can only crop at even pixels.
    crop_x &= ~1;
    crop_y &= ~1;
    // Crop just by modifying pointers.
    y_plane += width * crop_y + crop_x;
    uv_plane += uv_width * crop_y + crop_x;

    rtc::scoped_refptr<webrtc::I420Buffer> buffer = buffer_pool_.CreateBuffer(adapted_width, adapted_height);

    nv12toi420_scaler_.NV12ToI420Scale(
        y_plane, 
        width, 
        uv_plane, 
        uv_width * 2, 
        crop_width, 
        crop_height,
        buffer->MutableDataY(), 
        buffer->StrideY(),
        buffer->MutableDataU(), //OBS uses NV12, not NV21.
        buffer->StrideU(),
        buffer->MutableDataV(), 
        buffer->StrideV(), 
        buffer->width(), 
        buffer->height());

    OnFrame(webrtc::VideoFrame(buffer, static_cast<webrtc::VideoRotation>(rotation), translated_camera_time_us));
}

void WinVideoTrackSource::OnOutputFormatRequest(int width, int height, int fps) {
    cricket::VideoFormat format(width, height, cricket::VideoFormat::FpsToInterval(fps), cricket::FOURCC_NV12);
    video_adapter()->OnOutputFormatRequest(format);
}

rtc::scoped_refptr<webrtc::VideoTrackSourceInterface> WinVideoTrackSource::create(
    rtc::Thread *signaling_thread, 
    rtc::Thread *worker_thread,
    int32_t width,
    int32_t height,
    int32_t fps) {
    rtc::scoped_refptr<webrtc::VideoTrackSourceInterface> proxy_source;
    do {
        if (signaling_thread == NULL || worker_thread == NULL) {
            break;
        }

        auto source(new rtc::RefCountedObject<WinVideoTrackSource>(signaling_thread));
        source->OnOutputFormatRequest(width, height, fps);
        source->SetState(webrtc::MediaSourceInterface::kLive);
        proxy_source = webrtc::VideoTrackSourceProxy::Create(signaling_thread, worker_thread, source);       
    } while (0);
    return proxy_source;
}

}  // namespace bee
