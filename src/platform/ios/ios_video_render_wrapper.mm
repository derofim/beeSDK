#ifdef IOS
#import <Foundation/Foundation.h>
#import "platform/ios/ios_video_render_wrapper.h"
#import "platform/ios/RTCI420Buffer+Private.h"
#import "platform/ios/RTCVideoFrame+Private.h"
#import "WebRTC/RTCVideoRenderer.h"
#import "WebRTC/RTCVideoFrame.h"
#import "WebRTC/RTCVideoFrameBuffer.h"

namespace bee {

IOSVideoRendererWrapper::IOSVideoRendererWrapper(void *ios_video_renderer)
    : ios_video_renderer_(ios_video_renderer),
      logger_("IOSVideoRender"){
    
}

IOSVideoRendererWrapper::~IOSVideoRendererWrapper() {
    
}

void IOSVideoRendererWrapper::OnFrame(const webrtc::VideoFrame &nativeVideoFrame) {
    if (ios_video_renderer_ != NULL) {
        id<RTCVideoRenderer> videoRenderer = (__bridge id<RTCVideoRenderer>)ios_video_renderer_;
        
        RTCVideoFrame* videoFrame = [[RTCVideoFrame alloc] initWithNativeVideoFrame:nativeVideoFrame];
        
        CGSize current_size = (videoFrame.rotation % 180 == 0)
        ? CGSizeMake(videoFrame.width, videoFrame.height)
        : CGSizeMake(videoFrame.height, videoFrame.width);
        
        CGSize size_ = CGSizeMake(width_, height_);
        
        if (!CGSizeEqualToSize(size_, current_size)) {
            width_ = current_size.width;
            height_ = current_size.height;
            size_ = CGSizeMake(width_, height_);
            [videoRenderer setSize:size_];
        }
        
        //logger_.Debug("Render one frame\n");
        [videoRenderer renderFrame:videoFrame];
    }
}

} // namespace bee
#endif /* IOS */
