/* DO NOT EDIT THIS FILE - it is machine generated */
#include <jni.h>
/* Header for class com_sohu_tv_bee_BeeAudioSource */

#ifndef _Included_com_sohu_tv_bee_BeeAudioSource
#define _Included_com_sohu_tv_bee_BeeAudioSource
#ifdef __cplusplus
extern "C" {
#endif
/*
 * Class:     com_sohu_tv_bee_BeeAudioSource
 * Method:    nativeCreateBeeAudioSource
 * Signature: (ZZZZZ)J
 */
JNIEXPORT jlong JNICALL Java_com_sohu_tv_bee_BeeAudioSource_nativeCreateBeeAudioSource
        (JNIEnv *, jclass, jboolean, jboolean, jboolean, jboolean, jboolean);

/*
 * Class:     com_sohu_tv_bee_BeeAudioSource
 * Method:    nativeFreeBeeAudioSource
 * Signature: (J)V
 */
JNIEXPORT void JNICALL Java_com_sohu_tv_bee_BeeAudioSource_nativeFreeBeeAudioSource
  (JNIEnv *, jclass, jlong);

#ifdef __cplusplus
}
#endif
#endif