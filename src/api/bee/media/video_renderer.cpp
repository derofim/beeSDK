#include "bee/media/video_renderer.h"
#include "internal/video_renderer_internal.h"

namespace bee {

VideoRenderer::VideoRenderer() : video_renderer_internal_(new VideoRendererInternal) {

}

VideoRenderer::~VideoRenderer() {

}

} // namespace bee
