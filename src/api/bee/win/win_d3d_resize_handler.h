#ifndef __WIN_D3D_RESIZE_HANDLER_H__
#define __WIN_D3D_RESIZE_HANDLER_H__

namespace bee {

class WinD3DResizeHandler {
public:
    virtual void handle_resize(size_t width, size_t height) = 0;
};

} // namespace bee

#endif // #ifndef __WIN_D3D_RESIZE_HANDLER_H__
