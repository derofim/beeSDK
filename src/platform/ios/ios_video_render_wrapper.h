#ifndef ios_video_renderer_wrapper_h
#define ios_video_renderer_wrapper_h

#include "webrtc/api/video/video_frame.h"
#include "log/logger.h"

namespace bee {

class IOSVideoRendererWrapper {
public:
    IOSVideoRendererWrapper(void *ios_video_renderer);
    ~IOSVideoRendererWrapper();
    
public:
    void OnFrame(const webrtc::VideoFrame &nativeVideoFrame);
    
private:
    void *ios_video_renderer_ = NULL;
    int height_ = 0;
    int width_ = 0;
    Logger logger_;
};

} // namespace bee
#endif /* ios_video_renderer_wrapper_h */
