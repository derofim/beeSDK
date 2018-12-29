/**
 *  @file        BeeVideoRoomDefine.java
 *  @brief       BeeSDK视频会议室通用声明文件.
 *  @author      fangdali
 *  @copyright   sohu-inc
 *  @version     2.0.0.1
 */
package com.sohu.tv.bee;

/// 视频业务定义
public class BeeVideoRoomDefine {
    /// 视频会议业务码.
    public static final int kBeeSvcType_VideoRoom = 1;

    /// 视频会议消息类型.
    public enum VideoRoomMsgType {
        eVideoRoomMsgType_Local_Party_Join,         //!< 本地用户加入并推流结果.
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
    }

    /// 视频会议媒体类型.
    public enum VideoRoomMediaType {
        eBeeVideoRoomMediaType_None(0),             //!< 无.
        eBeeVideoRoomMediaType_Audio(1),            //!< 音频.
        eBeeVideoRoomMediaType_Video(2);            //!< 视频.

        private final int value;

        private VideoRoomMediaType(int value) {
            this.value = value;
        }

        public int getValue() {
            return value;
        }
    }

    /// 视频会议成员角色.
    public enum VideoRoomRole {
        eVideoRoomRole_None,                        //!< 无.
        eVideoRoomRole_Manager,                     //!< 管理者.
        eVideoRoomRole_Party                        //!< 普通参与者.
    }

    /// 视频会议成员类型.
    public enum VideoRoomPartyType {
        eVideoRoomPartyType_Local,                  //!< 本地用户.
        eVideoRoomPartyType_Remote                  //!< 远端用户.
    }

    /// 视频会议成员信息.
    public static class BeeVideoRoomMemberInfo {
        public String uid;                          //!< 用户唯一标识.
        public String nickName;                     //!< 用户昵称.
        public String streamName;                   //!< 用户推流流名.
        public int mediaType;                       //!< 用户推流媒体类型.
        public VideoRoomPartyType partyType;        //!< 成员类型.
        public VideoRoomRole role;                  //!< 用户会议角色.

        /**
         *  @brief  VideoRoomMemberInfo类构造函数.
         *  @param  partyType      成员类型.
         *  @param  uid             用户唯一标识.
         *  @param  nickName       用户昵称.
         *  @param  streamName     用户推流流名.
         *  @param  mediaType      用户推流媒体类型.
         *  @param  role            用户会议角色.
         */
        public BeeVideoRoomMemberInfo(String uid, String nickName, String streamName, int mediaType, VideoRoomPartyType partyType, VideoRoomRole role) {
            this.uid = uid;
            this.nickName = nickName;
            this.streamName = streamName;
            this.mediaType = mediaType;
            this.partyType = partyType;
            this.role = role;
        }
    }


}
