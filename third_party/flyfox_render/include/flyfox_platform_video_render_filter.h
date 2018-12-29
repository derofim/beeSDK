/*************************************************

  Copyright (C), 2001-2010, Flyfox Net Info 鈥?Tech Co., Ltd

  File name:      flyfox_platform_video_render_filter.h

  Description:

  Others:

*************************************************/

#ifndef _FLYFOX_PLATFORM_VIDEO_RENDER_FILTER_H_
#define _FLYFOX_PLATFORM_VIDEO_RENDER_FILTER_H_

#include "flyfox_std.h"
#include "flyfox_player_media_common.h"
#include "flyfox_media_player_def.h"

#ifdef __cplusplus
extern "C" {
#endif
        
typedef struct tagVideoRenderDisPlayParams {
    void*                        pvDisplayUserData;
    PlayerVideoRotateDirection_e eRotateDirection;    
    FF_RECT                      recDisplay;
    FF_BOOL                      bFull;
}VideoRenderDisPlayParams_s;

typedef struct tagVideoRenderSurfaceParams {
    FlyfoxVideoSurfaceFmt_e      eVideoSurfaceFmt;
    FF_INT                       nWidth;
    FF_INT                       nHeight;
}VideoRenderSurfaceParams_s;

typedef FF_VOID* FlyfoxVideoRenderHandle;

FLYFOX_DLL_EXPORT_API 	FlyfoxVideoRenderHandle flyfox_video_render_filter_new();
FLYFOX_DLL_EXPORT_API   FF_VOID flyfox_video_render_filter_delete(FlyfoxVideoRenderHandle handle);
// FLYFOX_DLL_EXPORT_API   FF_VOID flyfox_video_render_filter_init(flyfox_player_render_cb e_render_cb);
// FLYFOX_DLL_EXPORT_API 	FF_VOID flyfox_video_render_filter_uninit(FF_VOID);
FLYFOX_DLL_EXPORT_API 	FF_VOID flyfox_video_render_filter_set_display_param(FlyfoxVideoRenderHandle handle, VideoRenderDisPlayParams_s a_sDisPlayParams);
FLYFOX_DLL_EXPORT_API 	FF_VOID flyfox_video_render_filter_set_surface_param(FlyfoxVideoRenderHandle handle, VideoRenderSurfaceParams_s a_SurfaceParams);
FLYFOX_DLL_EXPORT_API 	FF_VOID flyfox_video_render_filter_update_displayparam(FlyfoxVideoRenderHandle handle, VideoRenderDisPlayParams_s a_sDisPlayParams);
FLYFOX_DLL_EXPORT_API 	FF_VOID flyfox_video_render_filter_update_video_ratio(FlyfoxVideoRenderHandle handle, Flyfox_player_video_display_mode_e in_ratioDisplay);
FLYFOX_DLL_EXPORT_API 	FF_VOID flyfox_video_render_filter_display_frame(FlyfoxVideoRenderHandle handle, FF_BYTE* pDisplayFrame, FF_SIZE a_nFramelen, FF_INT in_nWidth, FF_INT in_nHeight, FF_UINT in_nRotation);
FLYFOX_DLL_EXPORT_API 	FF_VOID flyfox_video_render_draw_current_frame(FlyfoxVideoRenderHandle handle);
FLYFOX_DLL_EXPORT_API   FF_VOID flyfox_video_render_filter_set_3d_mode(FlyfoxVideoRenderHandle handle, Flyfox_player_3d_mode_t in_3dMode, FF_BYTE in_SetMask);
FLYFOX_DLL_EXPORT_API   FF_VOID flyfox_video_render_filter_set_video_luma(FlyfoxVideoRenderHandle handle, FF_INT in_nluma);
FLYFOX_DLL_EXPORT_API   FF_VOID flyfox_video_render_filter_set_video_cr(FlyfoxVideoRenderHandle handle, FF_INT in_nCr);
FLYFOX_DLL_EXPORT_API   FF_VOID flyfox_video_render_filter_set_video_cb(FlyfoxVideoRenderHandle handle, FF_INT in_nCb);
FLYFOX_DLL_EXPORT_API   FF_VOID flyfox_video_render_filter_close(FlyfoxVideoRenderHandle handle);
FLYFOX_DLL_EXPORT_API   FF_BOOL flyfox_video_render_filter_grab_a_video_frame(FlyfoxVideoRenderHandle handle, const char* in_pszFullPath);
FLYFOX_DLL_EXPORT_API   FF_BOOL flyfox_video_render_filter_grab_a_video_frame_buffer(FF_BYTE* pDisplayFrame, FF_SIZE a_nFramelen, FF_INT in_nWidth, FF_INT in_nHeight, const char* in_pszFullPath);
FLYFOX_DLL_EXPORT_API   flyfox_media_player_video_render_mode_e flyfox_video_render_filter_set_video_render_mode(FlyfoxVideoRenderHandle handle, flyfox_media_player_video_render_mode_e in_eRender_mode);
FLYFOX_DLL_EXPORT_API   FF_INT  flyfox_video_render_filter_get_display_time(FlyfoxVideoRenderHandle handle);
FLYFOX_DLL_EXPORT_API   FF_VOID flyfox_video_render_filter_get_display_window_rect(VideoRenderDisPlayParams_s a_sDisPlayParams, FF_RECT* prect_ff, FF_RECT* prectParent_ff, FF_RECT* prectTop_ff);
FLYFOX_DLL_EXPORT_API 	FF_VOID flyfox_video_render_filter_reset(FlyfoxVideoRenderHandle handle);
FLYFOX_DLL_EXPORT_API 	FF_VOID flyfox_text_render(FlyfoxVideoRenderHandle handle, VideoRenderDisPlayParams_s* a_psDisPlayParams, FF_UTF8* a_pstrMainFileName);
FLYFOX_DLL_EXPORT_API	FF_VOID	flyfox_lyrics_render(FlyfoxVideoRenderHandle handle, VideoRenderDisPlayParams_s* a_psDisPlayParams, LyricsRenderParams_s* a_psLyricParams, FF_BOOL bEnforceRepaint);
FLYFOX_DLL_EXPORT_API	FF_VOID	flyfox_text_lyrics_render_clear(FlyfoxVideoRenderHandle handle);
FLYFOX_DLL_EXPORT_API   FF_VOID flyfox_capture_screen_when_submit_log(VideoRenderDisPlayParams_s* a_psDisPlayParams, const FF_CHAR* a_pstrLogPath);

#ifdef __cplusplus
}
#endif

#endif
