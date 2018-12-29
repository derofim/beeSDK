#include "bee/win/win_video_renderer_d3d_imp.h"
#include "common_video/libyuv/include/webrtc_libyuv.h"

namespace bee {

////////////////////////////////////D3D Define//////////////////////////////////////
#define D3DFVF_CUSTOMVERTEX (D3DFVF_XYZ|D3DFVF_TEX1)
const char kD3DClassName[] = "d3d_renderer";
struct D3dCustomVertex {
    float x, y, z;
    float u, v;
};

///////////////////////////////////WinVideoRendererD3DImp///////////////////////////////////////
WinVideoRendererD3DImp::WinVideoRendererD3DImp(HWND hWnd, size_t width, size_t height, std::shared_ptr<WinD3DResizeHandler> resize_handler)
    : hwnd_(hWnd),
      width_(width),
      height_(height),
      resize_handler_(resize_handler) {
    
}

WinVideoRendererD3DImp::~WinVideoRendererD3DImp() {
    close();
}

WinVideoRendererD3DImp::Ptr WinVideoRendererD3DImp::Create(HWND hWnd, size_t width, size_t height, std::shared_ptr<WinD3DResizeHandler> resize_handler) {
    WinVideoRendererD3DImp::Ptr renderer(new WinVideoRendererD3DImp(hWnd, width, height, resize_handler));
    if (renderer->open()) {
        return renderer;
    } else {
        return WinVideoRendererD3DImp::Ptr();
    }
}

bool WinVideoRendererD3DImp::open() {
    bool ret = true;
    do {
        if (hwnd_ == NULL) {
            ret = false;
            break;
        }

        d3d_ = Direct3DCreate9(D3D_SDK_VERSION);
        if (d3d_ == NULL) {
            ret = false;
            break;
        }

        D3DPRESENT_PARAMETERS d3d_params = {};

        d3d_params.Windowed = TRUE;
        d3d_params.SwapEffect = D3DSWAPEFFECT_COPY;

        IDirect3DDevice9* d3d_device;
        if (d3d_->CreateDevice(D3DADAPTER_DEFAULT,
            D3DDEVTYPE_HAL,
            hwnd_,
            D3DCREATE_SOFTWARE_VERTEXPROCESSING,
            &d3d_params,
            &d3d_device) != D3D_OK) {
            ret = false;
            break;
        }

        d3d_device_ = d3d_device;
        d3d_device->Release();

        IDirect3DVertexBuffer9* vertex_buffer;
        const int kRectVertices = 4;
        if (d3d_device_->CreateVertexBuffer(kRectVertices * sizeof(D3dCustomVertex),
            0,
            D3DFVF_CUSTOMVERTEX,
            D3DPOOL_MANAGED,
            &vertex_buffer,
            NULL) != D3D_OK) {
            ret = false;
            break;
        }
        vertex_buffer_ = vertex_buffer;
        vertex_buffer->Release();

        d3d_device_->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
        d3d_device_->SetRenderState(D3DRS_LIGHTING, FALSE);
        resize(width_, height_);

        ShowWindow(hwnd_, SW_SHOWNOACTIVATE);
        d3d_device_->Present(NULL, NULL, NULL, NULL);
    } while (0);

    if (!ret) {
        close();
    }
    return ret;
}

void WinVideoRendererD3DImp::close() {
    texture_ = NULL;
    vertex_buffer_ = NULL;
    d3d_device_ = NULL;
    d3d_ = NULL;

    if (hwnd_ != NULL) {
        DestroyWindow(hwnd_);
        RTC_DCHECK(!IsWindow(hwnd_));
        hwnd_ = NULL;
    }
}

void WinVideoRendererD3DImp::resize(size_t width, size_t height) {
    width_ = width;
    height_ = height;
    IDirect3DTexture9 *texture = NULL;

    d3d_device_->CreateTexture(static_cast<UINT>(width_),
        static_cast<UINT>(height_),
        1,
        0,
        D3DFMT_A8R8G8B8,
        D3DPOOL_MANAGED,
        &texture,
        NULL);
    texture_ = texture;
    texture->Release();

    // Vertices for the video frame to be rendered to.
    static const D3dCustomVertex rect[] = {
      {-1.0f, -1.0f, 0.0f, 0.0f, 1.0f},
      {-1.0f, 1.0f, 0.0f, 0.0f, 0.0f},
      {1.0f, -1.0f, 0.0f, 1.0f, 1.0f},
      {1.0f, 1.0f, 0.0f, 1.0f, 0.0f},
    };

    void *buf_data = NULL;
    if (vertex_buffer_->Lock(0, 0, &buf_data, 0) != D3D_OK) {
        return;
    }

    memcpy(buf_data, &rect, sizeof(rect));
    vertex_buffer_->Unlock();

    std::shared_ptr<WinD3DResizeHandler> resize_handler = resize_handler_.lock();
    if (resize_handler != NULL) {
        resize_handler->handle_resize(width_, height_);
    }
}

void WinVideoRendererD3DImp::OnFrame(const webrtc::VideoFrame& frame) {
    if (static_cast<size_t>(frame.width()) != width_ ||
        static_cast<size_t>(frame.height()) != height_) {
        resize(static_cast<size_t>(frame.width()), static_cast<size_t>(frame.height()));
    }

    D3DLOCKED_RECT lock_rect;
    if (texture_->LockRect(0, &lock_rect, NULL, 0) != D3D_OK) {
        return;
    }

    ConvertFromI420(frame, webrtc::VideoType::kARGB, 0, static_cast<uint8_t*>(lock_rect.pBits));
    texture_->UnlockRect(0);

    d3d_device_->BeginScene();
    d3d_device_->SetFVF(D3DFVF_CUSTOMVERTEX);
    d3d_device_->SetStreamSource(0, vertex_buffer_, 0, sizeof(D3dCustomVertex));
    d3d_device_->SetTexture(0, texture_);
    d3d_device_->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);
    d3d_device_->EndScene();

    d3d_device_->Present(NULL, NULL, NULL, NULL);
}

} // namespace bee
