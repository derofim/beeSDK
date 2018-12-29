#include "bee/win/win_video_source.h"
#include "internal/video_source_internal.h"
#include "platform/win32/win_video_track_source.h"
#include "lua/module/lua_webrtc_service.h"

namespace bee {

WinVideoSource::WinVideoSource(
    bee_int32_t width,
    bee_int32_t height,
    bee_int32_t fps,
    bool screencast)
    : VideoSource(width, height, fps, screencast) {

}

WinVideoSource::~WinVideoSource() {

}

BeeErrorCode WinVideoSource::open() {
    if (video_source_internal_->rtc_video_track_source_ == NULL) {
        rtc::scoped_refptr<WinVideoTrackSource> source(new rtc::RefCountedObject<WinVideoTrackSource>(
            LuaWebrtcService::signaling_thread(), 
            is_screencast_));

        video_source_internal_->rtc_video_track_source_ =
            webrtc::VideoTrackSourceProxy::Create(
                LuaWebrtcService::signaling_thread(), 
                LuaWebrtcService::worker_thread(), 
                source);
    }
    return kBeeErrorCode_Success;
}

void WinVideoSource::on_frame(
    const void* data,
    bee_int32_t data_len,
    bee_int32_t width,
    bee_int32_t height,
    bee_int32_t rotation,
    bee_int64_t timestamp_ns) {
    auto video_source = video_source_internal_->rtc_video_track_source_;
    auto proxy_source = reinterpret_cast<webrtc::VideoTrackSourceProxy*>(video_source.get());
    WinVideoTrackSource *win_video_source = reinterpret_cast<WinVideoTrackSource*>(proxy_source->internal());

    if (win_video_source != NULL) {
        win_video_source->OnByteBufferFrameCaptured(
            data,
            data_len,
            width,
            height,
            rotation,
            timestamp_ns);
    }
}

} // namespace bee
