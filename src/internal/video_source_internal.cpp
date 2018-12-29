#include "internal/video_source_internal.h"
#include "webrtc/media/base/adaptedvideotracksource.h"

namespace bee {

VideoSourceInternal::VideoSourceInternal() {

}

VideoSourceInternal::~VideoSourceInternal() {

}

void VideoSourceInternal::on_frame(
    const void* data,
    bee_int32_t data_len,
    bee_int32_t width,
    bee_int32_t height,
    bee_int32_t rotation,
    bee_int64_t timestamp_ns) {

}

} // namespace bee
