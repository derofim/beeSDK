//
// Created by fangdali on 2018/10/9.
//
#include <jni.h>
#include "webrtc/sdk/android/src/jni/jni_helpers.h"
#include "android_BeeService.h"

namespace bee {

androidBeeServiceAdapter::androidBeeServiceAdapter(JNIEnv* env, jobject obj, int svc_codec)
        : beeService_(new androidBeeService(env, obj, svc_codec)) {
}

androidBeeServiceAdapter::~androidBeeServiceAdapter() {
}

std::shared_ptr<androidBeeService> androidBeeServiceAdapter::getBeeService() {
    return beeService_;
}

androidBeeService::androidBeeService(JNIEnv* env, jobject obj, int svc_codec)
        : jni_(env),
          jobject_(env, obj),
          BeeService(svc_codec) {
    CHECK_EXCEPTION(jni_) << "error during initialization of Java BeeSDKService.";

}

androidBeeService::~androidBeeService() {

}

void androidBeeService::handle_data(const std::string &data) {
    JNIEnv *env = webrtc_jni::AttachCurrentThreadIfNeeded();
    jclass j_sdksService = env->GetObjectClass(*jobject_);
    assert(j_sdksService != NULL);
    jmethodID j_mid =env->GetMethodID(j_sdksService, "handleData", "(Ljava/lang/String;)V");
    assert(j_mid != NULL);

    jstring j_data = webrtc_jni::JavaStringFromStdString(env, data);
    env->CallVoidMethod(*jobject_, j_mid, j_data);
    env->DeleteLocalRef(j_sdksService);
    env->DeleteLocalRef(j_data);
}

}
