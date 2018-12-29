/**
 *  @file        bee_video_room.h
 *  @brief       BeeSDK视频会议声明文件.
 *  @author      HeZhen
 *  @copyright   sohu-inc
 *  @version     2.0.0.1
 */

#ifndef __BEE_VIDEO_ROOM_H__
#define __BEE_VIDEO_ROOM_H__

#import <Foundation/Foundation.h>
#import "bee/ios/bee_video_room_sink.h"
#import "bee/ios/bee_audio_source.h"
#import "bee/ios/bee_video_source.h"
#import "bee/ios/bee_video_renderer.h"
#import "bee/ios/bee_sdk_service.h"

/// BeeSDK视频会议类
@interface BeeVideoRoom : BeeSDKService

/**
 *  @brief  视频会议类构造函数.
 *  @param  handle      会话句柄.
 *  @param  token       APP令牌，每个APP必须绑定一个令牌用于鉴权.
 *  @param  timeout     视频会议的接口调用超时时间，单位ms.
 *  @param  sink        视频会议类接口回调对象. 
 *  @return 视频会议类对象.
 *  @see    BeeVideoRoomSink
 */
- (instancetype)initWithParam:(bee_handle)handle
                        token:(NSString*)token
                      timeout:(bee_int32_t)timeout
                         sink:(id<BeeVideoRoomSink>)sink;

/**
 *  @brief  加入视频会议.
 *  @param  roomName    房间名.
 *  @param  uid         本地用户唯一标识.
 *  @param  nickName    本地用户昵称.
 *  @param  creator     是否是会议创建者.
 *  @param  role        会议室角色.
 *  @note   结果通过[BeeVideoRoomSink onJoin:error:msg:]返回，并通过[BeeVideoRoomSink onMembers:]得到成员信息通知.
 *  @see    [BeeVideoRoomSink onJoin:error:msg:]，[BeeVideoRoomSink onMembers:].
 */
- (void)join:(NSString*)roomName
         uid:(NSString*)uid
    nickName:(NSString*)nickName
     creator:(BOOL)creator
        role:(VideoRoomRole)role;

/**
 *  @brief  离开视频会议.
 *  @note   结果通过[BeeVideoRoomSink onLeave:error:msg:]返回.
 *  @see    [BeeVideoRoomSink onLeave:error:msg:].
 */
- (void)leave;

/**
 *  @brief  拉一路流.
 *  @param  uid             远端用户唯一标识.
 *  @param  streamName      流名.
 *  @param  mediaType       拉流的媒体类型.
 *  @param  videoRenderer   显示所拉流的视频渲染器.
 *  @note   结果通过[BeeVideoRoomSink onConnect:streamName:error:msg:]返回.
 *  @see    [BeeVideoRoomSink onConnect:streamName:error:msg:].
 */
- (void)connect:(NSString*)uid
     streamName:(NSString*)streamName
      mediaType:(bee_int32_t)mediaType
  videoRenderer:(BeeVideoRenderer*)videoRenderer;

/**
 *  @brief  断一路流.
 *  @param  uid             远端用户唯一标识.
 *  @param  streamName      流名.
 *  @param  reason          原因(保留).
 *  @note   结果通过[BeeVideoRoomSink onDisconnect:streamName:error:msg:]返回.
 *  @see    [BeeVideoRoomSink onDisconnect:streamName:error:msg:].
 */
- (void)disconnect:(NSString*)uid
        streamName:(NSString*)streamName
            reason:(int)reason;

/**
 *  @brief  设置推流参数.
 *  @param  streamName      推流流名.
 *  @param  mediaType       推流媒体类型.
 *  @param  audioSource     推流音频源，设置为nil将使用内部默认音频源.
 *  @param  videoSource     推流视频源，设置为nil将使用内部默认视频源，数据采集自摄像头，默认前置摄像头.
 *  @param  videoRenderer   推流的本地渲染器.
 *  @param  handler         本方法的调用结果回调Block.
 */
- (void)setupPushStream:(NSString*)streamName
              mediaType:(bee_int32_t)mediaType
            audioSource:(BeeAudioSource*)audioSource
            videoSource:(BeeVideoSource*)videoSource
          videoRenderer:(BeeVideoRenderer*)videoRenderer
                handler:(void(^)(BeeErrorCode ec, NSString *msg))handler;

/**
 *  @brief  获取当前推流的音频输入音量.
 *  @param  streamName      流名.
 *  @note   结果通过[BeeVideoRoomSink onAudioInputLevel:msg:streamName:level:]返回.
 *  @see    [BeeVideoRoomSink onAudioInputLevel:msg:streamName:level:].
 */
- (void)getAudioInputLevel:(NSString*)streamName;

/**
 *  @brief  获取一路流的音频输出音量.
 *  @param  uid             远端用户唯一标识.
 *  @param  streamName      流名.
 *  @note   结果通过[BeeVideoRoomSink onAudioOutputLevel:msg:uid:streamName:level:]返回.
 *  @see    [BeeVideoRoomSink onAudioOutputLevel:msg:uid:streamName:level:].
 */
- (void)getAudioOutputLevel:(NSString*)uid
                 streamName:(NSString*)streamName;

/**
 *  @brief  修改声音输出.
 */
- (void)changeAudioRoute;

/**
 *  @brief  切换前后置摄像头.
 */
- (void)switchCamera;

/**
 *  @brief  使能某个流的状态获取.
 *  @param  uid             用户唯一标识.
 *  @param  streamName      流名.
 *  @param  partyType       会议成员类型.
 *  @param  period          获取状态的时间间隔，单位是ms.
 */
- (void)enableStatsEvents:uid
               streamName:(NSString*)streamName
                partyType:(VideoRoomPartyType)partyType
                   period:(NSTimeInterval)period;

/**
 *  @brief  禁用某个流的状态获取.
 *  @param  uid             用户唯一标识.
 *  @param  streamName      流名.
 *  @param  partyType       会议成员类型.
 */
- (void)disableStatsEvents:uid
                streamName:(NSString*)streamName
                 partyType:(VideoRoomPartyType)partyType;

@end

#endif // #ifndef __BEE_VIDEO_ROOM_H__
