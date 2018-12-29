#import "bee/ios/bee_video_source.h"
#import "WebRTC/RTCVideoSource.h"
#import "WebRTC/RTCVideoFrame.h"
#import "WebRTC/RTCVideoFrameBuffer.h"
#import "platform/ios/RTCVideoSource+Private.h"

#include "platform/ios/bee_video_source_oc_adapter.h"
#include "internal/video_source_internal.h"

@implementation BeeVideoSource {
    RTCVideoSource *_rtcVideoSource;    
    RTCVideoCapturer *_dummyCapturer;
    BOOL _opened;
}

@synthesize width = _width;
@synthesize height = _height;
@synthesize fps = _fps;
@synthesize isScreencast = _isScreencast;
@synthesize internalVideoSource = _internalVideoSource;

- (instancetype)initWithParam:(BOOL)isScreencast
                        width:(bee_int32_t)width
                       height:(bee_int32_t)height
                          fps:(bee_int32_t)fps {
    if (self = [super init]) {
        _rtcVideoSource = nil;
        _dummyCapturer = [[RTCVideoCapturer alloc] init];
        _opened = NO;
        _isScreencast = isScreencast;
        _width = width;
        _height = height;
        _fps = fps;
    }
    return self;
}

- (BeeErrorCode)open {
    BeeErrorCode ret = kBeeErrorCode_Success;
    do {
        if (_opened) {
            break;
        }
        
        _internalVideoSource.reset(new bee::BeeVideoSourceOcAdapter(_width, _height, _fps));
        ret = _internalVideoSource->open(_isScreencast ? true : false);
        if (ret != kBeeErrorCode_Success) {
            break;
        }
        
        auto videoSource = _internalVideoSource->video_source_internal_->rtc_video_track_source_;
        if (videoSource == NULL) {
            ret = kBeeErrorCode_Null_Pointer;
            break;
        }
        
        _rtcVideoSource = [[RTCVideoSource alloc] initWithNativeVideoSource:videoSource];
        _opened = YES;
    } while (false);
    return ret;
}

- (void)onFrame:(CVPixelBufferRef)pixelBuffer rotation:(UIDeviceOrientation)rotation timestamp:(int64_t)timestamp {
    if (_rtcVideoSource == nil) {
        return;
    }
    
    RTCVideoRotation rtcRotaion = RTCVideoRotation_90;
    switch (rotation) {
        case UIDeviceOrientationPortrait:
            rtcRotaion = RTCVideoRotation_90;
            break;
        case UIDeviceOrientationPortraitUpsideDown:
            rtcRotaion = RTCVideoRotation_270;
            break;
        case UIDeviceOrientationLandscapeLeft:
            rtcRotaion = RTCVideoRotation_0;
            break;
        case UIDeviceOrientationLandscapeRight:
            rtcRotaion = RTCVideoRotation_180;
            break;
        default:
            break;
    }
    
    RTCCVPixelBuffer *rtcPixelBuffer = [[RTCCVPixelBuffer alloc] initWithPixelBuffer:pixelBuffer];
    RTCVideoFrame *videoFrame = [[RTCVideoFrame alloc] initWithBuffer:rtcPixelBuffer rotation:rtcRotaion timeStampNs:timestamp];
    [_rtcVideoSource capturer:_dummyCapturer didCaptureVideoFrame:videoFrame];
}

@end
