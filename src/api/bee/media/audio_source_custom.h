/**
 *  @file        audio_source_custom.h
 *  @brief       BeeSDK自定义音频源声明文件.
 *  @author      HeZhen
 *  @copyright   sohu-inc
 *  @version     2.0.0.1
 */

#ifndef __AUDIO_SOURCE_CUSTOM_H__
#define __AUDIO_SOURCE_CUSTOM_H__

#include "bee/media/audio_source.h"

namespace webrtc {
    class FakeConstraints;
}

namespace bee {

/// BeeSDK自定义音频源类.
class AudioSourceCustom : public AudioSource {
public:
    /**
     *  @brief  AudioSourceCustom类构造函数.
     *  @param  channels            通道数.
     *  @param  sample_rate         采样率.
     *  @param  sample_size         采样大小，单位字节.
     */
    AudioSourceCustom(
        int32_t channels, 
        int32_t sample_rate, 
        int32_t sample_size);
    
    /**
     *  @brief  AudioSourceDefault类析构函数.
     */
    virtual ~AudioSourceCustom();

public:
    /**
     *  @brief  打开音频源.
     *  @return 错误码.
     */
    virtual BeeErrorCode open();

    /**
    *  @brief  输入音频pcm数据.
    *  @param  data                 音频pcm数据指针.
    *  @param  samples_per_channel  每个采样包含通道数.
    *  @return 错误码.
    */
    BeeErrorCode on_pcm_data(uint8_t* data, size_t samples_per_channel);

protected:
    bool opened_;               //!< 是否已经打开.
    int32_t channels_;          //!< 通道数.
    int32_t sample_rate_;       //!< 采样率.
    int32_t sample_size_;       //!< 采样大小，单位字节.
    std::shared_ptr<webrtc::FakeConstraints> audio_constraint_;  //!< 内部音频参数集.
};

} // namespace bee

#endif // #ifndef __AUDIO_SOURCE_CUSTOM_H__
