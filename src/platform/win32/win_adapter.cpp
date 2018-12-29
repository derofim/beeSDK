#include "win_adapter.h"
#include "win_video_track_source.h"

namespace bee {

rtc::scoped_refptr<webrtc::VideoTrackSourceInterface> WinAdapter::create_video_source(
    bee_handle handle,
    rtc::Thread *signaling_thread, 
    rtc::Thread *worker_thread,
    int32_t width,
    int32_t height,
    int32_t fps) {
    return WinVideoTrackSource::create(handle, signaling_thread, worker_thread, width, height, fps);
}

} // namespace bee
