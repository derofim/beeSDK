/**
 *  @file        video_frame.h
 *  @brief       BeeSDK视频帧类声明文件.
 *  @author      HeZhen
 *  @copyright   sohu-inc
 *  @version     2.0.0.1
 */

#ifndef __VIDEO_FRAME_H__
#define __VIDEO_FRAME_H__

#include "bee/base/bee_define.h"
#include <memory>

namespace bee {

class VideoFrameInternal;

/// BeeSDK视频帧类.
class VideoFrame {
public:
    /// VideoFrame构造函数.
    VideoFrame();
    /// VideoFrame析构函数.
    ~VideoFrame();

public:
    /// 内部视频帧.
    std::shared_ptr<VideoFrameInternal> video_frame_internal_;
    /// Y分量指针.
    bee_uint8_t *y_data_ = NULL;
    /// U分量指针.
    bee_uint8_t *u_data_ = NULL;
    /// V分量指针.
    bee_uint8_t *v_data_ = NULL;
    /// Y分量内存对齐宽度.
    bee_int32_t y_stride_ = 0;
    /// U分量内存对齐宽度.
    bee_int32_t u_stride_ = 0;
    /// V分量内存对齐宽度.
    bee_int32_t v_stride_ = 0;
    /// 视频宽.
    bee_int32_t width_ = 0;
    /// 视频高.
    bee_int32_t height_ = 0;
    /// 旋转角度，0、90、 180、 270、 360.
    bee_int32_t rotation_ = 0;
    /// 时间戳，单位us.
    bee_int64_t timestamp_ = 0;
};

} // namespace bee

#endif // #ifndef __VIDEO_FRAME_H__
