#include "bee/media/audio_source_default.h"
#include "internal/audio_source_internal.h"
#include "lua/module/lua_webrtc_service.h"
#include "webrtc/api/test/fakeconstraints.h"

namespace bee {

AudioSourceDefault::AudioSourceDefault(
    bool level_control,
    bool echo_cancel,
    bool gain_control,
    bool high_pass_filter,
    bool noise_suppression) 
    : AudioSource(
        level_control,
        echo_cancel,
        gain_control,
        high_pass_filter,
        noise_suppression), 
      opened_(false),
      level_control_(true),
      echo_cancel_(true),
      gain_control_(true),
      high_pass_filter_(true),
      noise_suppression_(true) {
    audio_constraint_.reset(new webrtc::FakeConstraints);
}

AudioSourceDefault::~AudioSourceDefault() {

}

BeeErrorCode AudioSourceDefault::open() {
    BeeErrorCode ret = kBeeErrorCode_Success;
    do {
        if (opened_) {
            break;
        }

        LuaWebrtcService::FactoryPtr peerconnection_factory = LuaWebrtcService::peer_connection_factory();
        if (peerconnection_factory == NULL) {
            ret = kBeeErrorCode_Null_Pointer;
            break;
        }

#ifndef WIN32
        audio_constraint_->AddMandatory(webrtc::MediaConstraintsInterface::kLevelControl, level_control_);
        audio_constraint_->AddMandatory(webrtc::MediaConstraintsInterface::kGoogEchoCancellation, echo_cancel_);
#else
        audio_constraint_->AddMandatory(webrtc::MediaConstraintsInterface::kGoogEchoCancellation, false);
#endif
        audio_constraint_->AddMandatory(webrtc::MediaConstraintsInterface::kAutoGainControl, gain_control_);
        audio_constraint_->AddMandatory(webrtc::MediaConstraintsInterface::kHighpassFilter, high_pass_filter_);
        audio_constraint_->AddMandatory(webrtc::MediaConstraintsInterface::kNoiseSuppression, noise_suppression_);

        audio_source_internal_->rtc_audio_track_source_ = peerconnection_factory->CreateAudioSource(audio_constraint_.get());
        opened_ = true;
    } while (0);
    return ret;
}

} // namespace bee
