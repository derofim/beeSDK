#ifndef __IOS_ADAPTER_H__
#define __IOS_ADAPTER_H__

#include "webrtc/api/peerconnectioninterface.h"
#include "webrtc/api/videosourceproxy.h"

namespace bee {
    
class IOSAdapter {
public:
    static cricket::WebRtcVideoEncoderFactory *create_video_encoder_facotory();
    static cricket::WebRtcVideoDecoderFactory *create_video_decoder_facotory();
    static std::string get_defualt_lua_file_path();
};

} // namespace bee

#endif /* __IOS_ADAPTER_H__ */
