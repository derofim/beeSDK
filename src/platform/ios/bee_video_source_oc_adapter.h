#ifndef __BEE_VIDEO_SOURCE_OC_ADAPTER_H__
#define __BEE_VIDEO_SOURCE_OC_ADAPTER_H__

#include "bee/media/video_source.h"

namespace bee {

class BeeVideoSourceOcAdapter : public VideoSource {
public:
    BeeVideoSourceOcAdapter(bee_int32_t width, bee_int32_t height, bee_int32_t fps);
    virtual ~BeeVideoSourceOcAdapter();
    
public:
    BeeErrorCode open(bool is_screencast);
    
protected:
    virtual BeeErrorCode open();
    
private:
    bool is_screencast_;
};

} // namespace bee

#endif // #ifndef __BEE_VIDEO_SOURCE_OC_ADAPTER_H__
