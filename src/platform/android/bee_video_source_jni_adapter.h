#ifdef ANDROID
#ifndef __BEE_VIDEO_SOURCE_JNI_ADAPTER_H__
#define __BEE_VIDEO_SOURCE_JNI_ADAPTER_H__

#include "bee/media/video_source.h"
#include "log/logger.h"
#include <jni.h>

namespace bee {

class BeeVideoSourceJniAdapter : public VideoSource {
public:
    BeeVideoSourceJniAdapter(bee_int32_t width, bee_int32_t height, bee_int32_t fps, bool is_screencast);
    virtual ~BeeVideoSourceJniAdapter();
    
public:
    BeeErrorCode open(JNIEnv *jni, jobject j_egl_context);
    long getRtcVideoSource();

protected:
    virtual BeeErrorCode open();
    
private:
    JNIEnv *jni_;
    jobject j_egl_context_;
    Logger logger_;
};

} // namespace bee

#endif // #ifndef __BEE_VIDsEO_SOURCE_JNI_ADAPTER_H__
#endif // #ifdef ANDROID
