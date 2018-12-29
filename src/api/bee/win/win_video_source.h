#ifndef __WIN_VIDEO_SOURCE_H__
#define __WIN_VIDEO_SOURCE_H__

#include "bee/base/bee_define.h"
#include "bee/media/video_source.h"
#include <memory>

namespace bee {

//////////////////////////////////WinVideoSource////////////////////////////////////////
class WinVideoSource : public VideoSource {
public:
    WinVideoSource(
        bee_int32_t width,
        bee_int32_t height,
        bee_int32_t fps,
        bool screencast);
    virtual ~WinVideoSource();

public:
    virtual BeeErrorCode open();

    virtual void on_frame(const void* data,
        bee_int32_t data_len,
        bee_int32_t width,
        bee_int32_t height,
        bee_int32_t rotation,
        bee_int64_t timestamp_ns);
};

} // namespace bee

#endif // #ifndef __WIN_VIDEO_SOURCE_CAM_H__
