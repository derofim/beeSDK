/**
 *  @file        bee_video_source_camera.h
 *  @brief       BeeSDK摄像头视频源声明文件.
 *  @author      HeZhen
 *  @copyright   sohu-inc
 *  @version     2.0.0.1
 */

#ifndef __BEE_VIDEO_SOURCE_CAMERA_H__
#define __BEE_VIDEO_SOURCE_CAMERA_H__

#import "bee/ios/bee_video_source.h"

/// BeeSDK摄像头视频源类.
@interface BeeVideoSourceCamera : BeeVideoSource
    
/**
 *  @brief  摄像头视频源类构造函数.
 *  @param  fps        摄像头输出帧率.
 *  @param  width      摄像头输出图像宽度.
 *  @param  height     摄像头输出图像高度.
 *  @param  fixRes     是否固定分辨率
 *  @return 摄像头视频源类对象.
 */
- (instancetype)initWithParam:(bee_int32_t)fps
                        width:(bee_int32_t)width
                       height:(bee_int32_t)height
                       fixRes:(BOOL)fixRes;

/**
 *  @brief  打开摄像头视频源.
 *  @return 错误码.
 */
- (BeeErrorCode)open;

/**
 *  @brief  关闭摄像头视频源.
 */
- (void)close;

/**
 *  @brief  切换前后置摄像头.
 */
- (void)switchCamera;

/**
 *  @brief  检查是否前置摄像头.
 *  @return 是否前置摄像头.
 */
- (BOOL)isFrontCamera;

@end

#endif // #ifndef __BEE_VIDEO_SOURCE_CAMERA_H__
