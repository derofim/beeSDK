#ifndef __AUDIO_SOURCE_INTERNAL_H__
#define __AUDIO_SOURCE_INTERNAL_H__

#include "bee/base/bee_define.h"
#include "webrtc/api/mediastreaminterface.h"

namespace bee {

class AudioSourceInternal {
public:
    AudioSourceInternal();
    virtual ~AudioSourceInternal();

public:
    rtc::scoped_refptr<webrtc::AudioSourceInterface> rtc_audio_track_source_;
};

} // namespace bee

#endif // #ifndef __AUDIO_SOURCE_INTERNAL_H__
