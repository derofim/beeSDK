#include "bee/win/win_video_room.h"
#include "bee/win/win_video_room_sink.h"
#include "bee/win/win_video_source_cam.h"
#include "bee/media/audio_source.h"
#include "bee/media/video_source.h"
#include "bee/media/audio_source_default.h"
#include "utility/json/json.h"

#include <sstream>

namespace bee {

////////////////////////////////////WinVideoRoom//////////////////////////////////////
WinVideoRoom::WinVideoRoom(std::shared_ptr<WinVideoRoomSink> sink)
    : BeeService(kBeeSvcType_VideoRoom),
      media_type_(0),
      role_(eVideoRoomRole_None),
      initialized_(false),
      sink_(sink) {

}

WinVideoRoom::~WinVideoRoom() {

}

BeeErrorCode WinVideoRoom::init(
    bee_handle handle,
    const std::string &token,
    bee_int32_t timeout) {
    BeeErrorCode ret = kBeeErrorCode_Success;
    do {
        if (handle < 0 || token.empty()) {
            ret = kBeeErrorCode_Invalid_Param;
            break;
        }

        ret = BeeService::reg(handle);
        if (ret != kBeeErrorCode_Success) {
            break;
        }

        token_      = token;
        timeout_    = timeout;

        initialized_ = true;
    } while (0);
    return ret;
}

BeeErrorCode WinVideoRoom::join(
    const std::string &room_name,
    const std::string &uid,
    const std::string &nick_name,
    bool creator,
    VideoRoomRole role) {
    BeeErrorCode ret = kBeeErrorCode_Success;
    do {
        if (room_name.empty() || uid.empty() || nick_name.empty() || role == eVideoRoomRole_None) {
            ret = kBeeErrorCode_Invalid_Param;
            break;
        }

        room_name_ = room_name;
        uid_ = uid;
        nick_name_ = nick_name;
        role_ = role;

        std::string as_creator = creator ? "true" : "false";
        std::string push_audio = will_push_audio() ? "true" : "false";
        std::string push_video = will_push_video() ? "true" : "false";
        int64_t audio_source = (int64_t)audio_source_.get();
        int64_t video_source = (int64_t)video_source_.get();
        int64_t video_renderer = (int64_t)video_renderer_.get();

        std::ostringstream os;
        os << BEE_OBJ_BEGIN
            << BEE_STR_OBJ_BEGIN(room_name)     << room_name_   << BEE_STR_OBJ_CONTINUE
            << BEE_VAL_OBJ_BEGIN(create)        << as_creator   << BEE_VAL_OBJ_CONTINUE
            << BEE_STR_OBJ_BEGIN(uid)           << uid_         << BEE_STR_OBJ_CONTINUE
            << BEE_STR_OBJ_BEGIN(nick_name)     << nick_name_   << BEE_STR_OBJ_CONTINUE
            << BEE_STR_OBJ_BEGIN(token)         << token_       << BEE_STR_OBJ_CONTINUE
            << BEE_VAL_OBJ_BEGIN(role)          << role_        << BEE_VAL_OBJ_CONTINUE
            << BEE_COM_OBJ_BEGIN(push)
                << BEE_STR_OBJ_BEGIN(stream_name)   << stream_name_ << BEE_STR_OBJ_CONTINUE
                << BEE_COM_OBJ_BEGIN(audio)
                    << BEE_VAL_OBJ_BEGIN(present)   << push_audio   << BEE_VAL_OBJ_CONTINUE
                    << BEE_VAL_OBJ_BEGIN(source)    << audio_source << BEE_VAL_OBJ_END
                << BEE_COM_OBJ_CONTINUE(audio)
                << BEE_COM_OBJ_BEGIN(video)
                    << BEE_VAL_OBJ_BEGIN(present)   << push_video       << BEE_VAL_OBJ_CONTINUE
                    << BEE_VAL_OBJ_BEGIN(renderer)  << video_renderer   << BEE_VAL_OBJ_CONTINUE
                    << BEE_VAL_OBJ_BEGIN(source)    << video_source     << BEE_VAL_OBJ_END
                << BEE_COM_OBJ_END(video)
            << BEE_COM_OBJ_END(push)
            << BEE_OBJ_END;
        std::string arg = os.str();
        const std::string cmd = "Join";
        ret = BeeService::execute(cmd, arg, timeout_);
    } while (0);
    return ret;
}

BeeErrorCode WinVideoRoom::leave() {
    BeeErrorCode ret = kBeeErrorCode_Success;
    do {
        std::ostringstream os;
        os << BEE_OBJ_BEGIN
            << BEE_STR_OBJ_BEGIN(room_name) << room_name_ << BEE_STR_OBJ_CONTINUE
            << BEE_VAL_OBJ_BEGIN(reason) << 0 << BEE_VAL_OBJ_END
            << BEE_OBJ_END;
        std::string arg = os.str();
        const std::string cmd = "Leave";
        ret = BeeService::execute(cmd, arg, timeout_);
    } while (0);
    return ret;
}

BeeErrorCode WinVideoRoom::connect(
    const std::string &uid,
    const std::string &stream_name,
    bee_int32_t media_type,
    std::shared_ptr<VideoRenderer> video_renderer) {
    BeeErrorCode ret = kBeeErrorCode_Success;
    do {
        int64_t renderer = (int64_t)video_renderer.get();
        std::string pull_audio = will_pull_audio(media_type) ? "true" : "false";
        std::string pull_video = will_pull_video(media_type) ? "true" : "false";

        std::ostringstream os;
        os << BEE_OBJ_BEGIN
            << BEE_STR_OBJ_BEGIN(room_name)     << room_name_       << BEE_STR_OBJ_CONTINUE
            << BEE_STR_OBJ_BEGIN(uid)           << uid              << BEE_STR_OBJ_CONTINUE
            << BEE_STR_OBJ_BEGIN(stream_name)   << stream_name      << BEE_STR_OBJ_CONTINUE
            << BEE_VAL_OBJ_BEGIN(pull_video)    << pull_video       << BEE_VAL_OBJ_CONTINUE
            << BEE_VAL_OBJ_BEGIN(pull_audio)    << pull_audio       << BEE_VAL_OBJ_CONTINUE
            << BEE_VAL_OBJ_BEGIN(renderer)      << renderer         << BEE_VAL_OBJ_END
            << BEE_OBJ_END;
        std::string arg = os.str();
        const std::string cmd = "ConnectStream";
        ret = BeeService::execute(cmd, arg, timeout_);
    } while (0);
    return ret;
}

BeeErrorCode WinVideoRoom::disconnect(
    const std::string &uid,
    const std::string &stream_name,
    bee_int32_t reason) {
    return kBeeErrorCode_Success;
}

BeeErrorCode WinVideoRoom::setup_push_stream(
    const std::string &stream_name,
    bee_int32_t media_type,
    std::shared_ptr<AudioSource> audio_source,
    std::shared_ptr<VideoSource> video_source,
    std::shared_ptr<VideoRenderer> video_renderer) {
    BeeErrorCode ret = kBeeErrorCode_Success;
    do {
        stream_name_ = stream_name;
        media_type_ = media_type;
        audio_source_ = audio_source;
        video_source_ = video_source;
        video_renderer_ = video_renderer;

        if (will_push_video()) {
            if (video_source == NULL) {
                video_source_.reset(new WinVideoSourceCam());
            } else {
                video_source_ = video_source;
            }
            
            ret = video_source_->open();
            if (ret != kBeeErrorCode_Success) {
                break;
            }
        }

        if (will_push_audio()) {
            if (audio_source == NULL) {
                audio_source_.reset(new AudioSourceDefault());
            } else {
                audio_source_ = audio_source;
            }
            
            ret = audio_source_->open();
            if (ret != kBeeErrorCode_Success) {
                break;
            }
        }
    } while (0);
    return ret;
}

void WinVideoRoom::handle_data(const std::string &data) {
    BeeJson::Value json_msg;
    BeeJson::Reader parse;
    try {
        std::string room_name;
        std::string uid;
        std::string stream_name;
        BeeErrorCode ec = kBeeErrorCode_Success;
        std::string err_msg;
        VideoRoomMsgType eType = eVideoRoomMsgType_Count;

        if (!parse.parse(data, json_msg)) {
            return;
        }

        if (!json_msg["type"].isNull()) {
            int type = json_msg["type"].asInt();
            if (type >= eVideoRoomMsgType_Local_Party_Join && type < eVideoRoomMsgType_Count) {
                eType = static_cast<VideoRoomMsgType>(type);
            } else {
                //LOG
                return;
            }
        }
        
        if (!json_msg["ec"].isNull()) {
            int ret = json_msg["ec"].asInt();
            if (ret >= kBeeErrorCode_Success && ret < kBeeErrorCode_Count) {
                ec = static_cast<BeeErrorCode>(ret);
            } else {
                //LOG
                return;
            }
        }

        if (!json_msg["msg"].isNull()) {
            err_msg = json_msg["msg"].asCString();
        }

        if (!json_msg["room_name"].isNull()) {
            room_name = json_msg["room_name"].asCString();
        }

        if (!json_msg["stream_name"].isNull()) {
            stream_name = json_msg["stream_name"].asCString();
        }

        if (!json_msg["uid"].isNull()) {
            uid = json_msg["uid"].asCString();
        }

        switch (eType) {
        case eVideoRoomMsgType_Local_Party_Join: {
            handle_local_member_joined(stream_name, ec, err_msg);
            break;
        }
        case eVideoRoomMsgType_Remote_Party_Join: {
            handle_remote_member_joined(uid, stream_name, ec, err_msg);
            break;
        }
        case eVideoRoomMsgType_Local_Leave: {
            handle_local_member_leaved(stream_name, ec, err_msg);
            break;
        }
        case eVideoRoomMsgType_Remote_Leave: {
            handle_remote_member_leaved(uid, stream_name, ec, err_msg);
            break;
        }
        case eVideoRoomMsgType_Remote_Members: {
            std::vector<VideoRoomMemberInfo> members;
            BeeJson::Value members_info;
            if (!json_msg["members"].isNull()) {
                members_info = json_msg["members"];                
                for (size_t i = 0; i < members_info.size(); ++i) {
                    BeeJson::Value member_info = members_info[i];
                    if (member_info.isNull()) {
                        continue;
                    }

                    std::string uid;
                    std::string nick_name;
                    std::string stream_name;
                    int32_t media_type = 0;
                    VideoRoomRole role = eVideoRoomRole_None;

                    if (member_info["uid"].isNull()) {
                        continue;
                    } else {
                        uid = member_info["uid"].asCString();
                    }

                    if (!member_info["nick_name"].isNull()) {
                        nick_name = member_info["nick_name"].asCString();
                    }

                    if (member_info["stream_name"].isNull()) {
                        continue;
                    } else {
                        stream_name = member_info["stream_name"].asCString();
                    }

                    if (!member_info["media_type"].isNull()) {
                        media_type = member_info["media_type"].asInt();
                    }

                    if (!member_info["role"].isNull()) {
                        role = static_cast<VideoRoomRole>(member_info["role"].asInt());
                    }

                    members.emplace_back(
                        eVideoRoomPartyType_Remote,
                        uid,
                        nick_name,
                        stream_name,
                        media_type,
                        role);
                }
            }
            handle_remote_members(members);
            break;
        }
        case eVideoRoomMsgType_Audio_Input_Level: {
            int32_t level = -1;
            if (!json_msg["level"].isNull()) {
                level = json_msg["level"].asInt();
            }
            handle_audio_input_level(stream_name, level);
            break;
        }
        case eVideoRoomMsgType_Audio_Output_Level: {
            int32_t level = -1;
            if (!json_msg["level"].isNull()) {
                level = json_msg["level"].asInt();
            }
            handle_audio_output_level(uid, stream_name, level);
            break;
        }
        default:
            break;
        }
    } catch (...) {
        return;
    }
}

void WinVideoRoom::handle_local_member_joined(const std::string &stream_name, BeeErrorCode ec, const std::string &msg) {
    if (sink_ != NULL) {
        sink_->on_join(stream_name, ec, msg);
    }
}

void WinVideoRoom::handle_remote_member_joined(const std::string &uid, const std::string &stream_name, BeeErrorCode ec, const std::string &msg) {
    if (sink_ != NULL) {
        sink_->on_connect(uid, stream_name, ec, msg);
    }
}

void WinVideoRoom::handle_local_member_leaved(const std::string &stream_name, BeeErrorCode ec, const std::string &msg) {
    if (sink_ != NULL) {
        sink_->on_leave(stream_name, ec, msg);
    }
}

void WinVideoRoom::handle_remote_member_leaved(const std::string &uid, const std::string &stream_name, BeeErrorCode ec, const std::string &msg) {
    if (sink_ != NULL) {
        sink_->on_disconnect(uid, stream_name, ec, msg);
    }
}

void WinVideoRoom::handle_local_member_slowlink(const std::string &uid, const std::string &stream_name, const std::string &info) {
    if (sink_ != NULL) {
        sink_->on_slow_link(uid, stream_name, eVideoRoomPartyType_Local, info);
    }
}

void WinVideoRoom::handle_remote_member_slowlink(const std::string &uid, const std::string &stream_name, const std::string &info) {
    if (sink_ != NULL) {
        sink_->on_slow_link(uid, stream_name, eVideoRoomPartyType_Remote, info);
    }
}

void WinVideoRoom::handle_remote_members(const std::vector<VideoRoomMemberInfo> &members) {
    if (sink_ != NULL) {
        sink_->on_members(members);
    }
}

void WinVideoRoom::handle_audio_input_level(const std::string &stream_name, int32_t level) {
    if (sink_ != NULL) {
        sink_->on_audio_input_level(stream_name, level);
    }
}

void WinVideoRoom::handle_audio_output_level(const std::string &uid, const std::string &stream_name, int32_t level) {
    if (sink_ != NULL) {
        sink_->on_audio_output_level(uid, stream_name, level);
    }
}

bool WinVideoRoom::will_push_video() {
    return media_type_ & eVideoRoomMediaType_Video;
}

bool WinVideoRoom::will_push_audio() {
    return media_type_ & eVideoRoomMediaType_Audio;
}

bool WinVideoRoom::will_pull_video(bee_int32_t media_type) {
    return media_type & eVideoRoomMediaType_Video;
}

bool WinVideoRoom::will_pull_audio(bee_int32_t media_type) {
    return media_type & eVideoRoomMediaType_Audio;
}


} // namespace bee
