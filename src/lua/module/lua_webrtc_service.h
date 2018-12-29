#ifndef __LUA_WEBRTC_SERVICE_H__
#define __LUA_WEBRTC_SERVICE_H__

#include "utility/common.h"
#include "bee/base/bee_define.h"
#include "comLib/SafeQueue.h"
#include "log/logger.h"
#include "webrtc/api/peerconnectioninterface.h"
#include "webrtc/media/engine/webrtcvideocapturerfactory.h"
#include "webrtc/modules/video_capture/video_capture_factory.h"

#ifdef WIN32
#include "platform/win32/audio_device_module_wrapper.h"
#elif defined(ANDROID)
#include "webrtc/sdk/android/src/jni/androidmediaencoder_jni.h"
#include "webrtc/sdk/android/src/jni/androidmediadecoder_jni.h"
#include "webrtc/sdk/android/src/jni/androidnetworkmonitor_jni.h"
#endif

namespace bee {

typedef struct ADM_t {
    int32_t channels = 0;
    int32_t sample_rate = 0;
    int32_t sample_size = 0;
} ADM_t;

/////////////////////////////////LuaWebrtcService/////////////////////////////////////////
class LuaWebrtcService {
public:
    typedef std::shared_ptr<LuaWebrtcService> Ptr;
    typedef rtc::scoped_refptr<webrtc::PeerConnectionFactoryInterface> FactoryPtr;
    LuaWebrtcService();
    virtual ~LuaWebrtcService();

public:    
    FactoryPtr get_peer_connection_factory();
    void set_session_id(int32_t session_id) { session_id_ = session_id; }
    int32_t get_session_id() { return session_id_; }
    static rtc::Thread *signaling_thread() { return signaling_thread_.get(); }
    static rtc::Thread *worker_thread() { return worker_thread_.get(); }
    static void init_service(
        bool enable_video_encoder_hw_acceleration, 
        bool enable_video_decoder_hw_acceleration,
        bool enable_custom_audio_source);
    static void uninit_service();
    static FactoryPtr peer_connection_factory() { return peer_connection_factory_; }
#ifdef WIN32
    static rtc::scoped_refptr<webrtc::AudioDeviceModule> get_custom_adm() { return custom_adm_; }
    static void set_video_encoder_facotry(cricket::WebRtcVideoEncoderFactory *factory) { win_video_encoder_factory_ = factory; }
    static void set_video_decoder_facotry(cricket::WebRtcVideoDecoderFactory *factory) { win_video_decoder_factory_ = factory; }
#endif

#ifdef ANDROID
    static void attach_jvm();
    static void set_application_context(JNIEnv* jni, jobject context);
    static void set_codec_egl_context(JNIEnv* jni, jobject local_egl_context, jobject remote_egl_context);
#endif

protected:
    static void init_factory(
        bool enable_video_encoder_hw_acceleration, 
        bool enable_video_decoder_hw_acceleration, 
        bool enable_custom_audio_source);
    static void print_thread_name(const std::string &name);

private:
    static FactoryPtr peer_connection_factory_;
    static std::unique_ptr<rtc::Thread> network_thread_;
    static std::unique_ptr<rtc::Thread> worker_thread_;
    static std::unique_ptr<rtc::Thread> signaling_thread_;
#ifdef WIN32
    static rtc::scoped_refptr<webrtc::AudioDeviceModule> custom_adm_;
    static cricket::WebRtcVideoEncoderFactory *win_video_encoder_factory_;
    static cricket::WebRtcVideoDecoderFactory *win_video_decoder_factory_;
#endif
    static Logger logger_;
    int32_t session_id_;
#ifdef ANDROID
    static webrtc_jni::MediaCodecVideoEncoderFactory *android_video_encoder_factory_;
    static webrtc_jni::MediaCodecVideoDecoderFactory *android_video_decoder_factory_;
#endif
};

} // namespace bee

#endif
