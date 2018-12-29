#ifndef __WIN_ADAPTER_H__
#define __WIN_ADAPTER_H__

#include "bee/base/bee_define.h"
#include "webrtc/api/peerconnectioninterface.h"
#include "webrtc/api/videosourceproxy.h"

namespace bee {
    
class WinAdapter {
public:
    static rtc::scoped_refptr<webrtc::VideoTrackSourceInterface> create_video_source(
        bee_handle handle,
        rtc::Thread *signaling_thread, 
        rtc::Thread *worker_thread,
        int32_t width, 
        int32_t height, 
        int32_t fps);
};
    
} // namespace bee

#endif /* __WIN_ADAPTER_H__ */
