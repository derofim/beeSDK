
#include "lua_webrtc_service.h"
#include "lua_webrtc_module.h"
#ifdef ANDROID
#elif defined(LINUX)
#else
#include "webrtc/api/audio_codecs/builtin_audio_encoder_factory.h"
#include "webrtc/api/audio_codecs/builtin_audio_decoder_factory.h"
#ifdef IOS
#include "platform/ios/ios_adapter.h"
#else //WIN32
#include "platform/win32/win_adapter.h"
#endif
#endif

namespace bee {
    
//////////////////////////////////LuaWebrtcService////////////////////////////////////////
LuaWebrtcService::FactoryPtr LuaWebrtcService::peer_connection_factory_;
std::unique_ptr<rtc::Thread> LuaWebrtcService::network_thread_;
std::unique_ptr<rtc::Thread> LuaWebrtcService::worker_thread_;
std::unique_ptr<rtc::Thread> LuaWebrtcService::signaling_thread_;
#ifdef WIN32
rtc::scoped_refptr<webrtc::AudioDeviceModule> LuaWebrtcService::custom_adm_;
cricket::WebRtcVideoEncoderFactory *LuaWebrtcService::win_video_encoder_factory_ = NULL;
cricket::WebRtcVideoDecoderFactory *LuaWebrtcService::win_video_decoder_factory_ = NULL;
#endif
#ifdef ANDROID
webrtc_jni::MediaCodecVideoEncoderFactory *LuaWebrtcService::android_video_encoder_factory_ = NULL;
webrtc_jni::MediaCodecVideoDecoderFactory *LuaWebrtcService::android_video_decoder_factory_ = NULL;
#endif

Logger LuaWebrtcService::logger_("LuaWebrtcService");

LuaWebrtcService::LuaWebrtcService() {
}

LuaWebrtcService::~LuaWebrtcService() {
}


LuaWebrtcService::FactoryPtr LuaWebrtcService::get_peer_connection_factory() {
    return peer_connection_factory_;
}

void LuaWebrtcService::init_factory(
    bool enable_video_encoder_hw_acceleration, 
    bool enable_video_decoder_hw_acceleration, 
    bool enable_custom_audio_source) {
    if (peer_connection_factory_ == nullptr) {
        cricket::WebRtcVideoEncoderFactory *video_encoder_factory = NULL;
        cricket::WebRtcVideoDecoderFactory *video_decoder_factory = NULL;
        rtc::NetworkMonitorFactory *network_monitor_factory = NULL;
        webrtc::AudioDeviceModule *adm = NULL;
#ifdef ANDROID
        if (enable_video_encoder_hw_acceleration) {
            logger_.Info("Using hardware encoder.\n");
            android_video_encoder_factory_ = new webrtc_jni::MediaCodecVideoEncoderFactory();
            video_encoder_factory = android_video_encoder_factory_;
        } else {
            logger_.Info("Using software encoder.\n");
        }

        if (enable_video_decoder_hw_acceleration) {
            logger_.Info("Using hardware decoder.\n");
            android_video_decoder_factory_ = new webrtc_jni::MediaCodecVideoDecoderFactory();
            video_decoder_factory = android_video_decoder_factory_;
        } else {
            logger_.Info("Using software decoder.\n");
        }

        if (/*enable_network_monitor*/true) {
            logger_.Info("Using net monitor.\n");
            network_monitor_factory = new webrtc_jni::AndroidNetworkMonitorFactory();
            rtc::NetworkMonitorFactory::SetFactory(network_monitor_factory);
        } else {
            logger_.Info("Not using net monitor.\n");
        }

		peer_connection_factory_ = webrtc::CreatePeerConnectionFactory(
			network_thread_.get(),
			worker_thread_.get(),
			signaling_thread_.get(),
            adm,
			video_encoder_factory,
			video_decoder_factory);
#elif defined(LINUX)
        peer_connection_factory_ = webrtc::CreatePeerConnectionFactory(
            network_thread_.get(),
            worker_thread_.get(),
            signaling_thread_.get(),
            adm,
            video_encoder_factory,
            video_decoder_factory);
#else
		rtc::scoped_refptr<webrtc::AudioEncoderFactory> audio_encoder_factory = webrtc::CreateBuiltinAudioEncoderFactory();
		rtc::scoped_refptr<webrtc::AudioDecoderFactory> audio_decoder_factory = webrtc::CreateBuiltinAudioDecoderFactory();
#ifdef IOS
        video_encoder_factory = IOSAdapter::create_video_encoder_facotory();
        video_decoder_factory = IOSAdapter::create_video_decoder_facotory();
#else //WIN
        if (enable_custom_audio_source) {
            custom_adm_ = AudioDeviceModuleWrapper::Create();
            adm = custom_adm_.get();
        }

        video_encoder_factory = win_video_encoder_factory_;
#endif        
        peer_connection_factory_ = webrtc::CreatePeerConnectionFactory(
            network_thread_.get(),
            worker_thread_.get(),
            signaling_thread_.get(),
            adm,
            audio_encoder_factory,
            audio_decoder_factory,
            video_encoder_factory,
            video_decoder_factory);
#endif
    }
}
    
void LuaWebrtcService::print_thread_name(const std::string &name) {
    logger_.Info("@@@@@@@ Webrtc thread %s started\n", name.c_str());
}

void LuaWebrtcService::init_service(
    bool enable_video_encoder_hw_acceleration,
    bool enable_video_decoder_hw_acceleration,
    bool enable_custom_audio_source) {
    rtc::ThreadManager::Instance()->WrapCurrentThread();
    if (network_thread_ == nullptr) {
        network_thread_ = rtc::Thread::CreateWithSocketServer();
        network_thread_->SetName("network_thread", nullptr);
        RTC_CHECK(network_thread_->Start()) << "Failed to start network thread";
        network_thread_->Invoke<void>(RTC_FROM_HERE, rtc::Bind(&LuaWebrtcService::print_thread_name, "network_thread"));
    }

    if (worker_thread_ == nullptr) {
        worker_thread_ = rtc::Thread::Create();
        worker_thread_->SetName("worker_thread", nullptr);
        RTC_CHECK(worker_thread_->Start()) << "Failed to start worker thread";

#ifdef ANDROID
        //Worker thread will send frame to java, so must attach first.
        worker_thread_->Invoke<void>(RTC_FROM_HERE, rtc::Bind(&LuaWebrtcService::attach_jvm));
#endif
        worker_thread_->Invoke<void>(RTC_FROM_HERE, rtc::Bind(&LuaWebrtcService::print_thread_name, "worker_thread"));
    }

    if (signaling_thread_ == nullptr) {
        signaling_thread_ = rtc::Thread::Create();
        signaling_thread_->SetName("signaling_thread", nullptr);
        RTC_CHECK(signaling_thread_->Start()) << "Failed to start signaling thread";
        signaling_thread_->Invoke<void>(RTC_FROM_HERE, rtc::Bind(&LuaWebrtcService::print_thread_name, "signaling_thread"));
    }

    init_factory(enable_video_encoder_hw_acceleration, enable_video_decoder_hw_acceleration, enable_custom_audio_source);
}

void LuaWebrtcService::uninit_service() {
    if (peer_connection_factory_ != NULL) {
        peer_connection_factory_ = NULL;
    }       

    if (network_thread_ != NULL) {
        network_thread_.reset();
    }

    if (worker_thread_ != NULL) {
        worker_thread_.reset();
    }

    if (signaling_thread_ != NULL) {
        signaling_thread_.reset();
    }

#ifdef WIN32
    custom_adm_ = NULL;
#endif

    rtc::ThreadManager::Instance()->UnwrapCurrentThread();
}

#ifdef ANDROID
void LuaWebrtcService::attach_jvm() {
    webrtc_jni::AttachCurrentThreadIfNeeded();
}

void LuaWebrtcService::set_application_context(JNIEnv* jni, jobject context) {
    webrtc_jni::AndroidNetworkMonitor::SetAndroidContext(jni, context);
}

void LuaWebrtcService::set_codec_egl_context(JNIEnv* jni, jobject local_egl_context, jobject remote_egl_context) {
    if (android_video_encoder_factory_ != NULL) {
        android_video_encoder_factory_->SetEGLContext(jni, local_egl_context);
    }

    if (android_video_decoder_factory_ != NULL) {
        android_video_decoder_factory_->SetEGLContext(jni, remote_egl_context);
    }
}
#endif

} // namespace bee
