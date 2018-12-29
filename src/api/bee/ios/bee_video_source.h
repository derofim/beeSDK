/**
 *  @file        bee_video_source.h
 *  @brief       BeeSDK视频源声明文件.
 *  @author      HeZhen
 *  @copyright   sohu-inc
 *  @version     2.0.0.1
 */

#ifndef __BEE_VIDEO_SOURCE_H__
#define __BEE_VIDEO_SOURCE_H__

#import <Foundation/Foundation.h>
#import <AVFoundation/AVFoundation.h>
#import <UIKit/UIDevice.h>

#include "bee/base/bee_define.h"
#include <memory>

namespace bee {
    class BeeVideoSourceOcAdapter;
}

/// BeeSDK视频源类.
@interface BeeVideoSource : NSObject

/// 视频宽.
@property(nonatomic, assign) bee_int32_t width;

/// 视频高.
@property(nonatomic, assign) bee_int32_t height;

/// 视频帧率.
@property(nonatomic, assign) bee_int32_t fps;

/// 是否抓屏.
@property(nonatomic, assign) BOOL isScreencast;

/// 视频源内部数据输出.
@property(nonatomic, readonly) std::shared_ptr<bee::BeeVideoSourceOcAdapter> internalVideoSource;

/**
 *  @brief  视频源类构造函数.
 *  @param  isScreencast    是否抓屏.
 *  @param  width           视频源输出图像宽度.
 *  @param  height          视频源输出图像高度.
 *  @param  fps             视频源输出帧率.
 *  @return 视频源类对象.
 *  @note   必须在派生类构造函数调用.
 */
- (instancetype)initWithParam:(BOOL)isScreencast
                        width:(bee_int32_t)width
                       height:(bee_int32_t)height
                          fps:(bee_int32_t)fps;
/**
 *  @brief  打开视频源.
 *  @return 错误码.
 *  @note   该方法必须被调用.
 */
- (BeeErrorCode)open;

/**
 *  @brief  视频源输出一帧.
 *  @param  pixelBuffer     数据缓冲.
 *  @param  rotation        图像旋转角度.
 *  @param  timestamp       时间戳.
 *  @note   必须调用该方法输出视频帧.
 */
- (void)onFrame:(CVPixelBufferRef)pixelBuffer rotation:(UIDeviceOrientation)rotation timestamp:(int64_t)timestamp;

@end

#endif // #ifndef __BEE_VIDEO_SOURCE_H__
