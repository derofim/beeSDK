#include "platform/ios/bee_video_renderer_oc_adapter.h"
#import "bee/ios/bee_video_renderer.h"

namespace bee {
    
BeeVideoRendererOcAdapter::BeeVideoRendererOcAdapter(BeeVideoRenderer *oc_video_renderer) : oc_video_renderer_(oc_video_renderer) {
    
}

BeeVideoRendererOcAdapter::~BeeVideoRendererOcAdapter() {
    
}

void BeeVideoRendererOcAdapter::on_frame(const VideoFrame &frame) {
    if (oc_video_renderer_ != nil) {
        [oc_video_renderer_ onFrame:frame];
    }
}

} // namespace bee
