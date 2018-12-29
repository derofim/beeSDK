/**
 *  @file        bee_video_room_sink.h
 *  @brief       BeeSDK视频会议回调声明文件.
 *  @author      HeZhen
 *  @copyright   sohu-inc
 *  @version     2.0.0.1
 */

#ifndef __BEE_VIDEO_ROOM_SINK_H__
#define __BEE_VIDEO_ROOM_SINK_H__

#import <Foundation/Foundation.h>
#include "bee/base/bee_define.h"
#include "bee/win/video_room_def.h"

using namespace bee;

#pragma mark - BeeVideoRoomMemberInfo

/// BeeSDK视频会议成员信息类.
@interface BeeVideoRoomMemberInfo : NSObject

/// 用户唯一标识.
@property(nonatomic, copy) NSString *uid;

/// 用户昵称.
@property(nonatomic, copy) NSString *nickName;

/// 原始流名.
@property(nonatomic, copy) NSString *streamName;

/// 流的媒体类型.
@property(nonatomic, assign) bee_int32_t mediaType;

/// 会议成员的类型.
@property(nonatomic, assign) VideoRoomPartyType partyType;

/// 会议成员的角色.
@property(nonatomic, assign) VideoRoomRole role;

/**
 *  @brief  BeeSDK会议室成员对象构造函数.
 *  @param  uid             用户唯一标识.
 *  @param  nickName        用户昵称.
 *  @param  streamName      原始流名.
 *  @param  mediaType       流中包含的媒体类型.
 *  @param  partyType       会议成员的类型.
 *  @param  role            会议成员的角色.
 *  @return BeeSDK会议室成员对象.
 */
- (instancetype)initWithParam:(NSString*)uid
                     nickName:(NSString*)nickName
                   streamName:(NSString*)streamName
                    mediaType:(bee_int32_t)mediaType
                    partyType:(VideoRoomPartyType)partyType
                         role:(VideoRoomRole)role;
@end

#pragma mark - BeeVideoRoomSink

/// BeeSDK视频会议回调协议，APP必须实现required部分的回调以获得基本的会议室通知.
@protocol BeeVideoRoomSink <NSObject>

@required

/**
 *  @brief  加入会议室结果回调.
 *  @param  streamName      原始流名.
 *  @param  error           错误码.
 *  @param  msg             错误描述.
 */
- (void)onJoin:(NSString*)streamName
         error:(BeeErrorCode)error
           msg:(NSString*)msg;

/**
 *  @brief  离开会议室结果回调.
 *  @param  streamName      原始流名.
 *  @param  error           错误码.
 *  @param  msg             错误描述.
 */
- (void)onLeave:(NSString*)streamName
          error:(BeeErrorCode)error
            msg:(NSString*)msg;

/**
 *  @brief  新加入会议室的成员信息回调.
 *  @param  remoteMembers   成员信息列表.
 */
- (void)onMembers:(NSArray<BeeVideoRoomMemberInfo*>*)remoteMembers;

/**
 *  @brief  连接(拉流)的结果回调.
 *  @param  uid             用户唯一标识.
 *  @param  streamName      原始流名.
 *  @param  error           错误码.
 *  @param  msg             错误描述.
 */
- (void)onConnect:(NSString*)uid
       streamName:(NSString*)streamName
            error:(BeeErrorCode)error
              msg:(NSString*)msg;

/**
 *  @brief  断开(断流)的结果回调.
 *  @param  uid             用户唯一标识.
 *  @param  streamName      原始流名.
 *  @param  error           错误码.
 *  @param  msg             错误描述.
 */
- (void)onDisconnect:(NSString*)uid
          streamName:(NSString*)streamName
               error:(BeeErrorCode)error
                 msg:(NSString*)msg;

/**
 *  @brief  获取输入音量结果回调.
 *  @param  error           错误码.
 *  @param  msg             错误描述.
 *  @param  streamName      原始流名.
 *  @param  level           音量.
 */
- (void)onAudioInputLevel:(BeeErrorCode)error
                      msg:(NSString*)msg
               streamName:(NSString*)streamName
                    level:(int)level;

/**
 *  @brief  获取某个流输出音量结果回调.
 *  @param  error           错误码.
 *  @param  msg             错误描述.
 *  @param  uid             用户唯一标识.
 *  @param  streamName      原始流名.
 *  @param  level           音量.
 */
- (void)onAudioOutputLevel:(BeeErrorCode)error
                       msg:(NSString*)msg
                       uid:(NSString*)uid
                streamName:(NSString*)streamName
                     level:(int)level;

@optional

/**
 *  @brief  流慢速通知回调，在网络恶化时调用.
 *  @param  uid             用户唯一标识.
 *  @param  streamName      原始流名.
 *  @param  partyType       会议成员的类型.
 *  @param  info            具体信息.
 */
- (void)onSlowLink:(NSString*)uid
        streamName:(NSString*)streamName
         partyType:(VideoRoomPartyType)partyType
              info:(NSString*)info;

/**
 *  @brief  音频差错统计回调.
 *  @param  totalNumberOfGlitches   音频差错总次数.
 */
- (void)onAudioGlitchDetected:(bee_int64_t)totalNumberOfGlitches;

@end

#endif // #ifndef __BEE_VIDEO_ROOM_SINK_H__
