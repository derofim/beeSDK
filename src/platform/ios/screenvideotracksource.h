#ifndef __SCREEN_VIDEO_TRACK_SOURCE_H__
#define __SCREEN_VIDEO_TRACK_SOURCE_H__

#include "platform/ios/objcvideotracksource.h"

namespace bee {

class ScreenVideoTrackSource : public webrtc::ObjcVideoTrackSource {
public:
    ScreenVideoTrackSource() {}
    bool is_screencast() const override { return true; }
};

}  // namespace bee

#endif  // __SCREEN_VIDEO_TRACK_SOURCE_H__
