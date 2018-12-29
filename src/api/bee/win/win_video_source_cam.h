#ifndef __WIN_VIDEO_SOURCE_CAM_H__
#define __WIN_VIDEO_SOURCE_CAM_H__

#include "bee/base/bee_define.h"
#include "bee/media/video_source.h"
#include <memory>

namespace bee {

/////////////////////////////////Const/////////////////////////////////////////
const bee_int32_t kBeeDefaultVideoWidth = 640;
const bee_int32_t kBeeDefaultVideoHeight = 480;
const bee_int32_t kBeeDefaultVideoFps = 30;

//////////////////////////////////WinVideoSourceCam////////////////////////////////////////
class WinVideoSourceCam : public VideoSource {
public:
    WinVideoSourceCam(
        bee_int32_t width = kBeeDefaultVideoWidth,
        bee_int32_t height = kBeeDefaultVideoHeight,
        bee_int32_t fps = kBeeDefaultVideoFps,
        bee_int32_t camera_index = 0);
    virtual ~WinVideoSourceCam();

public:
    /**********************************************************************************
    * Function : open
    * Desc     : Open video source.    
    *
    * Return   : Bee error code.
    ***********************************************************************************/
    virtual BeeErrorCode open();

private:
    bee_uint32_t camera_index_;
    bool opened_;
};

} // namespace bee

#endif // #ifndef __WIN_VIDEO_SOURCE_CAM_H__
