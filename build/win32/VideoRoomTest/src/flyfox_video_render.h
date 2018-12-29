#ifndef __FLYFOX_VIDEO_RENDER_H__
#define __FLYFOX_VIDEO_RENDER_H__

#include "flyfox_platform_video_render_filter.h"
#include "flyfox_media_player_def.h"
#include "bee/bee_api.h"
#include "comLib/SafeQueue.h"
#include <future>
#include <windows.h>

class FlyFoxVideoRender {
public:
    FlyFoxVideoRender(HWND hwnd, RECT rect);
    ~FlyFoxVideoRender();

public:
    FlyfoxVideoRenderHandle get_video_render_handler() { return ff_video_render_handle_; }
    static void BEE_CALLBACK video_render_callback(BeeVideoFrame *video_frame, void *opaque);

protected:
    void init(const FF_RECT &rect);
    void set_render_mode();
    void update_video_ratio();
    void update_display_param(const FF_RECT &rect);
    void set_display_param();
    void set_surface_param();

protected:
    HWND hwnd_ = NULL;
    size_t width_ = 0;
    size_t height_ = 0;
    FlyfoxVideoRenderHandle ff_video_render_handle_ = NULL;
    VideoRenderDisPlayParams_s display_param_;
    VideoRenderSurfaceParams_s surface_param_;
    comLib::SafeQueue<int> frame_queue_;
    bool rendering_ = false;
    std::promise<int> thread_exit_promise_;
    flyfox_media_player_video_render_mode_e render_mode_ = flyfox_media_player_video_render_none;
};

#endif
