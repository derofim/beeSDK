#ifndef __VIDEO_RENDERER_INTERNAL_H__
#define __VIDEO_RENDERER_INTERNAL_H__

#include "webrtc/api/video/video_frame.h"
#ifdef ANDROID
#include "webrtc/media/base/videosinkinterface.h"
#else
#include "webrtc/api/videosinkinterface.h"
#endif

namespace bee {

class VideoRendererInternal {
public:
    VideoRendererInternal();
    virtual ~VideoRendererInternal();

public:
    std::shared_ptr<rtc::VideoSinkInterface<webrtc::VideoFrame> > rtc_video_renderer_;
};

} // namespace bee

#endif // #ifndef __VIDEO_RENDERER_INTERNAL_H__
