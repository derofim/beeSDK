/*************************************************

  Copyright (C), 2001-2010, sohu Net Info - Tech Co., Ltd

  File name:      flyfox_platform_audio_render_filter.h

  Description:

  Others:

*************************************************/

#ifndef _FLYFOX_AUDIO_RENDER_FILTER_H_
#define _FLYFOX_AUDIO_RENDER_FILTER_H_

#include "flyfox_std.h"
#include "flyfox_player_media_common.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct tagFlyfoxAudioRenderAudioParms {
    void*                           pvDisplayUserData;
    FlyfoxAudioSampleFmt_e          eAudioSampleFmt;
    FlyfoxAudioChannels_e           iChannelCount;
    FF_UINT                         uiSampleRate;
    FF_UINT                         uiBitCount;
}FlyfoxAudioRenderAudioParms_s;

typedef FF_VOID* FlyfoxAudioRenderHandle;

typedef FF_BOOL (*flyfox_audio_render_filter_callback_cb)(FF_VOID *pParam, FF_VOID* pAudioBuffer, FF_INT* nBufferSize);

FLYFOX_DLL_EXPORT_API   FlyfoxAudioRenderHandle flyfox_audio_render_filter_new();
FLYFOX_DLL_EXPORT_API   FF_VOID flyfox_audio_render_filter_delete(FlyfoxAudioRenderHandle handle);
//FLYFOX_DLL_EXPORT_API FF_VOID  flyfox_audio_render_filter_init();
//FLYFOX_DLL_EXPORT_API FF_VOID  flyfox_audio_render_filter_uninit();
FLYFOX_DLL_EXPORT_API 	FF_BOOL  flyfox_audio_render_filter_set_param(FlyfoxAudioRenderHandle handle, FlyfoxAudioRenderAudioParms_s a_AudioRenderAudioPams, flyfox_audio_render_filter_callback_cb in_GetAudioCB, FF_VOID *in_GetAudioCBParam);
FLYFOX_DLL_EXPORT_API 	FF_VOID  flyfox_audio_render_filter_set_volume(FlyfoxAudioRenderHandle handle, FF_SIZE a_nVolume);
FLYFOX_DLL_EXPORT_API 	FF_VOID  flyfox_audio_render_filter_get_volume(FlyfoxAudioRenderHandle handle, FF_SIZE* a_nVolume);

	
FLYFOX_DLL_EXPORT_API 	FF_VOID  flyfox_audio_render_filter_play_audio(FlyfoxAudioRenderHandle handle, FF_CONST FF_BYTE* a_pPlayAudio, FF_SIZE a_nAudioLen, void* a_pUserData);
FLYFOX_DLL_EXPORT_API 	FF_VOID  flyfox_audio_render_filter_stop(FlyfoxAudioRenderHandle handle);
FLYFOX_DLL_EXPORT_API 	FF_VOID  flyfox_audio_render_filter_pause(FlyfoxAudioRenderHandle handle);
FLYFOX_DLL_EXPORT_API 	FF_VOID  flyfox_audio_render_filter_resume(FlyfoxAudioRenderHandle handle);
FLYFOX_DLL_EXPORT_API 	FF_VOID  flyfox_audio_render_filter_reset(FlyfoxAudioRenderHandle handle);
FLYFOX_DLL_EXPORT_API 	FF_VOID  flyfox_audio_render_filter_close(FlyfoxAudioRenderHandle handle);
FLYFOX_DLL_EXPORT_API 	FF_VOID  flyfox_audio_render_filter_flush(FlyfoxAudioRenderHandle handle);

FLYFOX_DLL_EXPORT_API 	FF_INT   flyfox_audio_render_filter_get_audio_delay(FlyfoxAudioRenderHandle handle);
FLYFOX_DLL_EXPORT_API 	FF_BOOL  flyfox_audio_render_filter_get_audio_sync_clock(FlyfoxAudioRenderHandle handle);
FLYFOX_DLL_EXPORT_API 	FF_INT64 flyfox_audio_render_filter_get_audio_play_time(FlyfoxAudioRenderHandle handle);

#ifdef __cplusplus
}
#endif

#endif
