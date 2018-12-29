#ifndef _FLYFOX_MEDIA_COMMON_H_
#define _FLYFOX_MEDIA_COMMON_H_

#include "flyfox_std.h"
#include "flyfox_media_player_def.h"

#define  FF_SOURCE_FILTER_HANDLE FF_INT
typedef FF_VOID (*flyfox_player_demux_decoder_open_cb)(FF_BOOL aSuccess);

typedef enum tagFlyfox_demux_decoder_message {
	FF_DEMUX_DECODER_MESSAGE_OPEN_SUCCESS  = 0,
	FF_DEMUX_DECODER_MESSAGE_OPEN_FAILED  ,
	FF_DEMUX_DECODER_MESSAGE_DEMUX_AV_OVER 
}Flyfox_demux_decoder_message_e;

typedef enum ff_player_media_AV_support {
	FF_MEDIA_HAS_NONE  = 0,
	FF_MEDIA_HAS_AUDIO = 0x01,	     // has audio
	FF_MEDIA_HAS_VIDEO = 0x02,	     // has video
	FF_MEDIA_HAS_AV    = 0x03,	     //has all
	FF_MEDIA_HAS_SUBTITLE  = 0x04,	 //has all
	FF_MEDIA_HAS_AVS  = 0x07,		 //has all
	FF_MEDIA_HAS_END
}ff_player_media_AV_support_e;

typedef enum ff_player_media_picture_type {
	FF_MEDIA_PICTURE_NONE = 0,      ///< Undefined
	FF_MEDIA_PICTURE_TYPE_I,        ///< Intra
	FF_MEDIA_PICTURE_TYPE_P,        ///< Predicted
	FF_MEDIA_PICTURE_TYPE_B,        ///< Bi-dir predicted
	FF_MEDIA_PICTURE_TYPE_S,        ///< S(GMC)-VOP MPEG4
	FF_MEDIA_PICTURE_TYPE_SI,       ///< Switching Intra
	FF_MEDIA_PICTURE_TYPE_SP,       ///< Switching Predicted
	FF_MEDIA_PICTURE_TYPE_BI,       ///< BI type
	FF_MEDIA_PICTURE_TYPE_END
}ff_player_media_picture_type_e;

typedef FF_VOID (*flyfox_player_mine_demux_decoder_cb)(Flyfox_demux_decoder_message_e in_eMessage, FF_INT  in_nIndex);

typedef enum tagFlyfox_render_message {
	FF_DEMUX_DECODER_MESSAGE_VMR_NOT_SUPPORT  = 0,
	FF_DEMUX_DECODER_MESSAGE_VMR_CONNET_FAILED 
}Flyfox_render_message_e;

typedef FF_VOID (*flyfox_player_render_cb)(Flyfox_render_message_e e_render_message);

#define  FLYFOX_DEMUX_DECODER_VIDEO_Q_NUM   8
#define  FLYFOX_DEMUX_DECODER_AUDIO_Q_NUM   32

#define FF_MAX_PKT_QUEUE_LOCAL_TS 	10000   // in ms unit
#define FF_MAX_PKT_QUEUE_NETWORK_TS 3000    // in ms unit

#define ff_min(a,b) (((a) <= (b)) ? (a):(b))
#define ff_max(a,b) (((a) >= (b)) ? (a):(b))

#define  PLAYER_ENGINE_PLAY_AUDIO_LENGTH  400

#define  PLAYER_ENGINE_PLAY_VIDEO_DETA_MAX   400

#define  M3U8_TS_BUFFER_READ_SIZE 16*1024  //20K
#define  M3U8_TS_BUFFER_SIZE 64*1024       //50K

////////////////////////////////////////////
#define FLYFOX_MOV_DEMUX_BUFFER_LENGTH      2*1024*1024
#define FLYFOX_MOV_DEMUX_ONCE_READ          64*1024
#define FLYFOX_READER_FILTER_READ_TIMEOUT   60000    //ms

#define  M3U8_START_QUEUE_TIME_LENGTH       3000     //1200
#define  M3U8_BUFFERING_QUEUE_TIME_LENGTH   6000

#define FLYFOX_DECODER_COST_TIME_MAX        45       //ms

typedef enum tagflyfox_control_player_display_mode {
	flyfox_control_player_display_mode_none = 0,
	flyfox_control_player_display_mode_original,
	flyfox_control_player_display_mode_ratio_room,
	flyfox_control_player_display_mode_fullscreen,
}flyfox_control_player_display_mode_e;

typedef enum taPlayerVideoRotateDirection {
	FLYFOX_PLAYER_ROTATE_DIRECTION_PORTRAIT =0 , 
	FLYFOX_PLAYER_ROTATE_DIRECTION_PORTRAIT_UPSIDE,
	FLYFOX_PLAYER_ROTATE_DIRECTION_LANDSCAPE_LEFT,
	FLYFOX_PLAYER_ROTATE_DIRECTION_LANDSCAPE_RIGHT,
	FLYFOX_PLAYER_ROTATE_DIRECTION_END
}PlayerVideoRotateDirection_e;

typedef enum tagFlyfoxVideoDecoderResolution {
	FLYFOX_VIDEO_DECODER_RESOLUTION_Invalide = 0,
	FLYFOX_VIDEO_DECODER_RESOLUTION_QCIF,
	FLYFOX_VIDEO_DECODER_RESOLUTION_CIF,
	FLYFOX_VIDEO_DECODER_RESOLUTION_QVGA,
	FLYFOX_VIDEO_DECODER_RESOLUTION_End
}FlyfoxVideoDecoderResolution_e;

typedef struct tagFlyfoxAVSample {
	FF_UTF8*                                  pBuffer;
	FF_UINT                                   nBufferSize;
	FF_INT64                                  dwTS;
	FF_INT                                    width;  // frame width	
	FF_INT                                    height; // frame height
	FF_INT64								  pts;    // for test
	FF_INT64								  dts;    // for test
    FF_BOOL                                   bHasVideoCfg;
	ff_player_media_picture_type_e            ePictureType;
} FlyfoxAVSample_t;

typedef enum  tagFlyfox_player_user_want {
	FF_PLAYER_USER_WANT_NONE = 0,
	FF_PLAYER_USER_WANT_PAUSE,
	FF_PLAYER_USER_WANT_PLAY,
}Flyfox_player_user_want_e;

typedef struct tagLyricStringProperties {
	FF_CHAR*		pString;
	FF_INT			nStringLength;
	FF_BOOL		bIsHighLight;
	FF_INT			nTimeStamp;
	FF_FLOAT		fNormRefPos;
}LyricStringProperties_s;

typedef struct tagLyricsRenderParams {
	FF_UINT						nCount;
	LyricStringProperties_s*	pLyricList; 
	FF_INT						nTimeStampOffset;
	FF_INT						nOrignalTimeStampOffset;
}LyricsRenderParams_s;

typedef struct tagOnlineDemuxDecoderParam {
    FlyfoxPlayerDrmParam        *pDrmParam;
    FlyFoxMediaDataCacheFunc_t  *pMediaCacheFunc;
}OnlineDemuxDecoderParam;

typedef enum tagEDxSoundMode {
    eDxSoundMode_S8,
    eDxSoundMode_S16LE,
    eDxSoundMode_S24LE,
    eDxSoundMode_AC3
}EDxSoundMode;

typedef struct tagSDSoundParams {
    FF_VOID  *hWnd;      //if there is not a specified play window, just set it as ZERO
    FF_UINT  uiChannels;
    FF_UINT  uiFreq;
    FF_UINT  uiBitsPerSample;
    EDxSoundMode eMode;
    FF_UINT uiBufSize;
}SNtsDSoundParams;

#ifdef __cplusplus
extern "C" {
#endif

FF_INT flyfox_audio_samplerates_e2v_convert(FlyfoxAudioSampleRates_e aFlyfoxAudioSampleRates);
FF_INT flyfox_audio_channels_e2v_convert(FlyfoxAudioChannels_e aAudioChannels);
  
FlyfoxAudioSampleRates_e flyfox_audio_samplerates_v2e_convert(FF_INT aFlyfoxAudioSampleRates);
FlyfoxAudioChannels_e    flyfox_audio_channels_v2e_convert(FF_INT aAudioChannels);

#ifdef __cplusplus
}
#endif

#endif
