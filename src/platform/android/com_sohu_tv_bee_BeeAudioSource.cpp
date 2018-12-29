#include "platform/android/com_sohu_tv_bee_BeeAudioSource.h"
#include "bee/media/audio_source_default.h"

JNIEXPORT jlong JNICALL Java_com_sohu_tv_bee_BeeAudioSource_nativeCreateBeeAudioSource
        (JNIEnv *env, jclass cls, jboolean level_control, jboolean echo_cancel,
        jboolean gain_control, jboolean high_pass_filter, jboolean noise_suppression) {
    std::unique_ptr<bee::AudioSourceDefault> beeAudioSource(
            new bee::AudioSourceDefault(
                    level_control, echo_cancel, gain_control, high_pass_filter, noise_suppression));
    if (beeAudioSource.get()->open() != kBeeErrorCode_Success) {
        return 0;
    } else {
        return (jlong)beeAudioSource.release();
    }
}

JNIEXPORT void JNICALL Java_com_sohu_tv_bee_BeeAudioSource_nativeFreeBeeAudioSource
        (JNIEnv *env, jclass cls, jlong jBeeAudioSource) {
    bee::AudioSourceDefault *beeAudioSource = reinterpret_cast<bee::AudioSourceDefault*>(jBeeAudioSource);
    if (beeAudioSource != NULL) {
        delete beeAudioSource;
    }
}
