/**
 *  @file        video_renderer.h
 *  @brief       BeeSDK视频渲染器基类声明文件.
 *  @author      HeZhen
 *  @copyright   sohu-inc
 *  @version     2.0.0.1
 */

#ifndef __VIDEO_RENDERER_H__
#define __VIDEO_RENDERER_H__

#include "bee/media/video_frame.h"

namespace bee {

class VideoRendererInternal;

/// BeeSDK视频渲染器基类.
class VideoRenderer {
public:
    /// VideoRenderer构造函数.
    VideoRenderer();
    /// VideoRenderer析构函数.
    virtual ~VideoRenderer();

public:
    /**
     *  @brief  渲染一帧，派生类必须实现该方法.
     *  @param  frame       视频帧.
     *  @see    VideoFrame.
     *  @note   VideoFrame已经是经过解码的图像.
     */
    virtual void on_frame(const VideoFrame &frame) = 0;

public:
    /// 内部渲染器.
    std::shared_ptr<VideoRendererInternal> video_renderer_internal_;
};

} // namespace bee

#endif // #ifndef __VIDEO_RENDERER_H__
