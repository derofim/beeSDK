#include "bee/media/audio_source.h"
#include "internal/audio_source_internal.h"

namespace bee {

AudioSource::AudioSource(
    bool level_control, 
    bool echo_cancel, 
    bool gain_control, 
    bool high_pass_filter, 
    bool noise_suppression) : audio_source_internal_(new AudioSourceInternal) {

}

AudioSource::~AudioSource() {
}

} // namespace bee
