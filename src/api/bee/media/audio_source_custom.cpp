#include "bee/media/audio_source_custom.h"
#include "internal/audio_source_internal.h"
#include "lua/module/lua_webrtc_service.h"
#include "webrtc/api/test/fakeconstraints.h"

namespace bee {

AudioSourceCustom::AudioSourceCustom(
    int32_t channels,
    int32_t sample_rate,
    int32_t sample_size)
    : AudioSource(false, false, false, false, false),
      opened_(false),
      channels_(channels),
      sample_rate_(sample_rate),
      sample_size_(sample_size) {
    audio_constraint_.reset(new webrtc::FakeConstraints);
}

AudioSourceCustom::~AudioSourceCustom() {

}

BeeErrorCode AudioSourceCustom::open() {
    BeeErrorCode ret = kBeeErrorCode_Success;
    do {
        if (opened_) {
            break;
        }

        ret = AudioDeviceModuleWrapper::SetInputParam(channels_, sample_rate_, sample_size_);
        if (ret != kBeeErrorCode_Success) {
            break;
        }

        LuaWebrtcService::FactoryPtr peerconnection_factory = LuaWebrtcService::peer_connection_factory();
        if (peerconnection_factory == NULL) {
            ret = kBeeErrorCode_Null_Pointer;
            break;
    }

#ifndef WIN32
        audio_constraint_->AddMandatory(webrtc::MediaConstraintsInterface::kLevelControl, false);
        audio_constraint_->AddMandatory(webrtc::MediaConstraintsInterface::kGoogEchoCancellation, false);
#else
        audio_constraint_->AddMandatory(webrtc::MediaConstraintsInterface::kGoogEchoCancellation, false);
#endif
        audio_constraint_->AddMandatory(webrtc::MediaConstraintsInterface::kAutoGainControl, false);
        audio_constraint_->AddMandatory(webrtc::MediaConstraintsInterface::kHighpassFilter, false);
        audio_constraint_->AddMandatory(webrtc::MediaConstraintsInterface::kNoiseSuppression, false);

        audio_source_internal_->rtc_audio_track_source_ = peerconnection_factory->CreateAudioSource(audio_constraint_.get());
        opened_ = true;
    } while (0);
    return ret;
}

BeeErrorCode AudioSourceCustom::on_pcm_data(uint8_t* data, size_t samples_per_channel) {
    BeeErrorCode ret = kBeeErrorCode_Success;
    do {
        if (!AudioDeviceModuleWrapper::IsEnabled()) {
            ret = kBeeErrorCode_Invalid_State;
            break;
        }

        rtc::scoped_refptr<AudioDeviceModule> adm = LuaWebrtcService::get_custom_adm();
        AudioDeviceModuleWrapper *custom_adm = (AudioDeviceModuleWrapper*)adm.get();
        if (custom_adm == NULL) {
            ret = kBeeErrorCode_Null_Pointer;
            break;
        }

        custom_adm->OnPCMData(data, samples_per_channel);
    } while (0);
    return ret;
}

} // namespace bee
