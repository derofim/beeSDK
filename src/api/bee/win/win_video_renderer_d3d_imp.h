#ifndef __WIN_VIDEO_RENDERER_D3D_IMP_H__
#define __WIN_VIDEO_RENDERER_D3D_IMP_H__

#include "webrtc/api/video/video_frame.h"
#include "webrtc/api/videosinkinterface.h"
#include "bee/win/win_d3d_resize_handler.h"

#include <Windows.h>
#include <d3d9.h>
#pragma comment(lib, "d3d9.lib")

namespace bee {

///////////////////////////////////WinVideoRendererD3DImp///////////////////////////////////////
class WinVideoRendererD3DImp : public rtc::VideoSinkInterface<webrtc::VideoFrame> {
public:
    typedef std::shared_ptr<WinVideoRendererD3DImp> Ptr;
    static Ptr Create(HWND hWnd, size_t width, size_t height, std::shared_ptr<WinD3DResizeHandler> resize_handler);
    virtual ~WinVideoRendererD3DImp();

public:
    bool open();
    void close();
    void resize(size_t width, size_t height);    
    void OnFrame(const webrtc::VideoFrame& frame) override;

private:
    WinVideoRendererD3DImp(HWND hWnd, size_t width, size_t height, std::shared_ptr<WinD3DResizeHandler> resize_handler);

public:
    HWND hwnd_;    
    size_t width_ = 0;
    size_t height_ = 0;
    std::weak_ptr<WinD3DResizeHandler> resize_handler_;
  
    rtc::scoped_refptr<IDirect3D9> d3d_;
    rtc::scoped_refptr<IDirect3DDevice9> d3d_device_;
    rtc::scoped_refptr<IDirect3DTexture9> texture_;
    rtc::scoped_refptr<IDirect3DVertexBuffer9> vertex_buffer_;
};

} // namespace bee

#endif // #ifndef __WIN_VIDEO_RENDERER_D3D_IMP_H__
