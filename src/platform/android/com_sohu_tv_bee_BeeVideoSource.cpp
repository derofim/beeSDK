#include <webrtc/api/videosourceproxy.h>
#include <platform/android/androidvideotracksource.h>
#include "platform/android/com_sohu_tv_bee_BeeVideoSource.h"
#include "platform/android/bee_video_source_jni_adapter.h"

/*
 * Class:     com_sohu_tv_bee_BeeVideoSource
 * Method:    nativeCreateBeeVideoSource
 * Signature: (IIILorg/webrtc/EglBase/Context;Z)J
 */
JNIEXPORT jlong JNICALL Java_com_sohu_tv_bee_BeeVideoSource_nativeCreateBeeVideoSource
        (JNIEnv *env, jclass cls, jint jWidth, jint jHeight, jint jFps, jobject j_egl_context, jboolean is_screencast) {
    std::unique_ptr<bee::BeeVideoSourceJniAdapter> beeVideoSource(new bee::BeeVideoSourceJniAdapter(jWidth, jHeight, jFps, is_screencast));
    if (beeVideoSource.get()->open(env, j_egl_context) != kBeeErrorCode_Success) {
        return 0;
    } else {
        return (jlong)beeVideoSource.release();
    }
}

/*
 * Class:     com_sohu_tv_bee_BeeVideoSource
 * Method:    nativeAdaptOutputFormat
 * Signature: (JIII)V
 */
JNIEXPORT void JNICALL Java_com_sohu_tv_bee_BeeVideoSource_nativeAdaptOutputFormat
        (JNIEnv *env, jclass cls, jlong jBeeVideoSource, jint j_width, jint j_height, jint j_fps) {
    LOG(LS_INFO) << "BeeVideoSource_nativeAdaptOutputFormat";
    bee::BeeVideoSourceJniAdapter *beeVideoSource = reinterpret_cast<bee::BeeVideoSourceJniAdapter*>(jBeeVideoSource);
    if (beeVideoSource != NULL) {
        auto proxy_source = reinterpret_cast<webrtc::VideoTrackSourceProxy*>(beeVideoSource->getRtcVideoSource());
        auto source = reinterpret_cast<bee::AndroidVideoTrackSource*>(proxy_source->internal());
        source->OnOutputFormatRequest(j_width, j_height, j_fps);
    }
}

/*
 * Class:     com_sohu_tv_bee_BeeVideoSource
 * Method:    nativeInitializeVideoCapturer
 * Signature: (Landroid/content/Context;Lorg/webrtc/VideoCapturer;JLorg/webrtc/VideoCapturer/CapturerObserver;)V
 */
JNIEXPORT void JNICALL Java_com_sohu_tv_bee_BeeVideoSource_nativeInitializeVideoCapturer
        (JNIEnv *env, jclass cls, jobject j_application_context, jobject j_video_capturer, jlong jBeeVideoSource, jobject j_frame_observer) {
    LOG(LS_INFO) << "BeeVideoSource_nativeInitializeVideoCapturer";
    bee::BeeVideoSourceJniAdapter *beeVideoSource = reinterpret_cast<bee::BeeVideoSourceJniAdapter*>(jBeeVideoSource);
    if (beeVideoSource != NULL) {
        auto proxy_source = reinterpret_cast<webrtc::VideoTrackSourceProxy *>(beeVideoSource->getRtcVideoSource());
        auto source = reinterpret_cast<bee::AndroidVideoTrackSource *>(proxy_source->internal());
        rtc::scoped_refptr<webrtc_jni::SurfaceTextureHelper> surface_texture_helper = source->surface_texture_helper();
        env->CallVoidMethod(
                j_video_capturer,
                env->GetMethodID(env->FindClass("org/webrtc/VideoCapturer"), "initialize",
                                 "(Lorg/webrtc/SurfaceTextureHelper;Landroid/content/"
                                         "Context;Lorg/webrtc/VideoCapturer$CapturerObserver;)V"),
                surface_texture_helper
                ? surface_texture_helper->GetJavaSurfaceTextureHelper()
                : nullptr,
                j_application_context, j_frame_observer);
    }
    CHECK_EXCEPTION(env) << "error during VideoCapturer.initialize()";
}

/*
 * Class:     com_sohu_tv_bee_BeeVideoSource
 * Method:    nativeFreeBeeVideoSource
 * Signature: (J)V
 */
JNIEXPORT void JNICALL Java_com_sohu_tv_bee_BeeVideoSource_nativeFreeBeeVideoSource
        (JNIEnv *env, jclass cls, jlong jBeeVideoSource) {
    bee::BeeVideoSourceJniAdapter *beeVideoSource = reinterpret_cast<bee::BeeVideoSourceJniAdapter*>(jBeeVideoSource);
    if (beeVideoSource != NULL) {
        delete beeVideoSource;
    }
}

/*
 * Class:     com_sohu_tv_bee_BeeVideoSource
 * Method:    nativeGetRtcVideoSource
 * Signature: (J)J
 */
JNIEXPORT jlong JNICALL Java_com_sohu_tv_bee_BeeVideoSource_nativeGetRtcVideoSource
        (JNIEnv *env, jclass cls, jlong jBeeVideoSource) {
    bee::BeeVideoSourceJniAdapter *beeVideoSource = reinterpret_cast<bee::BeeVideoSourceJniAdapter*>(jBeeVideoSource);
    if (beeVideoSource != NULL) {
        return beeVideoSource->getRtcVideoSource();
    } else {
        return 0;
    }
}
