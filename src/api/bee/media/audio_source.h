/**
 *  @file        audio_source.h
 *  @brief       BeeSDK音频源基类声明文件.
 *  @author      HeZhen
 *  @copyright   sohu-inc
 *  @version     2.0.0.1
 */

#ifndef __AUDIO_SOURCE_H__
#define __AUDIO_SOURCE_H__

#include "bee/base/bee_define.h"
#include <memory>

namespace bee {

class AudioSourceInternal;

/// BeeSDK音频源基类.
class AudioSource {
public:
    /**
     *  @brief  AudioSource类构造函数.
     *  @param  level_control       是否使能输入音量控制.
     *  @param  echo_cancel         是否使能回声消除.
     *  @param  gain_control        是否使能自动增益控制.
     *  @param  high_pass_filter    是否使能高通滤波器.
     *  @param  noise_suppression   是否使能噪声抑制.
     */
    AudioSource(
        bool level_control,
        bool echo_cancel, 
        bool gain_control, 
        bool high_pass_filter, 
        bool noise_suppression);
    
    /**
     *  @brief  AudioSource类析构函数.
     */
    virtual ~AudioSource();

public:
    /**
     *  @brief  打开音频源.
     *  @return 错误码.
     */
    virtual BeeErrorCode open() = 0;

    /// 内部音频源.
    std::shared_ptr<AudioSourceInternal> audio_source_internal_;
};

} // namespace bee

#endif // #ifndef __AUDIO_SOURCE_H__
