#ifndef __WIN_VIDEO_RENDERER_D3D_H__
#define __WIN_VIDEO_RENDERER_D3D_H__

#include "bee/media/video_renderer.h"
#include "bee/win/win_d3d_resize_handler.h"
#include <atomic>
#include <Windows.h>

namespace bee {

////////////////////////////////////WinVideoRendererD3D//////////////////////////////////////
class WinVideoRendererD3D : public VideoRenderer {
public:
    WinVideoRendererD3D();
    virtual ~WinVideoRendererD3D();

public:
    bool open(HWND hWnd, size_t width, size_t height, std::shared_ptr<WinD3DResizeHandler> resize_handler);
    void close();
    virtual void on_frame(const VideoFrame &frame);

private:
    std::atomic_bool opened;
};

} // namespace bee

#endif // #ifndef __WIN_VIDEO_RENDERER_D3D_H__
