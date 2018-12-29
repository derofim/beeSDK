/**
 *  @file        bee_audio_source.h
 *  @brief       BeeSDK音频源声明文件.
 *  @author      HeZhen
 *  @copyright   sohu-inc
 *  @version     2.0.0.1
 */

#ifndef __BEE_AUDIO_SOURCE_H__
#define __BEE_AUDIO_SOURCE_H__

#import <Foundation/Foundation.h>
#include "bee/media/audio_source.h"

/// BeeSDK音频源类.
@interface BeeAudioSource : NSObject

/**
 *  @brief  BeeSDK音频源类构造函数.
 *  @param  levelControl        是否使能输入音量控制.
 *  @param  echoCancel          是否使能回声消除.
 *  @param  gainControl         是否使能增益控制.
 *  @param  highPassFilter      是否使能高通滤波器.
 *  @param  noiceSuppression    是否使能噪声抑制.
 *  @return BeeSDK音频源类对象.
 */
- (instancetype)initWithParam:(BOOL)levelControl
                   echoCancel:(BOOL)echoCancel
                  gainControl:(BOOL)gainControl
               highPassFilter:(BOOL)highPassFilter
             noiceSuppression:(BOOL)noiceSuppression;

/// 内部音频源.
@property(nonatomic, readonly) std::shared_ptr<bee::AudioSource> internalAudioSource;

/**
 *  @brief  打开音频源.
 *  @return 错误码.
 */
- (BeeErrorCode)open;

@end

#endif // #ifndef __BEE_AUDIO_SOURCE_H__
