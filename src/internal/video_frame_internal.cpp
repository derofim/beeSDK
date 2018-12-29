#include "internal/video_frame_internal.h"

namespace bee {

VideoFrameInternal::VideoFrameInternal(const webrtc::VideoFrame &video_frame) : rtc_video_frame_(video_frame) {

}

VideoFrameInternal::~VideoFrameInternal() {

}

} // namespace bee
