#import "bee/ios/bee_video_source_camera.h"
#import "WebRTC/RTCCameraVideoCapturer.h"
#import "WebRTC/RTCVideoSource.h"
#import "platform/ios/ARDCaptureController.h"
#import "objc/Runtime.h"

@implementation BeeVideoSourceCamera {
    ARDCaptureController *_captureController;
}

- (instancetype)initWithParam:(bee_int32_t)fps
                        width:(bee_int32_t)width
                       height:(bee_int32_t)height
                       fixRes:(BOOL)fixRes {
    if (self = [super initWithParam:fixRes
                              width:width
                             height:height
                                fps:fps]) {
        _captureController = nil;
    }
    return self;
}

- (BeeErrorCode)open {
    BeeErrorCode ret = kBeeErrorCode_Success;
    do {
        ret = [super open];
        if (ret != kBeeErrorCode_Success) {
            break;
        }
        
        //Get parent private member using runtime.
        Ivar nameIvar = class_getInstanceVariable([super class], "_rtcVideoSource");
        if (nameIvar == nil) {
            ret = kBeeErrorCode_Signature_Error;
            break;
        }
        
        RTCVideoSource *rtcVideoSource = object_getIvar(self, nameIvar);
        if (rtcVideoSource == nil) {
            ret = kBeeErrorCode_Null_Pointer;
            break;
        }
        
#if !TARGET_IPHONE_SIMULATOR
        RTCCameraVideoCapturer *videoCapturer = [[RTCCameraVideoCapturer alloc] initWithDelegate:rtcVideoSource];
#endif
        if (videoCapturer == nil) {
            ret = kBeeErrorCode_Invalid_State;
            break;
        }
        
        _captureController = [[ARDCaptureController alloc] initWithCapturer:videoCapturer
                                                                        fps:self.fps
                                                                      width:self.width
                                                                     height:self.height];
        [_captureController startCapture];
    } while (false);
    return ret;
}

- (void)close {
    if (_captureController != nil) {
        [_captureController stopCapture];
        _captureController = nil;
    }
}

- (void)switchCamera {
    if ( _captureController != nil) {
        [_captureController switchCamera];
    }
}

- (BOOL)isFrontCamera {
    if (_captureController != nil) {
        return [_captureController isFrontCamera];
    } else {
        return YES;
    }
}

@end

