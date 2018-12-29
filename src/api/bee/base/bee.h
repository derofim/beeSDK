/**
 *  @file        bee_service.h
 *  @brief       BeeSDK底层通用业务无关接口声明文件.
 *  @author      HeZhen
 *  @copyright   sohu-inc
 *  @version     2.0.0.1
 */

#ifndef __BEE_H__
#define __BEE_H__

#include "bee/base/bee_define.h"
#include <vector>
#include <memory>
#include <string>
#ifdef ANDROID
#include <jni.h>
#elif defined(WIN32)
#include "webrtc/media/engine/webrtcvideoencoderfactory.h"
#include "webrtc/media/engine/webrtcvideodecoderfactory.h"
#endif

namespace bee {

/// BeeSDK底层业务能力结构.
typedef struct BeeCapability {
    /// 业务码.
    bee_int32_t svc_code;
    
    /// 业务描述.
    std::string description;

    /**
     *  @brief  BeeCapability类构造函数.
     *  @param  sc      业务码.
     *  @param  desc    业务描述.
     */
    BeeCapability(bee_int32_t sc, const std::string &desc) {
        svc_code = sc;
        description = desc;
    }
    
    /**
     *  @brief  BeeCapability类拷贝构造函数.
     *  @param  cap     另一个BeeCapability类对象.
     */
    BeeCapability(const BeeCapability &cap) {
        this->svc_code = cap.svc_code;
        this->description = cap.description;
    }
    
    /**
     *  @brief  BeeCapability类=操作符重载函数.
     *  @param  cap     另一个BeeCapability类对象.
     *  @return 当前BeeCapability类对象.
     */
    BeeCapability& operator=(const BeeCapability &cap) {
        if (this != &cap) {
            this->svc_code = cap.svc_code;
            this->description = cap.description;
        }
        return *this;
    }
    
    /**
     *  @brief  BeeCapability类<操作符重载函数，用于排序.
     *  @param  cap     另一个BeeCapability类对象.
     *  @return 比较结果.
     */
    bool operator<(const BeeCapability &cap) const {
        return this->svc_code < cap.svc_code;
    }
}BeeCapability;

class BeeSink;

/// BeeSDK通用接口类.
class Bee {
public:
    typedef std::shared_ptr<Bee> Ptr;
    
    /**
     *  @brief  单例方法.
     *  @return 单例对象.
     */
	static Ptr instance();
    
    /**
     *  @brief  释放单例对象.
     */
	static void destroy_instance();
    
    /**
     *  @brief  Bee类析构函数.
     */
    ~Bee();

public:
    /**
     *  @brief  底层初始化BeeSDK.
     *  @param  param       初始化参数.
     *  @param  sink        获取异步通知的回调对象.
     *  @param  timeout     通用接口的调用超时时间，单位ms.
     *  @param  ec2         平台相关错误码.
     *  @return 系统错误码.
     *  @note   非线程安全，确保在初始化完成前(同时)不要调用其他api.
     */
	BeeErrorCode initialize(const BeeSystemParam &param, std::shared_ptr<BeeSink> sink, bee_int32_t timeout, bee_int32_t &ec2);

    /**
     *  @brief  底层反初始化BeeSDK.
     *  @return 系统错误码.
     *  @note   非线程安全，确保在调用反初始化后(同时)不要调用其他api.
     */
	BeeErrorCode uninitialize();

    /**
     *  @brief  打开一个会话.
     *  @param  handle      [输出] 打开的会话句柄.
     *  @param  capability  [输出] 打开的会话包含的业务能力.
     *  @return 系统错误码.
     */
	BeeErrorCode open_session(bee_handle &handle, std::vector<BeeCapability> &capability);

    /**
     *  @brief  关闭一个会话.
     *  @param  handle      会话句柄.
     *  @return 系统错误码.
     */
	BeeErrorCode close_session(bee_handle handle);

#ifdef ANDROID
    /**
     *  @brief  编解码使用硬件加速时，传递egl上下文.
     *  @param  jni                 android JNIEnv.
     *  @param  local_egl_context   本地egl上下文.
     *  @param  remote_egl_context  外部egl上下文.
     *  @return 系统错误码.
     */
    BeeErrorCode set_codec_egl_context(JNIEnv* jni, jobject local_egl_context, jobject remote_egl_context);

    /**
     *  @brief  设置android上下文.
     *  @param  jni        android JNIEnv.
     *  @param  context    android 上下文.
     *  @note   必须在所有调用之前调用.
     */
    void set_application_context(JNIEnv* jni, jobject context);
#elif defined(WIN32)
    /**
    *  @brief  设置编码器工厂.
    *  @param  factory    编码器工厂.
    *  @note   必须在初始化之前调用.
    */
    void set_video_encoder_factory(cricket::WebRtcVideoEncoderFactory *factory);

    /**
    *  @brief  设置解码器工厂.
    *  @param  factory    解码器工厂.
    *  @note   必须在初始化之前调用.
    */
    void set_video_decoder_factory(cricket::WebRtcVideoDecoderFactory *factory);
#endif
private:
    /**
     *  @brief  Bee类构造函数.
     */
    Bee();

private:
	static Ptr instance_;
    
};

} // namespace bee

#endif // #ifndef __BEE_H__
