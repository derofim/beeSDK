#import <Foundation/Foundation.h>
#import "WebRTC/RTCVideoCodecH264.h"
#include "platform/ios/ios_adapter.h"
#include "platform/ios/objc_video_encoder_factory.h"
#include "platform/ios/objc_video_decoder_factory.h"
#include "platform/ios/screenvideotracksource.h"

namespace bee {

cricket::WebRtcVideoEncoderFactory *IOSAdapter::create_video_encoder_facotory() {
    return new webrtc::ObjCVideoEncoderFactory([[RTCVideoEncoderFactoryH264 alloc] init]);
}

cricket::WebRtcVideoDecoderFactory *IOSAdapter::create_video_decoder_facotory() {
    return new webrtc::ObjCVideoDecoderFactory([[RTCVideoDecoderFactoryH264 alloc] init]);
}

std::string IOSAdapter::get_defualt_lua_file_path() {
    std::string path;
    NSString *nsPath = [[NSBundle mainBundle] pathForResource:@"new_bee"
                                                       ofType:@"lua"];
    if (nsPath != NULL) {
        path = [nsPath UTF8String];
    }
    
    return path;
}

} // namespace bee
