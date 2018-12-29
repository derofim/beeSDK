#include "bee/win/win_video_renderer_d3d.h"
#include "bee/win/win_video_renderer_d3d_imp.h"
#include "bee/media/video_frame.h"
#include "internal/video_frame_internal.h"
#include "internal/video_renderer_internal.h"

namespace bee {

WinVideoRendererD3D::WinVideoRendererD3D() : opened(false) {
    
}

WinVideoRendererD3D::~WinVideoRendererD3D() {

}

bool WinVideoRendererD3D::open(HWND hWnd, size_t width, size_t height, std::shared_ptr<WinD3DResizeHandler> resize_handler) {
    if (opened) {
        return true;
    }
    video_renderer_internal_->rtc_video_renderer_ = WinVideoRendererD3DImp::Create(hWnd, width, height, resize_handler);
    if (video_renderer_internal_->rtc_video_renderer_ == NULL) {
        return false;
    } else {
        opened = true;
        return true;
    }
}

void WinVideoRendererD3D::close() {
    if (video_renderer_internal_->rtc_video_renderer_ != NULL) {
        video_renderer_internal_->rtc_video_renderer_.reset();
    }
}

void WinVideoRendererD3D::on_frame(const VideoFrame &frame) {
    if (opened) {
        video_renderer_internal_->rtc_video_renderer_->OnFrame(frame.video_frame_internal_->rtc_video_frame_);
    }
}

} // namespace bee
