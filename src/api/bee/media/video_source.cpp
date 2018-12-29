#include "bee/media/video_source.h"
#include "internal/video_source_internal.h"

namespace bee {

VideoSource::VideoSource(bee_int32_t width, bee_int32_t height, bee_int32_t fps, bool screencast)
    : video_source_internal_(new VideoSourceInternal),
      width_(width),
      height_(height), 
      fps_(fps),
      is_screencast_(screencast) {

}

VideoSource::~VideoSource() {

}
    
void VideoSource::on_frame(const void* data,
                           bee_int32_t data_len,
                           bee_int32_t width,
                           bee_int32_t height,
                           bee_int32_t rotation,
                           bee_int64_t timestamp_ns) {
    if (video_source_internal_ != NULL) {
        video_source_internal_->on_frame(data, data_len, width, height, rotation, timestamp_ns);
    }
}

} // namespace bee
