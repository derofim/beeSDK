#include "bee.h"
#include "bee_sink.h"
#include "service/bee_entrance.h"

namespace bee {

Bee::Ptr Bee::instance_;
Bee::Bee() {
}

Bee::~Bee() {
}

Bee::Ptr Bee::instance() {
    if (instance_ == NULL) {
        if (instance_ == NULL) {
            instance_.reset(new Bee);
        }
    }
    return instance_;
}

void Bee::destroy_instance() {
    if (instance_ != NULL) {
        if (instance_ != NULL) {
            instance_.reset();
        }
    }
}

BeeErrorCode Bee::initialize(const BeeSystemParam &param, std::shared_ptr<BeeSink> sink, bee_int32_t timeout, bee_int32_t &ec2) {
    BeeErrorCode ret = BeeEntrance::instance()->start(param, sink, timeout, ec2);
    if (ret != kBeeErrorCode_Success) {
        BeeEntrance::instance()->force_stop();
        BeeEntrance::destroy_instance();
    }
    return ret;
}

BeeErrorCode Bee::uninitialize() {
    if (BeeEntrance::is_instance_exist()) {
        BeeEntrance::instance()->stop();
        BeeEntrance::destroy_instance();
    }
#if defined(ANDROID) && defined(LEAK_CHECK)
    //Not supported in current newest NDK r16b.
    __lsan_do_recoverable_leak_check();
#endif
    return kBeeErrorCode_Success;
}

BeeErrorCode Bee::open_session(bee_handle &handle, std::vector<BeeCapability> &capability) {
    BeeErrorCode ec = kBeeErrorCode_Success;
    if (BeeEntrance::is_instance_exist()) {
        ec = BeeEntrance::instance()->sync_open_session(handle, capability);
    } else {
        ec = kBeeErrorCode_Service_Not_Started;
    }
    return ec;
}

BeeErrorCode Bee::close_session(bee_handle handle) {
    if (BeeEntrance::is_instance_exist()) {
        return BeeEntrance::instance()->close_session(handle);
    } else {
        return kBeeErrorCode_Service_Not_Started;
    }
}

#ifdef ANDROID
void Bee::set_application_context(JNIEnv* jni, jobject j_context) {
    BeeEntrance::instance()->set_application_context(jni, j_context);
}

BeeErrorCode Bee::set_codec_egl_context(JNIEnv* jni, jobject local_egl_context, jobject remote_egl_context) {
    if (BeeEntrance::is_instance_exist()) {
        return BeeEntrance::instance()->set_codec_egl_context(jni, local_egl_context, remote_egl_context);
    } else {
        return kBeeErrorCode_Service_Not_Started;
    }
}
#elif defined(WIN32)
void Bee::set_video_encoder_factory(cricket::WebRtcVideoEncoderFactory *factory) {
    BeeEntrance::instance()->set_video_encoder_factory(factory);
}

void Bee::set_video_decoder_factory(cricket::WebRtcVideoDecoderFactory *factory) {
    BeeEntrance::instance()->set_video_decoder_factory(factory);
}
#endif
} // namespace bee
