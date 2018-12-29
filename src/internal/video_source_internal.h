#ifndef __VIDEO_SOURCE_INTERNAL_H__
#define __VIDEO_SOURCE_INTERNAL_H__

#include "bee/base/bee_define.h"
#include "webrtc/api/mediastreaminterface.h"

namespace bee {

class VideoSourceInternal {
public:
    VideoSourceInternal();
    virtual ~VideoSourceInternal();

public:
    void on_frame(
        const void* data,
        bee_int32_t data_len,
        bee_int32_t width,
        bee_int32_t height,
        bee_int32_t rotation,
        bee_int64_t timestamp_ns);

public:
    rtc::scoped_refptr<webrtc::VideoTrackSourceInterface> rtc_video_track_source_;
};

} // namespace bee

#endif // #ifndef __VIDEO_SOURCE_INTERNAL_H__
