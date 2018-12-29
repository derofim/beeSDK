#include "platform/android/com_sohu_tv_bee_BeeVideoRender.h"
#include "platform/android/bee_video_renderer_jni_adapter.h"

JNIEXPORT jlong JNICALL Java_com_sohu_tv_bee_BeeVideoRender_nativeCreateBeeVideoRenderer
        (JNIEnv *env, jclass cls, jlong jRtcVideoRenderer) {
    std::unique_ptr<bee::BeeVideoRendererJniAdapter> beeVideoRendererJniAdapter(
            new bee::BeeVideoRendererJniAdapter());
    if (beeVideoRendererJniAdapter.get()->open(jRtcVideoRenderer) != kBeeErrorCode_Success) {
        return 0;
    } else {
        return (jlong)beeVideoRendererJniAdapter.release();
    }
}

JNIEXPORT void JNICALL Java_com_sohu_tv_bee_BeeVideoRender_nativeFreeBeeVideoRenderer
        (JNIEnv *env, jclass cls, jlong jBeeVideoRenderer) {
    bee::BeeVideoRendererJniAdapter *beeVideoRenderer = reinterpret_cast<bee::BeeVideoRendererJniAdapter*>(jBeeVideoRenderer);
    if (beeVideoRenderer != NULL) {
        delete beeVideoRenderer;
    }
}
