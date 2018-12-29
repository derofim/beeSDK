//
// Created by fangdali on 2018/10/9.
//

#ifndef BEEDEMO_ANDROIDBEESERVICE_H
#define BEEDEMO_ANDROIDBEESERVICE_H

#include <bee/base/bee_service.h>
#include "webrtc/sdk/android/src/jni/jni_helpers.h"
#include <jni.h>

namespace bee {

class androidBeeService : public BeeService {
public:
    androidBeeService(JNIEnv* env, jobject obj, int svc_codec);
    ~androidBeeService();

    void handle_data(const std::string &data);

private:
    JNIEnv* jni_;
    webrtc_jni::ScopedGlobalRef<jobject> jobject_;

};

class androidBeeServiceAdapter {
public:
    androidBeeServiceAdapter(JNIEnv* env, jobject jclass, int svc_codec);
    ~androidBeeServiceAdapter();

    std::shared_ptr<androidBeeService> getBeeService();
private:
    std::shared_ptr<androidBeeService> beeService_;
};

}
#endif //BEEDEMO_ANDROIDBEESERVICE_H
