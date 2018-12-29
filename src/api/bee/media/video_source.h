/**
 *  @file        video_source.h
 *  @brief       BeeSDK视频源基类声明文件.
 *  @author      HeZhen
 *  @copyright   sohu-inc
 *  @version     2.0.0.1
 */

#ifndef __VIDEO_SOURCE_H__
#define __VIDEO_SOURCE_H__

#include "bee/base/bee_define.h"
#include <memory>

namespace bee {

class VideoSourceInternal;

///BeeSDK视频源基类.
class VideoSource {
public:
    /**
     *  @brief  VideoSource类构造函数.
     *  @param  width       视频宽.
     *  @param  height      视频高.
     *  @param  fps         视频帧率.
     *  @param  screencast  是否抓屏.
     */
    VideoSource(bee_int32_t width, bee_int32_t height, bee_int32_t fps, bool screencast = false);
    
    /**
     *  @brief  VideoSource类析构函数.
     */
    virtual ~VideoSource();

public:
    /**
     *  @brief  打开视频源.
     *  @return 错误码.
     */
    virtual BeeErrorCode open() = 0;
    
    /**
     *  @brief  推送一帧.
     *  @param  data            视频帧数据指针.
     *  @param  data_len        视频帧数据长度.
     *  @param  width           视频宽度.
     *  @param  height          视频高度.
     *  @param  rotation        视频旋转角度.
     *  @param  timestamp_ns    时间戳，单位ns.
     *  @note   视频帧将在底层进行编码、发送.
     */
    virtual void on_frame(const void* data,
                          bee_int32_t data_len,
                          bee_int32_t width,
                          bee_int32_t height,
                          bee_int32_t rotation,
                          bee_int64_t timestamp_ns);

    /// 内部视频源.
    std::shared_ptr<VideoSourceInternal> video_source_internal_;

protected:
    /// 视频宽度.
    bee_int32_t width_;
    /// 视频高度.
    bee_int32_t height_;
    /// 视频帧率.
    bee_int32_t fps_;
    /// 是否抓屏
    bool is_screencast_;
};

} // namespace bee

#endif // #ifndef __VIDEO_SOURCE_H__
