//
// Created by fangdali on 2018/10/10.
//

#ifdef ANDROID

#include <stdio.h>
#include <string>
#include <android/log.h>
#include "bee/base/bee.h"
#include "webrtc/modules/utility/include/jvm_android.h"
#include "webrtc/sdk/android/src/jni/jni_helpers.h"
#include "com_sohu_tv_bee_BeeSDK.h"

using namespace bee;

#define TAG "bee-jni"
#define LOGD(...)  __android_log_print(ANDROID_LOG_DEBUG,TAG,__VA_ARGS__)
#define LOGI(...)  __android_log_print(ANDROID_LOG_INFO,TAG,__VA_ARGS__)
#define LOGW(...)  __android_log_print(ANDROID_LOG_WARN,TAG,__VA_ARGS__)
#define LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,TAG,__VA_ARGS__)
#define LOGF(...)  __android_log_print(ANDROID_LOG_FATAL,TAG,__VA_ARGS__)

static bool jvm_initialized = false;
static JavaVM *g_javaVM = NULL;

#define DEBUG

#ifdef DEBUG

static int g_pfd[2];
static pthread_t g_stderr_log_thread;
static const char *g_tag = "bee";
static bool g_capture_stderr = false;

static void *thread_func(void*) {
    ssize_t rdsz;
    char buf[128] = {0};
    while (g_capture_stderr && (rdsz = read(g_pfd[0], buf, sizeof(buf) - 1)) > 0) {
        if (buf[rdsz - 1] == '\n') {
            --rdsz;
        }
        buf[rdsz] = 0;  /* add null-terminator */
        __android_log_write(ANDROID_LOG_DEBUG, g_tag, buf);
    }
    return 0;
}

int start_stderr_logger() {
    /* make stdout line-buffered and stderr unbuffered */
    setvbuf(stdout, 0, _IOLBF, 0);
    setvbuf(stderr, 0, _IONBF, 0);

    /* create the pipe and redirect stdout and stderr */
    pipe(g_pfd);
    dup2(g_pfd[1], 1);
    dup2(g_pfd[1], 2);

    g_capture_stderr = true;

    /* spawn the logging thread */
    if (pthread_create(&g_stderr_log_thread, 0, thread_func, 0) == -1) {
        return -1;
    }

    pthread_detach(g_stderr_log_thread);
    return 0;
}

void stop_stderr_logger() {
    g_capture_stderr = false;
}

#endif

bool init_jvm(JNIEnv *env, jobject app_context) {
    bool ret = true;
    do {
        if (jvm_initialized) {
            break;
        }

        if (env == NULL) {
            ret = false;
            break;
        }

        env->GetJavaVM(&g_javaVM);
        if (g_javaVM == NULL) {
            ret = false;
            break;
        }

        jobject jac = env->NewGlobalRef(app_context);
        LOGD("app_context %x, jac %x\n", (unsigned int)app_context, (unsigned int)jac);
        webrtc::JVM::Initialize(reinterpret_cast<JavaVM*>(g_javaVM), jac);
        jvm_initialized = true;
    } while (0);
    return ret;
}

bool uninit_jvm(JNIEnv *env) {
    bool ret = true;
    do {
        if (!jvm_initialized) {
            break;
        }

        if (env == NULL) {
            ret = false;
            break;
        }

        webrtc::JVM::Uninitialize();

        jvm_initialized = false;
    } while (0);
    return ret;
}

void BEE_CALLBACK bee_system_notify(bee_handle handle, BeeErrorCode ec1, bee_int32_t ec2, const char *message, void *opaque) {

}

/*
 * Class:     com_sohu_tv_bee_BeeSDK
 * Method:    nativeBeeSyncInit
 * Signature: (Landroid/content/Context;Lcom/sohu/tv/bee/BeeSystemParam;ILcom/sohu/tv/bee/BeeSDKSink;)I
 */
JNIEXPORT jint JNICALL Java_com_sohu_tv_bee_BeeSDK_nativeBeeSyncInit
        (JNIEnv *env, jclass cls, jobject app_context, jobject j_bee_param, jint j_timeout, jobject j_sink) {
    jobject j_app_name          = NULL;
    jobject j_app_version       = NULL;
    jobject j_system_info       = NULL;
    jobject j_machine_code      = NULL;
    jobject j_log_path          = NULL;
    const char *app_name        = NULL;
    const char *app_version     = NULL;
    const char *system_info     = NULL;
    const char *machine_code    = NULL;
    const char *log_path        = NULL;

    BeeErrorCode ret = kBeeErrorCode_Success;
    do {
#ifdef DEBUG
        start_stderr_logger();
#endif

        if (!init_jvm(env, app_context)) {
            ret = kBeeErrorCode_General_Error;
            break;
        }

        if (j_bee_param == NULL) {
            ret = kBeeErrorCode_Null_Pointer;
            break;
        }

        jclass j_bee_param_class = env->GetObjectClass(j_bee_param);
        if (j_bee_param_class == NULL) {
            ret = kBeeErrorCode_Invalid_Param;
            break;
        }

        jfieldID platform_type_filed = env->GetFieldID(j_bee_param_class, "platform_type", "I");
        if (platform_type_filed == NULL) {
            ret = kBeeErrorCode_Invalid_Param;
            break;
        }

        jfieldID app_name_filed = env->GetFieldID(j_bee_param_class, "app_name", "Ljava/lang/String;");
        if (app_name_filed == NULL) {
            ret = kBeeErrorCode_Invalid_Param;
            break;
        }

        jfieldID app_version_filed = env->GetFieldID(j_bee_param_class, "app_version", "Ljava/lang/String;");
        if (app_version_filed == NULL) {
            ret = kBeeErrorCode_Invalid_Param;
            break;
        }

        jfieldID system_info_filed = env->GetFieldID(j_bee_param_class, "system_info", "Ljava/lang/String;");
        if (system_info_filed == NULL) {
            ret = kBeeErrorCode_Invalid_Param;
            break;
        }

        jfieldID machine_code_filed = env->GetFieldID(j_bee_param_class, "machine_code", "Ljava/lang/String;");
        if (machine_code_filed == NULL) {
            ret = kBeeErrorCode_Invalid_Param;
            break;
        }

        jfieldID log_path_filed = env->GetFieldID(j_bee_param_class, "log_path", "Ljava/lang/String;");
        if (log_path_filed == NULL) {
            ret = kBeeErrorCode_Invalid_Param;
            break;
        }

        jfieldID log_level_filed = env->GetFieldID(j_bee_param_class, "log_level", "I");
        if (log_level_filed == NULL) {
            ret = kBeeErrorCode_Invalid_Param;
            break;
        }

        jfieldID log_max_line_filed = env->GetFieldID(j_bee_param_class, "log_max_line", "I");
        if (log_max_line_filed == NULL) {
            ret = kBeeErrorCode_Invalid_Param;
            break;
        }

        jfieldID log_volume_count_filed = env->GetFieldID(j_bee_param_class, "log_volume_count", "I");
        if (log_volume_count_filed == NULL) {
            ret = kBeeErrorCode_Invalid_Param;
            break;
        }

        jfieldID log_volume_size_filed = env->GetFieldID(j_bee_param_class, "log_volume_size", "I");
        if (log_volume_size_filed == NULL) {
            ret = kBeeErrorCode_Invalid_Param;
            break;
        }

        jfieldID session_count_filed = env->GetFieldID(j_bee_param_class, "session_count", "I");
        if (session_count_filed == NULL) {
            ret = kBeeErrorCode_Invalid_Param;
            break;
        }

        jfieldID enable_statusd_field = env->GetFieldID(j_bee_param_class, "enable_statusd", "Z");
        jfieldID enable_video_encoder_hw_field = env->GetFieldID(j_bee_param_class, "enable_video_encoder_hw", "Z");
        jfieldID enable_video_decoder_hw_field = env->GetFieldID(j_bee_param_class, "enable_video_decoder_hw", "Z");

        jint platform_type = env->GetIntField(j_bee_param, platform_type_filed);
        jboolean enable_statusd = env->GetBooleanField(j_bee_param, enable_statusd_field);
        jboolean enable_video_encoder_hw = env->GetBooleanField(j_bee_param, enable_video_encoder_hw_field);
        jboolean enable_video_decoder_hw = env->GetBooleanField(j_bee_param, enable_video_decoder_hw_field);

        j_app_name = env->GetObjectField(j_bee_param, app_name_filed);
        if (j_app_name == NULL) {
            ret = kBeeErrorCode_Null_Pointer;
            break;
        }

        app_name = env->GetStringUTFChars((jstring)j_app_name, NULL);
        if (app_name == NULL) {
            ret = kBeeErrorCode_Not_Enough_Memory;
            break;
        }

        j_app_version = env->GetObjectField(j_bee_param, app_version_filed);
        if (j_app_version == NULL) {
            ret = kBeeErrorCode_Null_Pointer;
            break;
        }

        app_version = env->GetStringUTFChars((jstring)j_app_version, NULL);
        if (app_version == NULL) {
            ret = kBeeErrorCode_Not_Enough_Memory;
            break;
        }

        j_system_info = env->GetObjectField(j_bee_param, system_info_filed);
        if (j_system_info == NULL) {
            ret = kBeeErrorCode_Null_Pointer;
            break;
        }

        system_info = env->GetStringUTFChars((jstring)j_system_info, NULL);
        if (system_info == NULL) {
            ret = kBeeErrorCode_Not_Enough_Memory;
            break;
        }

        j_machine_code = env->GetObjectField(j_bee_param, machine_code_filed);
        if (j_machine_code == NULL) {
            ret = kBeeErrorCode_Null_Pointer;
            break;
        }

        machine_code = env->GetStringUTFChars((jstring)j_machine_code, NULL);
        if (machine_code == NULL) {
            ret = kBeeErrorCode_Not_Enough_Memory;
            break;
        }

        j_log_path = env->GetObjectField(j_bee_param, log_path_filed);
        if (j_log_path == NULL) {
            ret = kBeeErrorCode_Null_Pointer;
            break;
        }

        log_path = env->GetStringUTFChars((jstring)j_log_path, NULL);
        if (log_path == NULL) {
            ret = kBeeErrorCode_Not_Enough_Memory;
            break;
        }

        jint log_level = env->GetIntField(j_bee_param, log_level_filed);
        jint log_max_line = env->GetIntField(j_bee_param, log_max_line_filed);
        jint log_volume_count = env->GetIntField(j_bee_param, log_volume_count_filed);
        jint log_volume_size = env->GetIntField(j_bee_param, log_volume_size_filed);
        jint session_count = env->GetIntField(j_bee_param, session_count_filed);

        BeeSystemParam c_bee_param;
        memset(&c_bee_param, 0, sizeof(BeeSystemParam));
        c_bee_param.platform_type   = static_cast<BeePlatformType>(platform_type);
        c_bee_param.net_type        = static_cast<BeeNetType>(0);
        c_bee_param.app_name        = (char*)app_name;
        c_bee_param.app_version     = (char*)app_version;
        c_bee_param.system_info     = (char*)system_info;
        c_bee_param.machine_code    = (char*)machine_code;
        c_bee_param.log_path        = (char*)log_path;
        c_bee_param.log_level       = static_cast<BeeLogLevel>(log_level);
        c_bee_param.log_max_line    = log_max_line;
        c_bee_param.log_volume_count= log_volume_count;
        c_bee_param.log_volume_size = log_volume_size;
        c_bee_param.session_count   = session_count;
        c_bee_param.enable_statusd  = enable_statusd;
        c_bee_param.enable_video_encoder_hw_acceleration  = enable_video_encoder_hw;
        c_bee_param.enable_video_decoder_hw_acceleration  = enable_video_decoder_hw;

        bee_int32_t ec2 = 0;
        Bee::instance()->set_application_context(env, app_context);
        ret = Bee::instance()->initialize(c_bee_param, NULL, j_timeout, ec2);
    } while (0);

    if (env != NULL && j_app_name != NULL && app_name != NULL) {
        env->ReleaseStringUTFChars((jstring)j_app_name, app_name);
    }

    if (env != NULL && j_app_version != NULL && app_version != NULL) {
        env->ReleaseStringUTFChars((jstring)j_app_version, app_version);
    }

    if (env != NULL && j_system_info != NULL && system_info != NULL) {
        env->ReleaseStringUTFChars((jstring)j_system_info, system_info);
    }

    if (env != NULL && j_machine_code != NULL && machine_code != NULL) {
        env->ReleaseStringUTFChars((jstring)j_machine_code, machine_code);
    }

    if (env != NULL && j_log_path != NULL && log_path != NULL) {
        env->ReleaseStringUTFChars((jstring)j_log_path, log_path);
    }

    LOGD("bee init ret %d\n", ret);
    return (jint)ret;
}

/*
 * Class:     com_sohu_tv_bee_BeeSDK
 * Method:    nativeBeeSyncUninit
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_com_sohu_tv_bee_BeeSDK_nativeBeeSyncUninit(JNIEnv *env, jclass cls) {
    BeeErrorCode ret = kBeeErrorCode_Success;
    do {
#ifdef DEBUG
        stop_stderr_logger();
#endif
        ret = Bee::instance()->uninitialize();
        if (!uninit_jvm(env)) {
            ret = kBeeErrorCode_General_Error;
        }
    } while (false);

    return (jint)ret;
}

/*
 * Class:     com_sohu_tv_bee_BeeSDK
 * Method:    nativeBeeSyncOpenSession
 * Signature: ()Lcom/sohu/tv/bee/BeeOpenSessionParam;
 */
JNIEXPORT jobject JNICALL Java_com_sohu_tv_bee_BeeSDK_nativeBeeSyncOpenSession(JNIEnv *env, jclass cls) {
    BeeErrorCode ret = kBeeErrorCode_Success;
    jobject args;
    do {
        std::vector<BeeCapability> beeCapabilitys;
        bee_handle handler = -1;
        ret = Bee::instance()->open_session(handler, beeCapabilitys);

        jclass j_class_vector = env->FindClass("java/util/Vector");
        jmethodID j_vector_init = env->GetMethodID(j_class_vector, "<init>", "()V");
        jmethodID j_vector_add_method= env->GetMethodID(j_class_vector,"add","(Ljava/lang/Object;)Z");
        jobject j_vector_obj = env->NewObject(j_class_vector, j_vector_init, "");

        jclass j_beeOpenSessionParam = env->FindClass("com/sohu/tv/bee/BeeOpenSessionParam");
        assert(j_beeOpenSessionParam != NULL);
        jmethodID mid = env->GetMethodID(j_beeOpenSessionParam, "<init>", "(IILjava/util/Vector;)V");
        assert(mid != NULL);
        args = env->NewObject(j_beeOpenSessionParam, mid, handler, ret, j_vector_obj);

        jfieldID j_handle = env->GetFieldID(j_beeOpenSessionParam, "handle", "I");
        jfieldID j_errorCode = env->GetFieldID(j_beeOpenSessionParam, "errorCode", "I");
        jfieldID j_capability_field = env->GetFieldID(j_beeOpenSessionParam, "capabilities", "Ljava/util/Vector;");

        jclass j_beeSDKCapability = env->FindClass("com/sohu/tv/bee/BeeOpenSessionParam$BeeSDKCapability");
        assert(j_beeSDKCapability != NULL);
        jmethodID j_capability_mid = env->GetMethodID(j_beeSDKCapability, "<init>", "(Lcom/sohu/tv/bee/BeeOpenSessionParam;ILjava/lang/String;)V");
        assert(j_capability_mid != NULL);

        for (BeeCapability capability : beeCapabilitys) {
            jstring j_descriotion = env->NewStringUTF(capability.description.c_str());
            jobject j_capability = env->NewObject(j_beeSDKCapability, j_capability_mid, args, capability.svc_code, j_descriotion);
            env->CallBooleanMethod(j_vector_obj, j_vector_add_method, j_capability);
            env->DeleteLocalRef(j_descriotion);
            env->DeleteLocalRef(j_capability);
        }

        env->SetIntField(args, j_handle, handler);
        env->SetIntField(args, j_errorCode, ret);
        env->SetObjectField(args, j_capability_field, j_vector_obj);
        env->DeleteLocalRef(j_vector_obj);
    } while (false);

    return args;
}

/*
 * Class:     com_sohu_tv_bee_BeeSDK
 * Method:    nativeBeeSyncCloseSession
 * Signature: (I)I
 */
JNIEXPORT jint JNICALL Java_com_sohu_tv_bee_BeeSDK_nativeBeeSyncCloseSession(JNIEnv *env, jclass cls, jint j_handle) {
    BeeErrorCode ret = kBeeErrorCode_Success;
    do {
        Bee::instance()->close_session(j_handle);
    } while (false);
    return (jint)ret;
}

/*
 * Class:     com_sohu_tv_bee_BeeSDK
 * Method:    nativeSetCodecEglContext
 * Signature: (Lorg/webrtc/EglBase/Context;Lorg/webrtc/EglBase/Context;)I
 */
JNIEXPORT jint JNICALL Java_com_sohu_tv_bee_BeeSDK_nativeSetCodecEglContext
        (JNIEnv *env, jclass cls, jobject j_localEglBase, jobject j_remoteEglBase) {
    BeeErrorCode ret = kBeeErrorCode_Success;
    do {
        Bee::instance()->set_codec_egl_context(env, j_localEglBase, j_remoteEglBase);
    } while (false);
    return (jint)ret;
}

#endif