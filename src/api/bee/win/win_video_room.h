#ifndef __WIN_VIDEO_ROOM_H__
#define __WIN_VIDEO_ROOM_H__

#include "bee/base/bee.h"
#include "bee/base/bee_service.h"
#include "bee/win/video_room_def.h"

namespace bee {

class WinVideoRoomSink;
class VideoSource;
class AudioSource;
class VideoRenderer;

////////////////////////////////////WinVideoRoom//////////////////////////////////////
class WinVideoRoom : public BeeService {
public:
    typedef std::shared_ptr<WinVideoRoom> Ptr;
    WinVideoRoom(std::shared_ptr<WinVideoRoomSink> sink);
    ~WinVideoRoom();

public:
    /**********************************************************************************
    * Function : init
    * Desc     : Initialize with some common on-time param.
    *
    * param    : handle                     [IN] BeeSession handle.
    * param    : token                      [IN] App token.
    * param    : timeout                    [IN] Timeout of method in ms.
    *
    * Return   : Bee error code.
    ***********************************************************************************/
    BeeErrorCode init(
        bee_handle handle,
        const std::string &token, 
        bee_int32_t timeout);

    /**********************************************************************************
    * Function : join
    * Desc     : Create or join in a video room.
    *    
    * param    : room_name                  [IN] Room name of video room.
    * param    : uid                        [IN] Unique id of local user.
    * param    : nick_name                  [IN] Nick name of local user.
    * param    : creator                    [IN] If creator is true, create a new video room, or join in a exist one.
    * param    : role                       [IN] Role in video room.
    *
    * Return   : Bee error code.
    ***********************************************************************************/
    BeeErrorCode join(
        const std::string &room_name, 
        const std::string &uid,
        const std::string &nick_name, 
        bool creator, 
        VideoRoomRole role);

    /**********************************************************************************
    * Function : leave
    * Desc     : Leave from current video room.
    *
    * Return   : Bee error code.
    ***********************************************************************************/
    BeeErrorCode leave();

    /**********************************************************************************
    * Function : connect
    * Desc     : Pull stream from remote user.
    *
    * param    : uid                    [IN] User unique id.
    * param    : stream_name            [IN] Stream name.
    * param    : media_type             [IN] Stream media type, or(|) by VideoRoomMediaType.
    * param    : video_renderer         [IN] Video renderer for playback video stream.
    *
    * Return   : Bee error code.
    ***********************************************************************************/
    BeeErrorCode connect(
        const std::string &uid,
        const std::string &stream_name,
        bee_int32_t media_type,
        std::shared_ptr<VideoRenderer> video_renderer);
    
    /**********************************************************************************
    * Function : disconnect
    * Desc     : Disconnect a remote stream.
    *
    * param    : uid                    [IN] User unique id.
    * param    : stream_name            [IN] Stream name.
    * param    : reason                 [IN] Disconnect reason.
    *
    * Return   : Bee error code.
    ***********************************************************************************/
    BeeErrorCode disconnect(
        const std::string &uid,
        const std::string &stream_name,
        bee_int32_t reason);

    /**********************************************************************************
    * Function : setup_push_stream
    * Desc     : Setup push stream, must called before join if push video or audio.
    *
    * param    : stream_name                [IN] Push stream name.
    * param    : media_type                 [IN] Push stream media type, can be or(|) with VideoRoomMediaType.
    * param    : audio_source               [IN] Audio source, if set to NULL, could using default.
    * param    : video_source               [IN] Video source, if set to NULL, could using default.
    * param    : video_renderer             [IN] Video renderer for playback local video stream.
    *
    * Return   : Bee error code.
    ***********************************************************************************/
    BeeErrorCode setup_push_stream(
        const std::string &stream_name, 
        bee_int32_t media_type,
        std::shared_ptr<AudioSource> audio_source,
        std::shared_ptr<VideoSource> video_source,
        std::shared_ptr<VideoRenderer> video_renderer);

    /**********************************************************************************
    * Function : get_timeout
    * Desc     : Return timeout of method in ms.
    *
    * Return   : Timeout of method.
    ***********************************************************************************/
    bee_int32_t get_timeout() { return timeout_; }

protected:
    virtual void handle_data(const std::string &data);

private:
    void handle_local_member_joined(const std::string &stream_name, BeeErrorCode ec, const std::string &msg);
    void handle_remote_member_joined(const std::string &uid, const std::string &stream_name, BeeErrorCode ec, const std::string &msg);
    void handle_local_member_leaved(const std::string &stream_name, BeeErrorCode ec, const std::string &msg);
    void handle_remote_member_leaved(const std::string &uid, const std::string &stream_name, BeeErrorCode ec, const std::string &msg);
    void handle_local_member_slowlink(const std::string &uid, const std::string &stream_name, const std::string &info);
    void handle_remote_member_slowlink(const std::string &uid, const std::string &stream_name, const std::string &info);
    void handle_remote_members(const std::vector<VideoRoomMemberInfo> &members);
    void handle_audio_input_level(const std::string &stream_name, int32_t level);
    void handle_audio_output_level(const std::string &uid, const std::string &stream_name, int32_t level);
    bool will_push_video();
    bool will_push_audio();
    bool will_pull_video(bee_int32_t media_type);
    bool will_pull_audio(bee_int32_t media_type);

protected:
    std::string token_;
    std::string room_name_;    
    std::string uid_;
    std::string nick_name_;
    std::string stream_name_;
    bee_int32_t media_type_;
    VideoRoomRole role_;
    bool initialized_;
    bee_int32_t timeout_;
    std::shared_ptr<WinVideoRoomSink> sink_;
    std::shared_ptr<VideoSource> video_source_;
    std::shared_ptr<AudioSource> audio_source_;
    std::shared_ptr<VideoRenderer> video_renderer_;
};

} // namespace bee

#endif // #ifndef __WIN_VIDEO_ROOM_H__
