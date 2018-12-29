#ifndef __VIDEO_ROOM_H__
#define __VIDEO_ROOM_H__

#include "bee/base/bee.h"
#include "bee/base/bee_sink.h"
#include "bee/win/win_video_room.h"
#include "bee/win/win_video_room_sink.h"
#include "room_layout.h"
#include <memory>
#include <random>

using namespace bee;

typedef enum UIThreadMsg {
    eUIThreadMsgId_Video_Room_On_Joined = 0,
    eUIThreadMsgId_Video_Room_On_Leaved,
    eUIThreadMsgId_Video_Room_On_Members,
    eUIThreadMsgId_Video_Room_On_Connected,
    eUIThreadMsgId_Video_Room_On_Disconnected,
    eUIThreadMsgId_Count
}UIThreadMsg;

typedef struct OnJoinEvent {
    std::string stream_name;
    BeeErrorCode error;
    std::string msg;
}OnJoinEvent;

typedef struct OnLeaveEvent {
    std::string stream_name;
    BeeErrorCode error;
    std::string msg;
}OnLeaveEvent;

typedef struct OnMembersEvent {
    std::vector<VideoRoomMemberInfo> remote_members;
}OnMembersEvent;

typedef struct OnConnectEvent {
    std::string uid;
    std::string stream_name;
    BeeErrorCode error;
    std::string msg;
}OnConnectEvent;

typedef struct OnDisConnectEvent {
    std::string uid;
    std::string stream_name;
    BeeErrorCode error;
    std::string msg;
}OnDisConnectEvent;

typedef enum BoardType {
	eBoardType_None = 0,
	eBoardType_Teacher,
	eBoardType_Student
}BoardType;

typedef void(*PostToUIThreadCb)(UIThreadMsg msg_id, void *data);
typedef void(*VideoRoomClosedCb)();

class RoomLayout;
class VideoRoom : public BeeSink, public std::enable_shared_from_this<VideoRoom>, public WinVideoRoomSink {
public:
    VideoRoom(PostToUIThreadCb cb, VideoRoomClosedCb close_cb);
    ~VideoRoom();

public:
    bool init(HINSTANCE instance, BeeErrorCode &ec1);
    bool uninit();
    bool do_create(const std::string &room_name, const std::string &my_name, bool push_audio, bool push_video, BeeErrorCode &ec);
    bool do_join(const std::string &room_name, const std::string &my_name, bool push_audio, bool push_video, BeeErrorCode &ec);
    bool leave();
    void ui_thread_callback(UIThreadMsg msg_id, void *data);
    void on_room_layout_close();
    std::string random_string(size_t len);
    static void uninit_on_exit();

    virtual void on_join(const std::string &stream_name, BeeErrorCode error, const std::string &msg);
    virtual void on_leave(const std::string &stream_name, BeeErrorCode error, const std::string &msg);
    virtual void on_members(std::vector<VideoRoomMemberInfo> remote_members);
    virtual void on_connect(const std::string &uid, const std::string &stream_name, BeeErrorCode error, const std::string &msg);
    virtual void on_disconnect(const std::string &uid, const std::string &stream_name, BeeErrorCode error, const std::string &msg);
    virtual void on_slow_link(const std::string &uid, const std::string &stream_name, VideoRoomPartyType party_type, const std::string &info);
    virtual void on_audio_input_level(const std::string &stream_name, bee_int32_t level);
    virtual void on_audio_output_level(const std::string &uid, const std::string &stream_name, bee_int32_t level);

protected:
    void init_bee_param(BeeSystemParam &param);
    void post(UIThreadMsg msg_id, void *data);

    virtual void on_log(const char *log) {}
    virtual void on_notify(BeeErrorCode ec1, bee_int32_t ec2) {}

    void process_on_join(OnJoinEvent *ev);
    void process_on_leave(OnLeaveEvent *ev);
    void process_on_members(OnMembersEvent *ev);
    void process_on_connect(OnConnectEvent *ev);
    void process_on_disconnect(OnDisConnectEvent *ev);

protected:
    static bool bee_initialized_;
    BeeSystemParam bee_param_;
    int bee_timeout_ = 5000;
    bee_handle bee_session_ = -1;
    std::shared_ptr<RoomLayout> room_layout_;
    PostToUIThreadCb post_cb_ = NULL;
    VideoRoomClosedCb close_cb_ = NULL;
    std::string room_name_;
    std::string extra_data_;
    std::string token_ = "5eb73bb4918354cee213903c3940c0e6183f289d";
    std::random_device rd_;
    std::string uid_;
    std::string stream_name_;    
    std::string uname_ = "HeZhen";
    WinVideoRoom::Ptr video_room_svc_; 
    int media_type_ = 0;
};

#endif
