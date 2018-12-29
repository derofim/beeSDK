/**
 *  @file        bee_sdk.h
 *  @brief       BeeSDK默认渲染器声明文件.
 *  @author      HeZhen
 *  @copyright   sohu-inc
 *  @version     2.0.0.1
 */

#ifndef __BEE_VIDEO_RENDERER_H__
#define __BEE_VIDEO_RENDERER_H__

#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>
#include "bee/media/video_frame.h"

@class BeeVideoRenderer;

namespace bee {
    class BeeVideoRendererOcAdapter;
}

/// BeeSDK渲染器通知协议.
@protocol BeeVideoRendererDelegate

/**
 *  @brief  尺寸修改通知.
 *  @param  videoRenderer   当前渲染器对象.
 *  @param  size            修改的尺寸.
 *  @note   只在EAGL模式下调用.
 */
- (void)didChangeVideoSize:(BeeVideoRenderer*)videoRenderer size:(CGSize)size;

@end

/// BeeSDK渲染器类.
@interface BeeVideoRenderer : NSObject

/// 渲染器实际用于显示的view，在ARM64下使用METAL渲染，其他架构使用EAGL模式.
@property(nonatomic, readonly) UIView *view;

/// 渲染器内部数据源.
@property(nonatomic, readonly) std::shared_ptr<bee::BeeVideoRendererOcAdapter> internalVideoRenderer;

/// 渲染器委托，用于通知事件.
@property(nonatomic, weak) id<BeeVideoRendererDelegate> delegate;

/// 是否镜像.
@property(nonatomic, assign) BOOL mirror;

/**
 *  @brief  渲染一帧视频帧.
 *  @param  frame   视频帧.
 *  @note   自定义渲染器时必须重写该方法.
 */
- (void)onFrame:(const bee::VideoFrame &)frame;

@end

#endif // #ifndef __BEE_VIDEO_RENDERER_H__
