/**
 *  @file        BeeVideoRoomSink.java
 *  @brief       BeeSDK视频会议回调声明文件.
 *  @author      fangdali
 *  @copyright   sohu-inc
 *  @version     2.0.0.1
 */
package com.sohu.tv.bee;

import com.sohu.tv.bee.BeeSystemDefine.BeeErrorCode;

/// BeeSDK视频会议回调协议，APP必须实现@required回调以获得基本的会议室通知.
public interface BeeVideoRoomSink {
    /**
     *  @brief  加入会议室结果回调.
     *  @param  streamName      原始流名.
     *  @param  error           错误码.
     *  @param  msg             错误描述.
     */
    void onJoin(String streamName, BeeErrorCode error, String msg);

    /**
     *  @brief  离开会议室结果回调.
     *  @param  streamName      原始流名.
     *  @param  error           错误码.
     *  @param  msg             错误描述.
     */
    void onLeave(String streamName, BeeErrorCode error, String msg);

    /**
     *  @brief  新加入会议室的成员信息回调.
     *  @param  remoteMembers   成员信息列表.
     */
    void onMembers(BeeVideoRoomDefine.BeeVideoRoomMemberInfo[] remoteMembers);

    /**
     *  @brief  连接(拉流)的结果回调.
     *  @param  uid             用户唯一标识.
     *  @param  streamName      原始流名.
     *  @param  error           错误码.
     *  @param  msg             错误描述.
     */
    void onConnect(String uid, String streamName, BeeErrorCode error, String msg);

    /**
     *  @brief  断开(断流)的结果回调.
     *  @param  uid             用户唯一标识.
     *  @param  streamName      原始流名.
     *  @param  error           错误码.
     *  @param  msg             错误描述.
     */
    void onDisConnect(String uid, String streamName, BeeErrorCode error, String msg);

    /**
     *  @brief  获取输入音量结果回调.
     *  @param  error           错误码.
     *  @param  msg             错误描述.
     *  @param  streamName      原始流名.
     *  @param  level           音量.
     */
    void onAudioInputLevel(BeeErrorCode error, String msg, String streamName, int level);

    /**
     *  @brief  获取某个流输出音量结果回调.
     *  @param  error           错误码.
     *  @param  msg             错误描述.
     *  @param  uid             用户唯一标识.
     *  @param  streamName      原始流名.
     *  @param  level           音量.
     */
    void onAudioOutputLevel(BeeErrorCode error, String msg, String uid, String streamName, int level);

    /**
     *  @brief  流慢速通知回调，在网络恶化时调用.
     *  @param  uid             用户唯一标识.
     *  @param  streamName      原始流名.
     *  @param  partyType       会议成员的类型.
     *  @param  info            具体信息.
     */
    void onSlowLink(String uid, String streamName, BeeVideoRoomDefine.VideoRoomPartyType partyType, String info);

    /**
     * @brief   通知当前网络是非wifi网络.
     * @return  当前非wifi网络是否可以连接.
     */
    boolean onNotWifiConnect();
}
