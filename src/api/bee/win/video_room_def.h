/**
 *  @file        video_room_def.h
 *  @brief       BeeSDK视频会议室通用声明文件.
 *  @author      HeZhen
 *  @copyright   sohu-inc
 *  @version     2.0.0.1
 */

#ifndef __VIDEO_ROOM_DEF_H__
#define __VIDEO_ROOM_DEF_H__

#include "bee/base/bee_define.h"
#include <string>

namespace bee {
    
/// 视频会议业务码.
const bee_int32_t kBeeSvcType_VideoRoom = 1;

/// 视频会议消息类型.
typedef enum VideoRoomMsgType {
    eVideoRoomMsgType_Local_Party_Join = 0,     //!< 本地用户加入并推流结果.
    eVideoRoomMsgType_Remote_Party_Join,        //!< 远端用户拉流结果.
    eVideoRoomMsgType_Local_Leave,              //!< 本地用户离开并断流结果.
    eVideoRoomMsgType_Remote_Leave,             //!< 远端用户断流结果.
    eVideoRoomMsgType_Closed,                   //!< 已经关闭.
    eVideoRoomMsgType_Local_Slow_Link,          //!< 本地弱网通知.
    eVideoRoomMsgType_Remote_Slow_Link,         //!< 远端弱网通知.
    eVideoRoomMsgType_Remote_Members,           //!< 远端用户进入会议室.
    eVideoRoomMsgType_Message,                  //!< 消息(保留).
    eVideoRoomMsgType_Audio_Input_Level,        //!< 音频输入音量.
    eVideoRoomMsgType_Audio_Output_Level,       //!< 音频输出音量.
    eVideoRoomMsgType_Count                     //!< 视频会议消息类型数量.
}VideoRoomMsgType;

/// 视频会议媒体类型.
typedef enum VideoRoomMediaType {
    eVideoRoomMediaType_None = 0,               //!< 无.
    eVideoRoomMediaType_Audio = 1,              //!< 音频.
    eVideoRoomMediaType_Video = 2               //!< 视频.
}VideoRoomMediaType;

/// 视频会议成员角色.
typedef enum VideoRoomRole {
    eVideoRoomRole_None = 0,                    //!< 无.
    eVideoRoomRole_Manager,                     //!< 管理者.
    eVideoRoomRole_Party                        //!< 普通参与者.
}VideoRoomRole;

/// 视频会议成员类型.
typedef enum VideoRoomPartyType {
    eVideoRoomPartyType_Local = 0,              //!< 本地用户.
    eVideoRoomPartyType_Remote                  //!< 远端用户.
}VideoRoomPartyType;

/// BeeSDK视频会议成员信息.
typedef struct VideoRoomMemberInfo {
    VideoRoomPartyType party_type;              //!< 成员类型.
    std::string uid;                            //!< 用户唯一标识.
    std::string nick_name;                      //!< 用户昵称.
    std::string stream_name;                    //!< 用户推流流名.
    bee_int32_t media_type;                     //!< 用户推流媒体类型.
    VideoRoomRole role;                         //!< 用户会议角色.

    /**
     *  @brief  VideoRoomMemberInfo类构造函数.
     *  @param  party_type      成员类型.
     *  @param  uid             用户唯一标识.
     *  @param  nick_name       用户昵称.
     *  @param  stream_name     用户推流流名.
     *  @param  media_type      用户推流媒体类型.
     *  @param  role            用户会议角色.
     */
    VideoRoomMemberInfo(
        VideoRoomPartyType party_type,
        const std::string &uid,
        const std::string &nick_name,
        const std::string &stream_name,
        bee_int32_t media_type,
        VideoRoomRole role) {
        this->party_type = party_type;
        this->uid = uid;
        this->nick_name = nick_name;
        this->stream_name = stream_name;
        this->media_type = media_type;
        this->role = role;
    }
}VideoRoomMemberInfo;

} // namespace bee

#endif // #ifndef __VIDEO_ROOM_DEF_H__
