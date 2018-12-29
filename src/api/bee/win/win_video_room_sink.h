#ifndef __WIN_VIDEO_ROOM_SINK_H__
#define __WIN_VIDEO_ROOM_SINK_H__

#include "bee/base/bee.h"
#include "bee/win/video_room_def.h"

namespace bee {

////////////////////////////////////WinVideoRoomSink//////////////////////////////////////
class WinVideoRoomSink {
public:
    /**********************************************************************************
    * Function : on_join
    * Desc     : Async result of join method.
    *
    * param    : stream_name            [IN] Local published stream name.
    * param    : error                  [IN] Join method's return error code, see BeeErrorCode definition.
    * param    : msg                    [IN] Error description.
    *
    ***********************************************************************************/
    virtual void on_join(const std::string &stream_name, BeeErrorCode error, const std::string &msg) = 0;

    /**********************************************************************************
    * Function : on_leave
    * Desc     : result of leave method.
    *
    * param    : stream_name            [IN] Local published stream name.
    * param    : error                  [IN] Join method's return error code, see BeeErrorCode definition.
    * param    : msg                    [IN] Error description.
    *
    ***********************************************************************************/
    virtual void on_leave(const std::string &stream_name, BeeErrorCode error, const std::string &msg) = 0;

    /**********************************************************************************
    * Function : on_members
    * Desc     : Notify of video room members, app could call connect later.
    *
    * param    : remote_members         [IN] Video room members' info, see VideoRoomMemberInfo definition.
    *
    ***********************************************************************************/
    virtual void on_members(std::vector<VideoRoomMemberInfo> remote_members) = 0;

    /**********************************************************************************
    * Function : on_connect
    * Desc     : Async result of connect method.
    *
    * param    : uid                    [IN] Connected user id.
    * param    : stream_name            [IN] Connected stream name.
    * param    : error                  [IN] Connect method's return error code, see BeeErrorCode definition.
    * param    : msg                    [IN] Error description.
    *
    ***********************************************************************************/
    virtual void on_connect(const std::string &uid, const std::string &stream_name, BeeErrorCode error, const std::string &msg) = 0;

    /**********************************************************************************
    * Function : on_disconnect
    * Desc     : Notify of disconnect from video room member.
    *
    * param    : uid                    [IN] Connected user id.
    * param    : stream_name            [IN] DisConnected stream name, if null, the local stream disconnected.
    * param    : error                  [IN] Reason of disconnect.
    * param    : msg                    [IN] Error description.
    *
    ***********************************************************************************/
    virtual void on_disconnect(const std::string &uid, const std::string &stream_name, BeeErrorCode error, const std::string &msg) = 0;

    /**********************************************************************************
    * Function : on_slow_link
    * Desc     : Notify of stream slow link.
    *
    * param    : uid                    [IN] Connected user id.
    * param    : stream_name            [IN] Stream name.
    * param    : party_type             [IN] Member type, local or remote.
    * param    : info                   [IN] Extra custom info.
    *
    ***********************************************************************************/
    virtual void on_slow_link(const std::string &uid, const std::string &stream_name, VideoRoomPartyType party_type, const std::string &info) = 0;

    /**********************************************************************************
    * Function : on_audio_input_level
    * Desc     : Notify of audio input level.
    *
    * param    : stream_name            [IN] Stream name.
    * param    : level                  [IN] Audio input level.
    *
    ***********************************************************************************/
    virtual void on_audio_input_level(const std::string &stream_name, bee_int32_t level) = 0;

    /**********************************************************************************
    * Function : on_audio_output_level
    * Desc     : Notify of audio output level.
    *
    * param    : uid                    [IN] Connected user id.
    * param    : stream_name            [IN] Stream name.
    * param    : level                  [IN] Audio output level.
    *
    ***********************************************************************************/
    virtual void on_audio_output_level(const std::string &uid, const std::string &stream_name, bee_int32_t level) = 0;
};

} // namespace bee

#endif // #ifndef __WIN_VIDEO_ROOM_SINK_H__
