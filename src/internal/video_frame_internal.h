#ifndef __VIDEO_FRAME_INTERNAL_H__
#define __VIDEO_FRAME_INTERNAL_H__

#include "webrtc/api/video/video_frame.h"

namespace bee {

class VideoFrameInternal {
public:
    VideoFrameInternal(const webrtc::VideoFrame &video_frame);
    virtual ~VideoFrameInternal();

public:
    webrtc::VideoFrame rtc_video_frame_;
};

} // namespace bee

#endif // #ifndef __VIDEO_FRAME_INTERNAL_H__
