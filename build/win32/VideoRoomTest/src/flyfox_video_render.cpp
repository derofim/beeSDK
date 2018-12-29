#include "stdafx.h"
#include "flyfox_video_render.h"

FlyFoxVideoRender::FlyFoxVideoRender(HWND hwnd, RECT rect)
    : hwnd_(hwnd),
      width_(rect.right - rect.left),
      height_(rect.bottom - rect.top){

    ff_video_render_handle_ = flyfox_video_render_filter_new();
    memset(&display_param_, 0, sizeof(display_param_));
    memset(&surface_param_, 0, sizeof(surface_param_));

    FF_RECT ff_rect;
    ff_rect.left = rect.left;
    ff_rect.top = rect.top;
    ff_rect.right = rect.right;
    ff_rect.bottom = rect.bottom;
    init(ff_rect);
}

FlyFoxVideoRender::~FlyFoxVideoRender() {
    if (ff_video_render_handle_ != NULL) {
        flyfox_video_render_filter_delete(ff_video_render_handle_);
    }
}

void FlyFoxVideoRender::video_render_callback(BeeVideoFrame *video_frame, void *opaque) {
    flyfox_video_render_filter_display_frame((FlyfoxVideoRenderHandle)opaque, (FF_BYTE*)video_frame->y_data, 0, video_frame->width, video_frame->height, video_frame->rotation);
}

void FlyFoxVideoRender::init(const FF_RECT &rect) {
    set_render_mode();
    update_video_ratio();
    update_display_param(rect);
    set_display_param();
    set_surface_param();
}

void FlyFoxVideoRender::set_render_mode() {
    render_mode_ = flyfox_video_render_filter_set_video_render_mode(ff_video_render_handle_, flyfox_media_player_video_render_vmr9);
}

void FlyFoxVideoRender::update_video_ratio() {
    flyfox_video_render_filter_update_video_ratio(ff_video_render_handle_, FF_PLAYER_VIDEO_DISPLAY_RATIO_DEFAULT);
}

void FlyFoxVideoRender::update_display_param(const FF_RECT &rect) {
    display_param_.pvDisplayUserData = hwnd_;
    display_param_.bFull = FF_FALSE;
    display_param_.recDisplay = rect;
    flyfox_video_render_filter_update_displayparam(ff_video_render_handle_, display_param_);
}

void FlyFoxVideoRender::set_display_param() {
    flyfox_video_render_filter_set_display_param(ff_video_render_handle_, display_param_);
}

void FlyFoxVideoRender::set_surface_param() {
    surface_param_.eVideoSurfaceFmt = FLYFOX_VIDEO_SURFACE_FMT_II420;
    surface_param_.nHeight = height_;
    surface_param_.nWidth = width_;
    flyfox_video_render_filter_set_surface_param(ff_video_render_handle_, surface_param_);
}
