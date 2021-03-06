/* DO NOT EDIT THIS FILE - it is machine generated */
#include <jni.h>
/* Header for class com_sohu_tv_bee_BeeVideoSource */

#ifndef _Included_com_sohu_tv_bee_BeeVideoSource
#define _Included_com_sohu_tv_bee_BeeVideoSource
#ifdef __cplusplus
extern "C" {
#endif
/*
 * Class:     com_sohu_tv_bee_BeeVideoSource
 * Method:    nativeCreateBeeVideoSource
 * Signature: (IIILorg/webrtc/EglBase/Context;Z)J
 */
JNIEXPORT jlong JNICALL Java_com_sohu_tv_bee_BeeVideoSource_nativeCreateBeeVideoSource
  (JNIEnv *, jclass, jint, jint, jint, jobject, jboolean);

/*
 * Class:     com_sohu_tv_bee_BeeVideoSource
 * Method:    nativeAdaptOutputFormat
 * Signature: (JIII)V
 */
JNIEXPORT void JNICALL Java_com_sohu_tv_bee_BeeVideoSource_nativeAdaptOutputFormat
  (JNIEnv *, jclass, jlong, jint, jint, jint);

/*
 * Class:     com_sohu_tv_bee_BeeVideoSource
 * Method:    nativeInitializeVideoCapturer
 * Signature: (Landroid/content/Context;Lorg/webrtc/VideoCapturer;JLorg/webrtc/VideoCapturer/CapturerObserver;)V
 */
JNIEXPORT void JNICALL Java_com_sohu_tv_bee_BeeVideoSource_nativeInitializeVideoCapturer
  (JNIEnv *, jclass, jobject, jobject, jlong, jobject);

/*
 * Class:     com_sohu_tv_bee_BeeVideoSource
 * Method:    nativeFreeBeeVideoSource
 * Signature: (J)V
 */
JNIEXPORT void JNICALL Java_com_sohu_tv_bee_BeeVideoSource_nativeFreeBeeVideoSource
  (JNIEnv *, jclass, jlong);

/*
 * Class:     com_sohu_tv_bee_BeeVideoSource
 * Method:    nativeGetRtcVideoSource
 * Signature: (J)J
 */
JNIEXPORT jlong JNICALL Java_com_sohu_tv_bee_BeeVideoSource_nativeGetRtcVideoSource
  (JNIEnv *, jclass, jlong);

#ifdef __cplusplus
}
#endif
#endif
