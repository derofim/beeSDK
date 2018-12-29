/**
 *  @file        audio_source_default.h
 *  @brief       BeeSDK默认音频源声明文件.
 *  @author      HeZhen
 *  @copyright   sohu-inc
 *  @version     2.0.0.1
 */

#ifndef __AUDIO_SOURCE_DEFAULT_H__
#define __AUDIO_SOURCE_DEFAULT_H__

#include "bee/media/audio_source.h"

namespace webrtc {
class FakeConstraints;
}

namespace bee {

/// BeeSDK默认音频源类.
class AudioSourceDefault : public AudioSource {
public:
    /**
     *  @brief  AudioSourceDefault类构造函数.
     *  @param  level_control       是否使能输入音量控制.
     *  @param  echo_cancel         是否使能回声消除.
     *  @param  gain_control        是否使能自动增益控制.
     *  @param  high_pass_filter    是否使能高通滤波器.
     *  @param  noise_suppression   是否使能噪声抑制.
     */
    AudioSourceDefault(
        bool level_control = true,
        bool echo_cancel = true,
        bool gain_control = true,
        bool high_pass_filter = true,
        bool noise_suppression = true);
    
    /**
     *  @brief  AudioSourceDefault类析构函数.
     */
    virtual ~AudioSourceDefault();

public:
    /**
     *  @brief  打开音频源.
     *  @return 错误码.
     */
    virtual BeeErrorCode open();

protected:
    bool opened_;               //!< 是否已经打开.
    bool level_control_;        //!< 是否使能输入音量控制.
    bool echo_cancel_;          //!< 是否使能回声消除.
    bool gain_control_;         //!< 是否使能自动增益控制.
    bool high_pass_filter_;     //!< 是否使能高通滤波器.
    bool noise_suppression_;    //!< 是否使能噪声抑制.
    std::shared_ptr<webrtc::FakeConstraints> audio_constraint_;  //!< 内部音频参数集.
};

} // namespace bee

#endif // #ifndef __AUDIO_SOURCE_DEFAULT_H__
