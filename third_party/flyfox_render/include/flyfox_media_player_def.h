#ifndef _FLYFOX_MEDIA_PLAYER_DEF_H_
#define _FLYFOX_MEDIA_PLAYER_DEF_H_

#include "flyfox_std.h"

#define FLYFOX_PLAYER_VOLUME_LEVEL_MAX           1500
#define FLYFOX_PLAYER_PRELOAD_NEXT_PIECE_TIME    30000  //30s

#ifdef __cplusplus
extern "C" {
#endif

typedef FF_VOID (*flyfox_media_player_notify_to_ui)(FF_INT a_nNotifyMsg, FF_VOID* wParam,FF_VOID* lParam);
typedef FF_VOID (*flyfox_media_player_open_cb)(FF_BOOL in_bSuceess);

typedef enum flyfox_media_player_status {
	flyfox_media_player_none = 0,
	flyfox_media_player_load,
	flyfox_media_player_play,
	flyfox_media_player_pause,
	flyfox_media_player_ready,
	flyfox_media_player_endof
}flyfox_media_player_status_e;

typedef enum flyfox_media_player_video_render_mode {
	flyfox_media_player_video_render_none = 0,
	flyfox_media_player_video_render_vmr7,
	flyfox_media_player_video_render_vmr9,
	flyfox_media_player_video_render_evr,
	flyfox_media_player_video_render_ddraw
}flyfox_media_player_video_render_mode_e;

typedef enum tagFlyfox_player_ui_message {
	FF_PLAYER_UI_MESSAGE_NONE = 0,
	FF_PLAYER_UI_MESSAGE_NOT_SUPPORTED_FORMAT,
	FF_PLAYER_UI_MESSAGE_OPEN_MEDIA_SUCCESS,
	FF_PLAYER_UI_MESSAGE_OPEN_MEDIA_FAILED,
	FF_PLAYER_UI_MESSAGE_SEEK_MEDIA_SUCCESS,
	FF_PLAYER_UI_MESSAGE_SEEK_MEDIA_FAILED,
	FF_PLAYER_UI_MESSAGE_BUFFERING_PERCENT,
	FF_PLAYER_UI_MESSAGE_GET_MEDIA_INFO_SUCCESS,
	FF_PLAYER_UI_MESSAGE_READY_TO_PLAY,
	FF_PLAYER_UI_MESSAGE_PLAY_ENDOF,
	FF_PLAYER_UI_MESSAGE_MEDIA_TOTAL_DURATION,
	FF_PLAYER_UI_MESSAGE_MEDIA_CURRENT_POS,
	FF_PLAYER_UI_MESSAGE_MEDIA_PRELOAD_LEN,
	FF_PLAYER_UI_MESSAGE_MEDIA_TOTAL_FILE_LEN,
	FF_PLAYER_UI_MESSAGE_CONNECT_NETWORK_FAILED,
	FF_PLAYER_UI_MESSAGE_REQUEST_HOT_VRS_FAILED,
	FF_PLAYER_UI_MESSAGE_NETWORK_MEDIA_PARSER_FAILED,
	FF_PLAYER_UI_MESSAGE_NETWORK_CLOSED,
	FF_PLAYER_UI_MESSAGE_NETWORK_PRELOAD_OVER,
	FF_PLAYER_UI_MESSAGE_PAUSE_PLAYING_FOR_BUFFERING,
	FF_PLAYER_UI_MESSAGE_START_PLAYING_FOR_BUFFERING,
	FF_PLAYER_UI_MESSAGE_PLAYING_PIECE_INDEX,
	FF_PLAYER_UI_MESSAGE_DDSHOW_NOT_SUPPORT,
	FF_PLAYER_UI_MESSAGE_NEED_DOWNLOAD_ACCE,
	FF_PLAYER_UI_MESSAGE_NEED_CHANGE_VIDEO_DEFINITION,
	FF_PLAYER_UI_MESSAGE_MOOV_AT_END,
	FF_PLAYER_UI_MESSAGE_RECEIVE_DATA_LEN,
	FF_PLAYER_UI_MESSAGE_START_PLAYING,
    FF_PLAYER_UI_MESSAGE_LOADED_TIME
}Flyfox_player_ui_message_e;

typedef enum tagFlyfox_player_video_display_ratio {
	FF_PLAYER_VIDEO_DISPLAY_RATIO_NONE = 0,
	FF_PLAYER_VIDEO_DISPLAY_RATIO_DEFAULT,
	FF_PLAYER_VIDEO_DISPLAY_RATIO_4_3,
	FF_PLAYER_VIDEO_DISPLAY_RATIO_16_9,
	FF_PLAYER_VIDEO_DISPLAY_RATIO_FULL_RECT

}Flyfox_player_video_display_mode_e;

#define  FLYFOX_MEDIA_DES_MAX             128
#define  FLYFOX_MEDIA_PIECE_HASH_ID       128
#define  FLYFOX_MEDIA_PIECE_KEY_ID        128
#define  FLYFOX_MEDIA_NAME_MAX            64
#define  FLYFOX_MEDIA_ASPECT              16
#define  FLYFOX_MEDIA_ONLINE_URL_MAX_LEN  512


typedef struct tagFlyfoxMediaAspect {
	FF_INT			    time;
	FF_CHAR             stzDes[FLYFOX_MEDIA_DES_MAX];
}FlyfoxMediaAspect_t;

typedef struct tagFlyfoxMediaSection {
	FF_DOUBLE			media_piece_duration;
	FF_INT			    media_piece_size;
	FF_CHAR	            media_piece_url[FLYFOX_MEDIA_ONLINE_URL_MAX_LEN];
	FF_CHAR	            media_piece_hashId[FLYFOX_MEDIA_PIECE_HASH_ID];
	FF_CHAR	            media_piece_key[FLYFOX_MEDIA_PIECE_KEY_ID];
	FF_CHAR	            media_piece_newAddress[FLYFOX_MEDIA_ONLINE_URL_MAX_LEN];
}FlyfoxMediaSection_t;

typedef struct tagFlyfoxOnlineMediaInfo {
	FF_INT				    id;
	FF_INT				    p2pflag;
	FF_INT				    vid;
	FF_BOOL			        longVideo;
	FF_INT				    tn;
	FF_INT				    status;
	FF_BOOL			        play;
	FF_BOOL			        fms;
	FF_BOOL			        fee;
	FF_INT				    pid;
	FF_INT				    fps;
	FF_INT				    version;
	FF_INT				    num;
	FF_INT				    st;
	FF_INT				    et;
	FF_INT                  norVid;
	FF_INT                  highVid;
	FF_INT                  superVid;
	FF_INT                  nPieceNum;
	FF_CHAR			        name[FLYFOX_MEDIA_NAME_MAX];
	FF_CHAR			        ch[FLYFOX_MEDIA_NAME_MAX];
	FF_CHAR			        allot[FLYFOX_MEDIA_ONLINE_URL_MAX_LEN];
	FF_CHAR			        url[FLYFOX_MEDIA_ONLINE_URL_MAX_LEN];
	FlyfoxMediaAspect_t	    aspects[FLYFOX_MEDIA_ASPECT];
	FlyfoxMediaSection_t*	MediasectionsArray;
    FF_INT                  type;
}FlyfoxOnlineMediaInfo_t;

typedef enum tagflyfox_media_player_3d_source_mode {
	FF_PLAYER_3D_SOUECE_ABOVE_BELOW  = 0,
	FF_PLAYER_3D_SOUECE_LEFT_RIGHT   = 1,
	FF_PLAYER_3D_SOUECE_INTERLACE    = 2,
}flyfox_media_player_3d_source_mode_e;


typedef enum tagflyfox_media_player_3d_convert_mode {
	FF_PLAYER_3D_CONVERT_TRANS2DTO3D  = 3,
	FF_PLAYER_3D_CONVERT_EASY3D   = 4,
}flyfox_media_player_3d_convert_mode_e;

typedef enum tagflyfox_media_player_3d_display_mode {
	FF_PLAYER_3D_DISPLAY_2D             = 0,
	FF_PLAYER_3D_DISPLAY_RED_BLUE       = 1,
	FF_PLAYER_3D_DISPLAY_GREEN_VIOL     = 2,
	FF_PLAYER_3D_DISPLAY_BROWN_BLUE     = 3
}flyfox_media_player_3d_display_mode_e;


typedef enum tagflyfox_media_player_3d_swap {
	FF_PLAYER_3D_SWAP_NORMAL             = 0,
	FF_PLAYER_3D_SWAP_SWAP               = 1,
}flyfox_media_player_3d_swap_e;

typedef struct tagFlyfox_player_3d_mode {
	flyfox_media_player_3d_source_mode_e   source_mode;
	flyfox_media_player_3d_convert_mode_e  convert_mode;
	flyfox_media_player_3d_display_mode_e  display_mode;
	flyfox_media_player_3d_swap_e          swap_mode;
	FF_INT                                 bit_depth;
	FF_BOOL                                b_opened;
}Flyfox_player_3d_mode_t;



typedef enum tagflyfox_media_player_video_resolution {
	FF_PLAYER_3D_VIDEO_NORMAL_RESOLUTION             = 0,
	FF_PLAYER_3D_VIDEO_HIGH_RESOLUTION               = 1,
	FF_PLAYER_3D_VIDEO_SUPER_RESOLUTION              = 2,
	FF_PLAYER_3D_VIDEO_AUTO_RESOLUTION               = 3,
}flyfox_media_player_video_resolution_e;

typedef enum tagVideoSurfaceFmt {
	FLYFOX_VIDEO_SURFACE_FMT_Invalid = 0,
	FLYFOX_VIDEO_SURFACE_FMT_YV12 ,
	FLYFOX_VIDEO_SURFACE_FMT_II420,
	FLYFOX_VIDEO_SURFACE_FMT_RGB444,
	FLYFOX_VIDEO_SURFACE_FMT_RGB565,
	FLYFOX_VIDEO_SURFACE_FMT_RGB24,
	FLYFOX_VIDEO_SURFACE_FMT_BGR24,
	FLYFOX_VIDEO_SURFACE_FMT_RGB32,
	FLYFOX_VIDEO_SURFACE_FMT_H264,
	FLYFOX_VIDEO_SURFACE_FMT_H263,
	FLYFOX_VIDEO_SURFACE_FMT_MPEG2,
	FLYFOX_VIDEO_SURFACE_FMT_MPEG4,
	FLYFOX_VIDEO_SURFACE_FMT_WMV,
	FLYFOX_VIDEO_SURFACE_FMT_RMVB,
	FLYFOX_VIDEO_SURFACE_FMT_H265,
	FLYFOX_VIDEO_SURFACE_FMT_End
} FlyfoxVideoSurfaceFmt_e;

typedef enum tagFlyfoxAudioChannels {
	FLYFOX_AUDIO_CHANNELS_MONO = 0x01,
	FLYFOX_AUDIO_CHANNELS_STEREO
}FlyfoxAudioChannels_e;

typedef enum tagFlyfoxAudioSampleRates {
	FLYFOX_AUDIO_SAMPLERATES_8000HZ,            //8000HZ
	FLYFOX_AUDIO_SAMPLERATES_11025HZ,           //11025HZ
	FLYFOX_AUDIO_SAMPLERATES_12000HZ,           //12000HZ
	FLYFOX_AUDIO_SAMPLERATES_16000HZ,           //16000HZ
	FLYFOX_AUDIO_SAMPLERATES_22050HZ,           //22050HZ
	FLYFOX_AUDIO_SAMPLERATES_24000HZ,           //24000HZ
	FLYFOX_AUDIO_SAMPLERATES_32000HZ,           //32000HZ
	FLYFOX_AUDIO_SAMPLERATES_44100HZ,           //44100HZ
	FLYFOX_AUDIO_SAMPLERATES_48000HZ,           //48000HZ
	FLYFOX_AUDIO_SAMPLERATES_64000HZ,           //64000HZ
	FLYFOX_AUDIO_SAMPLERATES_96000HZ            //96000HZ
}FlyfoxAudioSampleRates_e;

typedef enum tagFlyfoxAudioSampleFmt {
	FLYFOX_VIDEO_AUDIO_FMT_Invalide = 0,
	FLYFOX_VIDEO_AUDIO_FMT_PCM,
	FLYFOX_VIDEO_AUDIO_FMT_ADPCM,
	FLYFOX_VIDEO_AUDIO_FMT_AAC,
	FLYFOX_VIDEO_AUDIO_FMT_AMRWB,
	FLYFOX_VIDEO_AUDIO_FMT_AMRNB,
	FLYFOX_VIDEO_AUDIO_FMT_MP3,
	FLYFOX_VIDEO_AUDIO_FMT_WMA,
	FLYFOX_VIDEO_AUDIO_FMT_VORBIS,
	FLYFOX_VIDEO_AUDIO_FMT_AC3,
	FLYFOX_VIDEO_AUDIO_FMT_RM,
	FLYFOX_VIDEO_AUDIO_FMT_End
}FlyfoxAudioSampleFmt_e;

typedef enum tagFlyfoxMediaMuxerFmt {
	FLYFOX_MEDIA_MUXER_FMT_Invalid = 0,
	FLYFOX_MEDIA_MUXER_FMT_3GP ,
	FLYFOX_MEDIA_MUXER_FMT_MP4 ,
	FLYFOX_MEDIA_MUXER_FMT_WMV ,
	FLYFOX_MEDIA_MUXER_FMT_AVI ,
	FLYFOX_MEDIA_MUXER_FMT_MOV ,
	FLYFOX_MEDIA_MUXER_FMT_MKV ,
	FLYFOX_MEDIA_MUXER_FMT_FV,
	FLYFOX_MEDIA_MUXER_FMT_FLV,
	FLYFOX_MEDIA_MUXER_FMT_RMVB ,
	FLYFOX_MEDIA_MUXER_FMT_RTSP,
	FLYFOX_MEDIA_MUXER_FMT_MP3,	
	FLYFOX_MEDIA_MUXER_FMT_WMA,
	FLYFOX_MEDIA_MUXER_FMT_End
} FlyfoxMediaMuxerFmt_e;

typedef enum tagFlyfoxSubTitleFmt {
	FLYFOX_MEDIA_SUBTITLE_FMT_Invalid = 0,
	FLYFOX_MEDIA_SUBTITLE_FMT_SSA,
	FLYFOX_MEDIA_SUBTITLE_FMT_SRT, 
	FLYFOX_MEDIA_SUBTITLE_FMT_TEXT, 
	FLYFOX_MEDIA_SUBTITLE_FMT_End 
} FlyfoxSubTitleFmt_e;

typedef enum tagFlyfoxReaderFilterFmt {
	FLYFOX_READER_FILTER_FMT_Invalid = 0,
	FLYFOX_READER_FILTER_FMT_LOCAL_FILE ,
	FLYFOX_READER_FILTER_FMT_HTTP_FILE  ,
	FLYFOX_READER_FILTER_FMT_RTSP_FILE  ,
	FLYFOX_READER_FILTER_FMT_END
} FlyfoxReaderFilterFmt_e;

typedef struct tagFlySubtitleDesc {
	FlyfoxSubTitleFmt_e eSubtitleFmt;
}FlySubtitleDesc;

typedef struct tagFlyfoxVideoDesc {
	FlyfoxVideoSurfaceFmt_e                  eVideoSurfaceFmt;
	FF_UINT                                  nWidth;
	FF_UINT                                  nHeight;
	FF_UINT                                  nBitRate;
	FF_UINT                                  nFrameRate;
	FF_DWORD                                 nFrameTotalDuration;
	FF_UINT                                  nSizeImage ;
	FlySubtitleDesc                          sFlySubtitleDesc;
}FlyfoxVideoDesc_t;

typedef struct tagFlyfoxAudioDesc {
	FlyfoxAudioSampleFmt_e                   eAudioSampleFmt;
	FlyfoxAudioChannels_e                    eChannelCount;
	FlyfoxAudioSampleRates_e                 euiFreq;
	FF_UINT                                  uiCodecFreq;
	FF_UINT                                  uiAudioTrackNum;
	FF_UINT                                  uiBitCount;
	FF_UINT                                  uiBitsPerSample;
	FF_UTF8                                  strSongName[150];
	FF_UTF8                                  strAlbumName[50];
	FF_UTF8                                  strSingerName[50];
	FF_UINT                                  uiKbps;
}FlyfoxAudioDesc_t;

typedef enum tagFlyfoxSubtitleLanguage {
	FLYFOX_MEDIA_SUBTITLE_LAG_Invalid = 0,
	FLYFOX_MEDIA_SUBTITLE_LAG_ENGLISH,
	FLYFOX_MEDIA_SUBTITLE_LAG_SIMPLE_CHINESE, 
	FLYFOX_MEDIA_SUBTITLE_LAG_TRADITIONAL_CHINESE, 
	FLYFOX_MEDIA_SUBTITLE_LAG_OTHER, 
	FLYFOX_MEDIA_SUBTITLE_LAG_End 
}FlyfoxSubtitleLanguage_e;

typedef struct tagFlyfoxSubtitleDesc {
	FlyfoxSubTitleFmt_e             eFmt;
	FF_BOOL							bEmbedded;
	FlyfoxSubtitleLanguage_e		eLanguage;
	FF_CHAR							cLanguageName[260];
}FlyfoxSubtitleDesc_t;

#define FLYFOX_PLAYER_SUBTITLE_STREAM_MAX   8
#define FLYFOX_PLAYER_CUSTOM_DATA_BUFFER_MAX 1024

typedef struct tagFlyfoxMediaMuxerDesc {
	FlyfoxMediaMuxerFmt_e          eMediaMuxerFmt;
	FlyfoxVideoSurfaceFmt_e        eVideoSurfaceFmt;
	FlyfoxAudioSampleFmt_e         eAudioSampleFmt;
	FF_CHAR                        strFormat[64];
	FF_CHAR                        strVideoCodec[64];
	FF_CHAR                        strAudioCodec[64];
	FF_UINT                        uiVideoSurfaceCount;
	FF_UINT                        uiAudioSampleCount;
	FF_UINT                        uiEmbeddedSubtitleStreamCount;
	FlyfoxSubtitleDesc_t           sEmbeddedSubtitleStreamInfo[FLYFOX_PLAYER_SUBTITLE_STREAM_MAX];	
	FF_CHAR	                       strCustomDataBuffer[FLYFOX_PLAYER_CUSTOM_DATA_BUFFER_MAX];
	FF_UINT                        uiCustomDataBufferSize;
}FlyfoxMediaMuxerDesc_t;


typedef enum tagFlyfox_player_request_media_status_code {
	FLYFOX_PLAYER_REQUEST_MEDIA_STATUS_CLOSED_PIECE  = 0,
	FLYFOX_PLAYER_REQUEST_MEDIA_STATUS_NETWORK_ERROR
}Flyfox_player_request_media_status_code_e;

typedef FF_VOID(*flyfox_player_request_media_cb)(FF_INT nPieceIndex, FF_BYTE* in_pReceiveDataBuffer, FF_INT nBufferLen, FF_INT sid);
typedef FF_VOID(*flyfox_player_request_media_status_cb)(FF_INT in_bPieceIndex, Flyfox_player_request_media_status_code_e e_StatusCode);
typedef FF_VOID (*flyfox_media_data_cache_init)(flyfox_player_request_media_cb RequestMedia_cb, flyfox_player_request_media_status_cb Error_cb);
typedef FF_VOID (*flyfox_media_data_cache_uninit)();
typedef FF_BOOL (*flyfox_media_data_cache_request_piece)(FF_INT in_nPieceIndex, FF_INT  in_nStartPos, FF_INT sid); //  in_nSeekIndex ms
typedef FF_BOOL (*flyfox_media_data_cache_cancel_request_piece)(FF_INT in_nPieceIndex);

typedef struct tagFlyFoxMediaDataCacheFunc {
	flyfox_media_data_cache_init                                         data_cache_init;
	flyfox_media_data_cache_uninit                                       data_cache_uninit;
	flyfox_media_data_cache_request_piece                                data_cache_request_piece;
	flyfox_media_data_cache_cancel_request_piece                         data_cache_cancle_request_piece;
}FlyFoxMediaDataCacheFunc_t;

#define FF_E_GENIRAIC_FAILED        -1
#define FF_E_NOT_IMPLETMENT         -2
#define FF_E_NOT_SUPPORTED          -3
#define FF_E_OUT_OF_MEMERY          -4
#define FF_E_INVALID_FILE_PATH      -5
#define FF_E_CREATE_FILE_FAILED     -6

typedef enum tagFlyfoxDisplayRotationFmt {
	FLYFOX_DISPLAY_NO_CHANGED = 0,
	FLYFOX_DISPLAY_90_DEGREE_CLOCKWISE_ROTATED,
	FLYFOX_DISPLAY_90_DEGREE_COUNTCLOCKWISE_ROTATED
} FlyfoxDisplayRotationFmt_e;

typedef enum tagFlyfoxPlayerNotify {
	FLYFOX_PLAYER_NOTIFY_NETWORK_CONNECT_START = 0,
	FLYFOX_PLAYER_NOTIFY_NETWORK_CONNECT_FAILED,
	FLYFOX_PLAYER_NOTIFY_INIT_RESULT,
	FLYFOX_PLAYER_NOTIFY_NOT_SUPPORTED_FORMAT,
	FLYFOX_PLAYER_NOTIFY_OPEN_MEDIA_SUCCESS, 
	FLYFOX_PLAYER_NOTIFY_OPEN_MEDIA_FAILED,
	FLYFOX_PLAYER_NOTIFY_MEDIA_CONFIGURATION_OK,
	FLYFOX_PLAYER_NOTIFY_BUFFERING_START,
	FLYFOX_PLAYER_NOTIFY_BUFFERING_PERCENT,
	FLYFOX_PLAYER_NOTIFY_RECEIVE_DATA_LEN,
	FLYFOX_PLAYER_NOTIFY_BUFFERING_READY,
	FLYFOX_PLAYER_NOTIFY_OAVQ_DURATION_CHANGED,
	FLYFOX_PLAYER_NOTIFY_READ_DATA_OVER,
	FLYFOX_PLAYER_NOTIFY_CURRENT_PLAY_PIECE_INDEX,
	FLYFOX_PLAYER_NOTIFY_RELOAD_FILE_SIZE,
	FLYFOX_PLAYER_NOTIFY_MEDIA_FILE_TOTAL_SIZE,
	FLYFOX_PLAYER_NOTIFY_SEEK_MEDIA_SUCCESS,
	FLYFOX_PLAYER_NOTIFY_SEEK_MEDIA_FAILED,
	FLYFOX_PLAYER_NOTIFY_NETWORK_PRELOAD_OVER,
	FLYFOX_PLAYER_NOTIFY_PAUSE_ENGINE_TO_BUFFERING,
	FLYFOX_PLAYER_NOTIFY_RESUME_ENGINE_WHEN_BUFFERING,
	FLYFOX_PLAYER_NOTIF_PLAYING_PIECE_INDEX,
	FLYFOX_PLAYER_NOTIF_MESSAGE_MEDIA_TOTAL_DURATION,
	FLYFOX_PLAYER_NOTIFY_NETWORK_CLOSED,
	FLYFOX_PLAYER_NOTIFY_DDSHOW_NOT_SUPPORT,
	FLYFOX_PLAYER_NOTIFY_NEED_DOWNLOAD_ACCE,
	FLYFOX_PLAYER_NOTIFY_NEED_CHANGE_VIDEO_DEFINITION,
	FLYFOX_PLAYER_NOTIFY_PARSER_OVER,
	FLYFOX_PLAYER_NOTIFY_VIDEO_DECODE_FAILED,
	FLYFOX_PLAYER_NOTIFY_AUDIO_DECODE_FAILED,
    FLYFOX_PLAYER_NOTIFY_LOADED_TIME,
	FLYFOX_PLAYER_NOTIFY_END
}FlyfoxPlayerNotify_e;

typedef FF_VOID  (*player_notify_sendmsg_callback)(FlyfoxPlayerNotify_e a_eNotifyMsg, FF_VOID* wParam, FF_VOID* lParam);
extern player_notify_sendmsg_callback g_player_notify_sendmsg;

typedef FF_INT   (*custom_read_callback)(FF_VOID *opaque,FF_BYTE *data,FF_INT data_size);
typedef FF_INT64 (*custom_seek_callback)(FF_VOID *opaque,FF_INT64 pos,FF_INT whence);
typedef FF_BOOL  (*custom_async_read_callback)(FF_VOID *opaque,FF_INT index, FF_INT64 pos,FF_INT data_size,FF_INT sid);

typedef struct FlyfoxPlayerDrmParam {
    FF_VOID *opaque;
    custom_read_callback read_callback;
    custom_seek_callback seek_callback;
    custom_async_read_callback async_read_callback;
}FlyfoxPlayerDrmParam;

#ifdef __cplusplus
}
#endif

#endif
