//
// Created by fangdali on 2018/10/10.
//

#ifdef ANDROID

#include <bee/base/bee.h>
#include <bee/base/bee_service.h>
#include "com_sohu_tv_bee_BeeSDKService.h"
#include "android_BeeService.h"

using namespace bee;

/*
 * Class:     com_sohu_tv_bee_BeeSDKService
 * Method:    nativeCreateSDKService
 * Signature: (I)J
 */
JNIEXPORT jlong JNICALL Java_com_sohu_tv_bee_BeeSDKService_nativeCreateSDKService(JNIEnv *env, jobject obj, jint j_svc) {
    std::unique_ptr<androidBeeServiceAdapter> beeService(new androidBeeServiceAdapter(env, obj, j_svc));
    return (jlong)beeService.release();
}

/*
 * Class:     com_sohu_tv_bee_BeeSDKService
 * Method:    nativeFreeSDKService
 * Signature: (J)V
 */
JNIEXPORT void JNICALL Java_com_sohu_tv_bee_BeeSDKService_nativeFreeSDKService(JNIEnv *env, jobject obj, jlong jsdkService) {
    androidBeeServiceAdapter *beeService = reinterpret_cast<androidBeeServiceAdapter*>(jsdkService);
    if (beeService != NULL) {
        delete beeService;
    }
}

/*
 * Class:     com_sohu_tv_bee_BeeSDKService
 * Method:    nativeReg
 * Signature: (JI)I
 */
JNIEXPORT jint JNICALL Java_com_sohu_tv_bee_BeeSDKService_nativeReg(JNIEnv *env, jobject obj, jlong j_service, jint j_handle) {
    BeeErrorCode ret = kBeeErrorCode_Success;
    androidBeeServiceAdapter *serviceAdapter = reinterpret_cast<androidBeeServiceAdapter*>(j_service);
    if (serviceAdapter != NULL) {
        ret = serviceAdapter->getBeeService()->reg(j_handle);
    }

    return (jint)ret;
}

/*
 * Class:     com_sohu_tv_bee_BeeSDKService
 * Method:    nativeUnReg
 * Signature: (J)I
 */
JNIEXPORT jint JNICALL Java_com_sohu_tv_bee_BeeSDKService_nativeUnReg(JNIEnv *env, jobject obj, jlong j_service) {
    BeeErrorCode ret = kBeeErrorCode_Success;
    androidBeeServiceAdapter *serviceAdapter = reinterpret_cast<androidBeeServiceAdapter*>(j_service);
    if (serviceAdapter != NULL) {
        ret = serviceAdapter->getBeeService()->unreg();
    }

    return (jint)ret;
}

/*
 * Class:     com_sohu_tv_bee_BeeSDKService
 * Method:    nativeExecute
 * Signature: (JLjava/lang/String;Ljava/lang/String;I)I
 */
JNIEXPORT jint JNICALL Java_com_sohu_tv_bee_BeeSDKService_nativeExecute
        (JNIEnv *env, jobject obj, jlong j_service, jstring j_cmd, jstring j_args, jint j_timeout) {
    BeeErrorCode ret = kBeeErrorCode_Success;
    const char *cmd        = NULL;
    const char *args        = NULL;

    do{
        if (j_cmd == NULL || j_args == NULL) {
            ret = kBeeErrorCode_Null_Pointer;
            break;
        }

        cmd = env->GetStringUTFChars((jstring)j_cmd, NULL);
        if (cmd == NULL) {
            ret = kBeeErrorCode_Not_Enough_Memory;
            break;
        }

        args = env->GetStringUTFChars((jstring)j_args, NULL);
        if (args == NULL) {
            ret = kBeeErrorCode_Not_Enough_Memory;
            break;
        }

        androidBeeServiceAdapter *serviceAdapter = reinterpret_cast<androidBeeServiceAdapter*>(j_service);
        if (serviceAdapter != NULL) {
            ret = serviceAdapter->getBeeService()->execute(cmd, args, j_timeout);
        }
    } while (0);

    if (env != NULL && j_cmd != NULL && cmd != NULL) {
        env->ReleaseStringUTFChars((jstring)j_cmd, cmd);
    }

    if (env != NULL && j_args != NULL && args != NULL) {
        env->ReleaseStringUTFChars((jstring)j_args, args);
    }

    return (jint)ret;
}

#endif