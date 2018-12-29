#ifndef __BEE_VIDEO_RENDERER_JNI_ADAPTER_H__
#define __BEE_VIDEO_RENDERER_JNI_ADAPTER_H__

#include <log/logger.h>
#include "bee/media/video_renderer.h"

namespace bee {

class BeeVideoRendererJniAdapter : VideoRenderer {
public:
    BeeVideoRendererJniAdapter();
    virtual ~BeeVideoRendererJniAdapter();
    
public:
    BeeErrorCode open(long nativeRtcVideoRenderer);
    virtual void on_frame(const VideoFrame &frame);

private:
    Logger logger_;
};

} // namespace bee

#endif // #ifndef __BEE_VIDEO_RENDERER_JNI_ADAPTER_H__
