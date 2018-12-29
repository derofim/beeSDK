#import "bee/ios/bee_video_renderer.h"
#import "platform/ios/RTCVideoFrame+Private.h"
#import <WebRTC/RTCEAGLVideoView.h>
#import <WebRTC/RTCMTLVideoView.h>

#import <Metal/Metal.h>
#import <MetalKit/MetalKit.h>

#include "platform/ios/bee_video_renderer_oc_adapter.h"
#include "internal/video_frame_internal.h"

#if defined(__aarch64__)
#define BEE_SUPPORTS_METAL
#endif

@interface BeeVideoRenderer () <RTCEAGLVideoViewDelegate>
@end

@implementation BeeVideoRenderer {
    CGSize _currentSize;
}

@synthesize view = _view;
@synthesize internalVideoRenderer = _internalVideoRenderer;
@synthesize delegate = _delegate;
@synthesize mirror = _mirror;

- (instancetype)init {
    if (self = [super init]) {
        _internalVideoRenderer.reset(new bee::BeeVideoRendererOcAdapter(self));
#ifdef BEE_SUPPORTS_METAL
        //A trick to trigger RTCMTLVideoView init successly.wierd!!!
        MTKView *view = [[MTKView alloc] initWithFrame:CGRectZero];
        if (view) {
            NSLog(@"MTKView init success");
        } else {
            NSLog(@"MTKView init fail");
        }
        
        _view = [[RTCMTLVideoView alloc] initWithFrame:CGRectZero];
#else
        RTCEAGLVideoView *view = [[RTCEAGLVideoView alloc] initWithFrame:CGRectZero];
        view.delegate = self;
        _view = view;
#endif
        id<RTCVideoRenderer> renderer = (id<RTCVideoRenderer>)_view;
        [renderer renderFrame:nil];
        _currentSize = CGSizeZero;
        _mirror = NO;
    }
    return self;
}

- (void)onFrame:(const bee::VideoFrame &)frame {
    RTCVideoFrame *videoFrame = [[RTCVideoFrame alloc] initWithNativeVideoFrame:frame.video_frame_internal_->rtc_video_frame_];
    
    CGSize currentFrameSize = CGSizeZero;
    if (videoFrame.rotation % 180 == 0) {
        currentFrameSize = CGSizeMake(videoFrame.width, videoFrame.height); //0, 180, 360
    } else {
        currentFrameSize = CGSizeMake(videoFrame.height, videoFrame.width); //90, 270
    }
    
    id<RTCVideoRenderer> renderer = (id<RTCVideoRenderer>)_view;
    if (!CGSizeEqualToSize(_currentSize, currentFrameSize)) {
        _currentSize = currentFrameSize;
        [renderer setSize:_currentSize];
    }
    
    [renderer renderFrame:videoFrame];
}

- (void)setMirror:(BOOL)mirror {
    if (mirror != _mirror && _view != nil) {
        CGAffineTransform transform = _view.transform;
        transform = CGAffineTransformScale(transform, -1, 1);
        _view.transform = transform;
    }
    _mirror = mirror;
}

- (BOOL)mirror {
    return _mirror;
}

#pragma mark - RTCEAGLVideoViewDelegate

- (void)videoView:(RTCEAGLVideoView*)videoView didChangeVideoSize:(CGSize)size {
    if (_delegate != nil) {
        [_delegate didChangeVideoSize:self size:size];
    }
}

@end
