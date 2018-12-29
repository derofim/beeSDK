#ifndef __BEE_VIDEO_RENDERER_OC_ADAPTER_H__
#define __BEE_VIDEO_RENDERER_OC_ADAPTER_H__

#include "bee/media/video_renderer.h"

@class BeeVideoRenderer;

namespace bee {

class BeeVideoRendererOcAdapter : VideoRenderer {
public:
    BeeVideoRendererOcAdapter(BeeVideoRenderer *oc_video_renderer);
    virtual ~BeeVideoRendererOcAdapter();
    
public:
    virtual void on_frame(const VideoFrame &frame);

private:
    __weak BeeVideoRenderer *oc_video_renderer_;
};

} // namespace bee

#endif // #ifndef __BEE_VIDEO_RENDERER_OC_ADAPTER_H__
