#include "stdafx.h"
#include "video_room.h"
#include "webrtc/api/mediastreaminterface.h"
#include "webrtc/api/peerconnectioninterface.h"

class WinStatsObserver : public webrtc::StatsObserver {
public:
    WinStatsObserver() {

    }
    ~WinStatsObserver() {

    }

    virtual void OnComplete(const webrtc::StatsReports& reports) {
        printf("1\n");
    }
};

bool VideoRoom::bee_initialized_ = false;

VideoRoom::VideoRoom(PostToUIThreadCb cb, VideoRoomClosedCb close_cb)
    : post_cb_(cb),
      close_cb_(close_cb),
      uid_(random_string(8)),
      stream_name_(random_string(8)) {
}

VideoRoom::~VideoRoom() {
}

bool VideoRoom::init(HINSTANCE instance, BeeErrorCode &ec1) {
    bool ret = true;
    do {
        if (!bee_initialized_) {
            init_bee_param(bee_param_);
            bee_int32_t ec2 = 0;
            ec1 = Bee::instance()->initialize(bee_param_, NULL, bee_timeout_, ec2);
            if (ec1 != kBeeErrorCode_Success || ec2 != 0) {
                ret = false;
                break;
            }
            bee_initialized_ = true;
        }

        std::vector<BeeCapability> capability;
        ec1 = Bee::instance()->open_session(bee_session_, capability);
        if (bee_session_ == -1 || ec1 != kBeeErrorCode_Success) {
            ret = false;
            break;
        }

        bool valid = false;
        for (BeeCapability cap : capability) {
            if (cap.svc_code == kBeeSvcType_VideoRoom) {
                valid = true;
                break;
            }
        }

        if (!valid) {
            ret = false;
            break;
        }

        int width = GetSystemMetrics(SM_CXFULLSCREEN) - 20;
        int height = width / 2;
        height = height * 3 / 4;

        room_layout_.reset(new RoomLayout(instance, width, height, shared_from_this()));
        room_layout_->CreateBackground();

        video_room_svc_.reset(new WinVideoRoom(shared_from_this()));
    } while (0);
    return ret;
}

bool VideoRoom::uninit() {
    bool ret = true;
    do {
        if (!bee_initialized_) {
            break;
        }

        Bee::instance()->uninitialize();

        if (room_layout_ != NULL) {
            room_layout_.reset();
        }

        bee_initialized_ = false;
    } while (0);
    return ret;
}

bool VideoRoom::do_create(const std::string &room_name, const std::string &my_name, bool push_audio, bool push_video, BeeErrorCode &ec) {
    bool ret = true;
    do {
        if (room_layout_ == NULL) {
            ret = false;
            break;
        }

        room_name_ = room_name;
        uname_ = my_name;

        PartyWindow::Ptr party_window = room_layout_->CreateLocalWindow(bee_session_, room_name_, stream_name_);
        if (party_window == NULL) {
            ret = false;
            break;
        }

        if (video_room_svc_ == NULL) {
            ret = false;
            break;
        }

        ec = video_room_svc_->init(bee_session_, token_, bee_timeout_);
        if (ec != kBeeErrorCode_Success) {
            ret = false;
            break;
        }

        if (push_audio) {
            media_type_ |= eVideoRoomMediaType_Audio;
        }

        if (push_video) {
            media_type_ |= eVideoRoomMediaType_Video;
        }

        std::shared_ptr<WinVideoRendererD3D> renderer;
        if (push_video) {
            renderer = party_window->get_video_renderer();
        }

        ec = video_room_svc_->setup_push_stream(stream_name_, media_type_, NULL, NULL, renderer);
        if (ec != kBeeErrorCode_Success) {
            ret = false;
            break;
        }

        ec = video_room_svc_->join(room_name_, uid_, uname_, true, eVideoRoomRole_Party);
        if (ec != kBeeErrorCode_Success) {
            ret = false;
            break;
        }
    } while (0);
    return ret;
}

bool VideoRoom::do_join(const std::string &room_name, const std::string &my_name, bool push_audio, bool push_video, BeeErrorCode &ec) {
    bool ret = true;
    do {
        if (room_layout_ == NULL) {
            ret = false;
            break;
        }

        room_name_ = room_name;
        uname_ = my_name;
        
        PartyWindow::Ptr party_window = room_layout_->CreateLocalWindow(bee_session_, room_name_, stream_name_);
        if (party_window == NULL) {
            ret = false;
            break;
        }
        
        if (video_room_svc_ == NULL) {
            ret = false;
            break;
        }

        ec = video_room_svc_->init(bee_session_, token_, bee_timeout_);
        if (ec != kBeeErrorCode_Success) {
            ret = false;
            break;
        }
        
        if (push_audio) {
            media_type_ |= eVideoRoomMediaType_Audio;
        }

        if (push_video) {
            media_type_ |= eVideoRoomMediaType_Video;
        }

        std::shared_ptr<WinVideoRendererD3D> renderer;
        if (push_video) {
            renderer = party_window->get_video_renderer();
        }
                
        ec = video_room_svc_->setup_push_stream(stream_name_, media_type_, NULL, NULL, renderer);
        if (ec != kBeeErrorCode_Success) {
            ret = false;
            break;
        }

        ec = video_room_svc_->join(room_name_, uid_, uname_, false, eVideoRoomRole_Party);
        if (ec != kBeeErrorCode_Success) {
            ret = false;
            break;
        }
    } while (0);
    return ret;
}

bool VideoRoom::leave() {
    if (video_room_svc_ == NULL) {
        return false;
    } else {
        bool ret = video_room_svc_->leave() == kBeeErrorCode_Success;
        video_room_svc_.reset();
        return ret;
    }
}

void VideoRoom::ui_thread_callback(UIThreadMsg msg_id, void *data) {
    switch (msg_id) {
    case eUIThreadMsgId_Video_Room_On_Joined: {
        process_on_join((OnJoinEvent*)data);
        break;
    }
    case eUIThreadMsgId_Video_Room_On_Leaved: {
        process_on_leave((OnLeaveEvent*)data);
        break;
    }
    case eUIThreadMsgId_Video_Room_On_Members: {
        process_on_members((OnMembersEvent*)data);
        break;
    }
    case eUIThreadMsgId_Video_Room_On_Connected: {
        process_on_connect((OnConnectEvent*)data);
        break;
    }
    case eUIThreadMsgId_Video_Room_On_Disconnected: {
        process_on_disconnect((OnDisConnectEvent*)data);
        break;
    }
    default:
        break;
    }
}

void VideoRoom::on_room_layout_close() {
    if (room_layout_ != NULL) {
        room_layout_.reset();
    }
    
    if (close_cb_ != NULL) {
        close_cb_();
    }

    Bee::instance()->close_session(bee_session_);
    bee_session_ = -1;
}

std::string VideoRoom::random_string(size_t len) {
    std::string str;
    str.resize(len);
    const char char_set[63] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
    char *p = (char*)str.data();
    for (size_t i = 0; i < len; ++i) {
        p[i] = char_set[rd_() % 62];
    }
    return str;
}

void VideoRoom::uninit_on_exit() {
    if (bee_initialized_) {
        Bee::instance()->uninitialize();
        bee_initialized_ = false;
    }
}

void VideoRoom::init_bee_param(BeeSystemParam &param) {
    memset(&param, 0, sizeof(BeeSystemParam));

    param.platform_type = kPlatformType_PC;
    param.net_type = kNetType_WireLine;
    param.app_name = "VideoRoomTest";
    param.app_version = "1.0";
    param.system_info = "win10";
    param.machine_code = "ABCD";
    param.log_path = "./log";
    param.log_level = kLogLevel_Debug;
    param.log_volume_count = 50;
    param.log_volume_size = 10 * 1024; //K
    param.session_count = 16;
    param.enable_statusd = true;
}

void VideoRoom::post(UIThreadMsg msg_id, void *data) {
    if (post_cb_ != NULL) {
        post_cb_(msg_id, data);
    }
}

void VideoRoom::on_join(const std::string &stream_name, BeeErrorCode error, const std::string &msg) {
    OnJoinEvent *ev = new OnJoinEvent;
    ev->stream_name = stream_name;
    ev->error = error;
    ev->msg = msg;
    post(eUIThreadMsgId_Video_Room_On_Joined, ev);
}

void VideoRoom::on_leave(const std::string &stream_name, BeeErrorCode error, const std::string &msg) {
    OnLeaveEvent *ev = new OnLeaveEvent;
    ev->stream_name = stream_name;
    ev->error = error;
    ev->msg = msg;
    post(eUIThreadMsgId_Video_Room_On_Leaved, ev);
}

void VideoRoom::on_members(std::vector<VideoRoomMemberInfo> remote_members) {
    OnMembersEvent *ev = new OnMembersEvent;
    ev->remote_members = remote_members;
    post(eUIThreadMsgId_Video_Room_On_Members, ev);
}

void VideoRoom::on_connect(const std::string &uid, const std::string &stream_name, BeeErrorCode error, const std::string &msg) {
    OnConnectEvent *ev = new OnConnectEvent;
    ev->uid = uid;
    ev->stream_name = stream_name;
    ev->error = error;
    ev->msg = msg;
    post(eUIThreadMsgId_Video_Room_On_Connected, ev);
}

void VideoRoom::on_disconnect(const std::string &uid, const std::string &stream_name, BeeErrorCode error, const std::string &msg) {
    OnConnectEvent *ev = new OnConnectEvent;
    ev->uid = uid;
    ev->stream_name = stream_name;
    ev->error = error;
    ev->msg = msg;
    post(eUIThreadMsgId_Video_Room_On_Disconnected, ev);
}

void VideoRoom::on_slow_link(const std::string &uid, const std::string &stream_name, VideoRoomPartyType party_type, const std::string &info) {

}

void VideoRoom::on_audio_input_level(const std::string &stream_name, bee_int32_t level) {

}

void VideoRoom::on_audio_output_level(const std::string &uid, const std::string &stream_name, bee_int32_t level) {

}

void VideoRoom::process_on_join(OnJoinEvent *ev) {
    do {
        if (ev == NULL) {
            break;
        }

        if (ev->error != kBeeErrorCode_Success) {
            break;
        }

        if (room_layout_ != NULL && (media_type_ & eVideoRoomMediaType_Video)) {
            room_layout_->OpenLocalVideoRenderer(bee_session_, room_name_, ev->stream_name);
        }

        delete ev;
    } while (0);
}

void VideoRoom::process_on_leave(OnLeaveEvent *ev) {
    if (ev == NULL) {
        return;
    }

    if (room_layout_ != NULL) {
        room_layout_->CloseLocalWindow(bee_session_, room_name_, ev->stream_name);
    }
    delete ev;
}

void VideoRoom::process_on_members(OnMembersEvent *ev) {
    if (ev == NULL) {
        return;
    }

    if (IDCANCEL == MessageBoxW(NULL, L"有用户加入，是否拉流", L"拉流", MB_OKCANCEL)) {
        return;
    }

    if (room_layout_ != NULL) {
        for (VideoRoomMemberInfo info : ev->remote_members) {
            PartyWindow::Ptr party = room_layout_->CreateRemoteWindow(bee_session_, room_name_, info.stream_name);
            if (party != NULL && video_room_svc_ != NULL) {
                std::shared_ptr<VideoRenderer> renderer = party->get_video_renderer();
                video_room_svc_->connect(info.uid, info.stream_name, info.media_type, renderer);
            }
        }
    }

    delete ev;
}

void VideoRoom::process_on_connect(OnConnectEvent *ev) {
    if (ev == NULL) {
        return;
    }

    if (room_layout_ != NULL) {
        room_layout_->OpenRemoteVideoRenderer(bee_session_, room_name_, ev->stream_name);
    }

    delete ev;
}

void VideoRoom::process_on_disconnect(OnDisConnectEvent *ev) {
    if (ev == NULL) {
        return;
    }

    if (room_layout_ != NULL) {
        room_layout_->CloseRemotebWindow(bee_session_, room_name_, ev->stream_name);
    }

    delete ev;
}
