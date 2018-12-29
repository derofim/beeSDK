#import "bee/ios/bee_video_room.h"
#import "bee/ios/bee_sdk.h"
#import "bee/ios/bee_video_source_camera.h"

#import "WebRTC/RTCAudioSession.h"
#import "WebRTC/RTCAudioSessionConfiguration.h"
#import "WebRTC/RTCDispatcher.h"

#include <sstream>

/////////////////////////////////Const/////////////////////////////////////////
const bee_int32_t kBeeIOSDefaultVideoWidth = 640;
const bee_int32_t kBeeIOSDefaultVideoHeight = 480;
const bee_int32_t kBeeIOSDefaultVideoFps = 30;

static BOOL _audioSessionInitialized = NO;

#pragma mark - BeeVideoRoomMemberInfo

@implementation BeeVideoRoomMemberInfo

@synthesize uid = _uid;
@synthesize nickName = _nickName;
@synthesize streamName = _streamName;
@synthesize mediaType = _mediaType;
@synthesize partyType = _partyType;
@synthesize role = _role;

- (instancetype)initWithParam:(NSString*)uid
                     nickName:(NSString*)nickName
                   streamName:(NSString*)streamName
                    mediaType:(bee_int32_t)mediaType
                    partyType:(VideoRoomPartyType)partyType
                         role:(VideoRoomRole)role {
    if (self = [super init]) {
        _uid = uid;
        _nickName = nickName;
        _streamName = streamName;
        _mediaType = mediaType;
        _partyType = partyType;
        _role = role;
    }
    return self;
}

@end

#pragma mark - BeeVideoRoom

@interface BeeVideoRoom () <RTCAudioSessionDelegate>

@end

@implementation BeeVideoRoom {
    //Service must register to a session.
    bee_handle _handle;
    
    //Room name must be unique and published by app.
    NSString *_roomName;
    
    //App should bind token for validating service.
    NSString *_token;
    
    //Local user info.
    BOOL _creator;
    VideoRoomRole _role;
    NSString *_uid;
    NSString *_nickName;
    
    //Push stream info.
    NSString *_streamName;
    bee_int32_t _mediaType;
    BeeAudioSource *_audioSource;
    BeeVideoSource *_videoSource;
    BeeVideoRenderer *_videoRenderer;
    
    bee_int32_t _timeout;
    __weak id<BeeVideoRoomSink> _sink;
    AVAudioSessionPortOverride _portOverride;
}

#pragma mark - Instance method

- (instancetype)initWithParam:(bee_handle)handle
                        token:(NSString*)token
                      timeout:(bee_int32_t)timeout
                         sink:(id<BeeVideoRoomSink>)sink {
    if (self = [super initWithSvcCode:kBeeSvcType_VideoRoom]) {
        _handle = handle;
        _token = token;
        _uid = nil;
        _nickName = nil;
        _timeout = timeout;
        _sink = sink;
        _roomName = nil;
        _creator = YES;
        _role = eVideoRoomRole_None;
        _streamName = nil;
        _mediaType = eVideoRoomMediaType_None;
        _audioSource = nil;
        _videoSource = nil;
        _videoRenderer = nil;
        _portOverride = AVAudioSessionPortOverrideNone;
        if (!_audioSessionInitialized) {
            [self initAudioSession];
            _audioSessionInitialized = YES;
        }
    }
    return self;
}

#pragma mark - Export methods

- (void)join:(NSString*)roomName
         uid:(NSString*)uid
    nickName:(NSString*)nickName
     creator:(BOOL)creator
        role:(VideoRoomRole)role {
    dispatch_async([[BeeSDK sharedInstance] getDispatchQueue], ^{
        [self joinInternal:roomName
                       uid:uid
                  nickName:nickName
                   creator:creator
                      role:role];
    });
}

- (void)leave {
    dispatch_async([[BeeSDK sharedInstance] getDispatchQueue], ^{
        [self leaveInternal];
    });
}

- (void)connect:(NSString*)uid
     streamName:(NSString*)streamName
      mediaType:(bee_int32_t)mediaType
  videoRenderer:(BeeVideoRenderer*)videoRenderer {
    dispatch_async([[BeeSDK sharedInstance] getDispatchQueue], ^{
        [self connectInternal:uid streamName:streamName mediaType:mediaType videoRenderer:videoRenderer];
    });
}

- (void)disconnect:(NSString*)uid
        streamName:(NSString*)streamName
            reason:(int)reason {
    dispatch_async([[BeeSDK sharedInstance] getDispatchQueue], ^{
        [self disconnectInternal:uid streamName:streamName reason:reason];
    });
}

- (void)setupPushStream:(NSString*)streamName
              mediaType:(bee_int32_t)mediaType
            audioSource:(BeeAudioSource*)audioSource
            videoSource:(BeeVideoSource*)videoSource
          videoRenderer:(BeeVideoRenderer*)videoRenderer
                handler:(void(^)(BeeErrorCode ec, NSString *msg))handler{
    dispatch_async([[BeeSDK sharedInstance] getDispatchQueue], ^{
        [self setupPushStreamInternal:streamName
                            mediaType:mediaType
                          audioSource:audioSource
                          videoSource:videoSource
                        videoRenderer:videoRenderer
                              handler:handler];
    });
}

- (void)getAudioInputLevel:(NSString*)streamName {
    dispatch_async([[BeeSDK sharedInstance] getDispatchQueue], ^{
        [self getAudioInputLevelInternal:streamName];
    });
}

- (void)getAudioOutputLevel:(NSString*)uid
                 streamName:(NSString*)streamName {
    dispatch_async([[BeeSDK sharedInstance] getDispatchQueue], ^{
        [self getAudioOutputLevelInternal:uid
                               streamName:streamName];
    });
}

- (void)changeAudioRoute {
    AVAudioSessionPortOverride override = AVAudioSessionPortOverrideNone;
    if (_portOverride == AVAudioSessionPortOverrideNone) {
        override = AVAudioSessionPortOverrideSpeaker;
    }
    
    [RTCDispatcher dispatchAsyncOnType:RTCDispatcherTypeAudioSession
                                 block:^{
                                     RTCAudioSession *session = [RTCAudioSession sharedInstance];
                                     [session lockForConfiguration];
                                     NSError *error = nil;
                                     if ([session overrideOutputAudioPort:override error:&error]) {
                                         _portOverride = override;
                                     } else {
                                         NSLog(@"Error overriding output port: %@", error.localizedDescription);
                                     }
                                     [session unlockForConfiguration];
                                 }];
}

- (void)switchCamera {
    dispatch_async([[BeeSDK sharedInstance] getDispatchQueue], ^{
        [self switchCameraInternal];
    });
}

- (void)enableStatsEvents:uid
               streamName:(NSString*)streamName
                partyType:(VideoRoomPartyType)partyType
                   period:(NSTimeInterval)period {
    
}

- (void)disableStatsEvents:uid
                streamName:(NSString*)streamName
                 partyType:(VideoRoomPartyType)partyType {
    
}

#pragma mark - Internal methods.

- (void)joinInternal:(NSString*)roomName
                 uid:(NSString*)uid
            nickName:(NSString*)nickName
             creator:(BOOL)creator
                role:(VideoRoomRole)role {
    BeeErrorCode ret = kBeeErrorCode_Success;
    BOOL enabledAudioSession = NO;
    do {
        [self enableAudioSession];
        enabledAudioSession = YES;
        
        if (roomName == nil || uid == nil || nickName == nil || _token == nil) {
            ret = kBeeErrorCode_Invalid_Param;
            break;
        }
        
        NSLog(@"Joining video room:%@, creator:%@, role:%d, uid:%@, nick name:%@.", roomName, creator?@"YES":@"NO", role, uid, nickName);
        
        if (![self isRegistered]) {
            ret = [self Register:_handle];
            if (ret != kBeeErrorCode_Success) {
                NSLog(@"[ER] VideoRoom service register failed.");
                break;
            }
        }
        
        _roomName = roomName;
        _uid = uid;
        _nickName = nickName;
        _creator = creator;
        _role = role;
        
        bee_int64_t audioSource = (bee_int64_t)((_audioSource == nil) ? 0 : _audioSource.internalAudioSource.get());
        bee_int64_t videoSource = (bee_int64_t)((_videoSource == nil) ? 0 : _videoSource.internalVideoSource.get());
        bee_int64_t videoRenderer = (bee_int64_t)((_videoRenderer == nil) ? 0 : _videoRenderer.internalVideoRenderer.get());
        
        NSMutableDictionary *dictionary = [[NSMutableDictionary alloc] init];
        [dictionary setValue:_roomName forKey:@"room_name"];
        [dictionary setValue:[NSNumber numberWithBool:_creator] forKey:@"create"];
        [dictionary setValue:_uid forKey:@"uid"];
        [dictionary setValue:_nickName forKey:@"nick_name"];
        [dictionary setValue:_token forKey:@"token"];
        [dictionary setValue:[NSNumber numberWithInt:_role] forKey:@"role"];
        
        if ([self willPushAudio] || [self willPushVideo]) {
            NSMutableDictionary *pushObj = [[NSMutableDictionary alloc] init];
            [dictionary setObject:pushObj forKey:@"push"];
            [pushObj setValue:_streamName forKey:@"stream_name"];
            
            NSMutableDictionary *audioObj = [[NSMutableDictionary alloc] init];
            [pushObj setObject:audioObj forKey:@"audio"];
            [audioObj setValue:[NSNumber numberWithBool:[self willPushAudio]] forKey:@"present"];
            [audioObj setValue:[NSNumber numberWithLongLong:(long long)audioSource] forKey:@"source"];
            
            NSMutableDictionary *videoObj = [[NSMutableDictionary alloc] init];
            [pushObj setObject:videoObj forKey:@"video"];
            [videoObj setValue:[NSNumber numberWithBool:[self willPushVideo]] forKey:@"present"];
            [videoObj setValue:[NSNumber numberWithLongLong:(long long)videoRenderer] forKey:@"renderer"];
            [videoObj setValue:[NSNumber numberWithLongLong:(long long)videoSource] forKey:@"source"];
        }
        
        if (![NSJSONSerialization isValidJSONObject:dictionary]) {
            NSLog(@"Command with error format.");
            break;
        }
        
        NSError *error = nil;
        NSData *jsonData = [NSJSONSerialization dataWithJSONObject:dictionary options:NSJSONWritingPrettyPrinted error:&error];
        if ([jsonData length] <= 0 || error) {
            NSLog(@"Command serialize fail.");
            break;
        }
        
        NSString *args = [[NSString alloc]initWithData:jsonData encoding:NSUTF8StringEncoding];
        NSString *cmd = @"Join";
        
        ret = [self execute:cmd args:args timeout:_timeout];
        if (ret != kBeeErrorCode_Success) {
            break;
        }
    } while (false);
    
    NSLog(@"Join video room %@ return %d.", roomName, ret);
    
    if (ret != kBeeErrorCode_Success) {
        if (_sink != nil) {
            [_sink onJoin:_streamName error:ret msg:[[BeeSDK sharedInstance] errorToString:ret]];
        }
        
        if (enabledAudioSession) {
            [self disableAudioSession];
        }
    }
}

- (void)leaveInternal {
    BeeErrorCode ret = kBeeErrorCode_Success;
    do {
        NSLog(@"Leaving from video room.");
        
        [self disableAudioSession];
        
        if (![self isRegistered]) {
            NSLog(@"[ER] VideoRoom service not registered.");
            ret = kBeeErrorCode_Service_Not_Registered;
            break;
        }

        NSMutableDictionary *dictionary = [[NSMutableDictionary alloc] init];
        [dictionary setValue:_roomName forKey:@"room_name"];
        [dictionary setValue:[NSNumber numberWithInt:0] forKey:@"reason"];
        
        if (![NSJSONSerialization isValidJSONObject:dictionary]) {
            NSLog(@"Command with error format.");
            break;
        }
        
        NSError *error = nil;
        NSData *jsonData = [NSJSONSerialization dataWithJSONObject:dictionary options:NSJSONWritingPrettyPrinted error:&error];
        if ([jsonData length] <= 0 || error) {
            NSLog(@"Command serialize fail.");
            break;
        }
        
        NSString *args = [[NSString alloc]initWithData:jsonData encoding:NSUTF8StringEncoding];
        NSString *cmd = @"Leave";
        
        ret = [self execute:cmd args:args timeout:_timeout];
        if (ret != kBeeErrorCode_Success) {
            NSLog(@"[ER] Leave failed with error %d.", ret);
        }
        
        [self disableAudioSession];
    } while (false);
    
    if ([self isRegistered]) {
        [self unRegister];
    }
    
    NSLog(@"Leave video room %@ return %d.", _roomName, ret);
    
    if (_sink != nil) {
        [_sink onLeave:_streamName error:ret msg:[[BeeSDK sharedInstance] errorToString:ret]];
        _sink = nil;
    }
}

- (void)connectInternal:(NSString*)uid
             streamName:(NSString*)streamName
              mediaType:(bee_int32_t)mediaType
          videoRenderer:(BeeVideoRenderer*)videoRenderer {
    BeeErrorCode ret = kBeeErrorCode_Success;
    do {
        if (uid == nil || streamName == nil || videoRenderer == nil) {
            ret = kBeeErrorCode_Invalid_Param;
            break;
        }
        
        NSLog(@"Connecting stream, uid:%@, stream name:%@, media type:%d.", uid, streamName, mediaType);
        
        if (![self isRegistered]) {
            NSLog(@"[ER] VideoRoom service not registered.");
            ret = kBeeErrorCode_Service_Not_Registered;
            break;
        }
        
        bee_int64_t renderer = (bee_int64_t)videoRenderer.internalVideoRenderer.get();
        
        NSMutableDictionary *dictionary = [[NSMutableDictionary alloc] init];
        [dictionary setValue:_roomName forKey:@"room_name"];
        [dictionary setValue:uid forKey:@"uid"];
        [dictionary setValue:streamName forKey:@"stream_name"];
        [dictionary setValue:[NSNumber numberWithBool:[self willPullAudio:mediaType]] forKey:@"pull_audio"];
        [dictionary setValue:[NSNumber numberWithBool:[self willPullVideo:mediaType]] forKey:@"pull_video"];
        [dictionary setValue:[NSNumber numberWithLongLong:(long long)renderer] forKey:@"renderer"];
        
        if (![NSJSONSerialization isValidJSONObject:dictionary]) {
            NSLog(@"Command with error format.");
            break;
        }
        
        NSError *error = nil;
        NSData *jsonData = [NSJSONSerialization dataWithJSONObject:dictionary options:NSJSONWritingPrettyPrinted error:&error];
        if ([jsonData length] <= 0 || error) {
            NSLog(@"Command serialize fail.");
            break;
        }
        
        NSString *args = [[NSString alloc]initWithData:jsonData encoding:NSUTF8StringEncoding];
        NSString *cmd = @"ConnectStream";
        
        ret = [self execute:cmd args:args timeout:_timeout];
    } while (false);
    
    NSLog(@"Connect stream, uid:%@, stream name:%@ return %d.", uid, streamName, ret);
    
    if (ret != kBeeErrorCode_Success && _sink != nil) {
        [_sink onConnect:uid streamName:streamName error:ret msg:[[BeeSDK sharedInstance] errorToString:ret]];
    }
}

- (void)disconnectInternal:(NSString*)uid
                streamName:(NSString*)streamName
                    reason:(int)reason {
    BeeErrorCode ret = kBeeErrorCode_Success;
    do {
        if (uid == nil || streamName == nil) {
            ret = kBeeErrorCode_Invalid_Param;
            break;
        }
        
        NSLog(@"Disconnecting stream, uid:%@, stream name:%@, reason:%d.", uid, streamName, reason);
        
        if (![self isRegistered]) {
            NSLog(@"[ER] VideoRoom service not registered.");
            ret = kBeeErrorCode_Service_Not_Registered;
            break;
        }
        
        NSMutableDictionary *dictionary = [[NSMutableDictionary alloc] init];
        [dictionary setValue:_roomName forKey:@"room_name"];
        [dictionary setValue:uid forKey:@"uid"];
        [dictionary setValue:streamName forKey:@"stream_name"];
        [dictionary setValue:[NSNumber numberWithInt:reason] forKey:@"reason"];
        
        if (![NSJSONSerialization isValidJSONObject:dictionary]) {
            NSLog(@"Command with error format.");
            break;
        }
        
        NSError *error = nil;
        NSData *jsonData = [NSJSONSerialization dataWithJSONObject:dictionary options:NSJSONWritingPrettyPrinted error:&error];
        if ([jsonData length] <= 0 || error) {
            NSLog(@"Command serialize fail.");
            break;
        }
        
        NSString *args = [[NSString alloc]initWithData:jsonData encoding:NSUTF8StringEncoding];
        NSString *cmd = @"DisconnectStream";
        
        ret = [self execute:cmd args:args timeout:_timeout];
    } while (false);
    
    NSLog(@"Disconnect stream, uid:%@, stream name:%@ return %d.", uid, streamName, ret);
    
    if (ret != kBeeErrorCode_Success && _sink != nil) {
        [_sink onDisconnect:uid streamName:streamName error:ret msg:[[BeeSDK sharedInstance] errorToString:ret]];
    }
}

- (void)setupPushStreamInternal:(NSString*)streamName
                      mediaType:(bee_int32_t)mediaType
                    audioSource:(BeeAudioSource*)audioSource
                    videoSource:(BeeVideoSource*)videoSource
                  videoRenderer:(BeeVideoRenderer*)videoRenderer
                        handler:(void(^)(BeeErrorCode ec, NSString *msg))handler {
    BeeErrorCode ret = kBeeErrorCode_Success;
    do {
        _streamName = streamName;
        _mediaType = mediaType;
        _audioSource = audioSource;
        _videoSource = videoSource;
        _videoRenderer = videoRenderer;
        
        if ([self willPushVideo]) {
            if (videoSource == nil) {
                _videoSource = [[BeeVideoSourceCamera alloc] initWithParam:kBeeIOSDefaultVideoFps
                                                                     width:kBeeIOSDefaultVideoWidth
                                                                    height:kBeeIOSDefaultVideoHeight
                                                                    fixRes:NO];
            } else {
                _videoSource = videoSource;
            }
            
            ret = [_videoSource open];
            if (ret != kBeeErrorCode_Success) {
                break;
            }
        }
        
        if ([self willPushAudio]) {
            if (audioSource == nil) {
                _audioSource = [[BeeAudioSource alloc] initWithParam:YES
                                                          echoCancel:YES
                                                         gainControl:YES
                                                      highPassFilter:YES
                                                    noiceSuppression:YES];
            } else {
                _audioSource = audioSource;
            }
            
            ret = [_audioSource open];
            if (ret != kBeeErrorCode_Success) {
                break;
            }
        }
    } while (false);
    
    if (handler != nil) {
        handler(ret, [[BeeSDK sharedInstance] errorToString:ret]);
    }
}

- (void)getAudioInputLevelInternal:(NSString*)streamName {
    BeeErrorCode ret = kBeeErrorCode_Success;
    do {
        if (streamName == nil) {
            ret = kBeeErrorCode_Invalid_Param;
            break;
        }
        
        if (![self isRegistered]) {
            NSLog(@"[ER] VideoRoom service not registered.");
            ret = kBeeErrorCode_Service_Not_Registered;
            break;
        }
        
        NSMutableDictionary *dictionary = [[NSMutableDictionary alloc] init];
        [dictionary setValue:_roomName forKey:@"room_name"];
        [dictionary setValue:_uid forKey:@"uid"];
        [dictionary setValue:streamName forKey:@"stream_name"];
        
        if (![NSJSONSerialization isValidJSONObject:dictionary]) {
            NSLog(@"Command with error format.");
            break;
        }
        
        NSError *error = nil;
        NSData *jsonData = [NSJSONSerialization dataWithJSONObject:dictionary options:NSJSONWritingPrettyPrinted error:&error];
        if ([jsonData length] <= 0 || error) {
            NSLog(@"Command serialize fail.");
            break;
        }
        
        NSString *args = [[NSString alloc]initWithData:jsonData encoding:NSUTF8StringEncoding];
        NSString *cmd = @"GetAudioInputLevel";
        
        ret = [self execute:cmd args:args timeout:_timeout];
    } while (false);
    
    if (ret != kBeeErrorCode_Success && _sink != nil) {
        [_sink onAudioInputLevel:ret
                             msg:[[BeeSDK sharedInstance] errorToString:ret]
                      streamName:streamName
                           level:-1];
    }
}

- (void)getAudioOutputLevelInternal:(NSString*)uid
                         streamName:(NSString*)streamName {
    BeeErrorCode ret = kBeeErrorCode_Success;
    do {
        if (uid == nil || streamName == nil) {
            ret = kBeeErrorCode_Invalid_Param;
            break;
        }
        
        if (![self isRegistered]) {
            NSLog(@"[ER] VideoRoom service not registered.");
            ret = kBeeErrorCode_Service_Not_Registered;
            break;
        }
        
        NSMutableDictionary *dictionary = [[NSMutableDictionary alloc] init];
        [dictionary setValue:_roomName forKey:@"room_name"];
        [dictionary setValue:_uid forKey:@"uid"];
        [dictionary setValue:streamName forKey:@"stream_name"];
        
        if (![NSJSONSerialization isValidJSONObject:dictionary]) {
            NSLog(@"Command with error format.");
            break;
        }
        
        NSError *error = nil;
        NSData *jsonData = [NSJSONSerialization dataWithJSONObject:dictionary options:NSJSONWritingPrettyPrinted error:&error];
        if ([jsonData length] <= 0 || error) {
            NSLog(@"Command serialize fail.");
            break;
        }
        
        NSString *args = [[NSString alloc]initWithData:jsonData encoding:NSUTF8StringEncoding];
        NSString *cmd = @"GetAudioOutputLevel";
        
        ret = [self execute:cmd args:args timeout:_timeout];
    } while (false);
    
    if (ret != kBeeErrorCode_Success && _sink != nil) {
        [_sink onAudioOutputLevel:ret
                             msg:[[BeeSDK sharedInstance] errorToString:ret]
                              uid:uid
                      streamName:streamName
                           level:-1];
    }
}

- (void)switchCameraInternal {
    if (_videoSource != nil && [_videoSource isKindOfClass:[BeeVideoSourceCamera class]]) {
        BeeVideoSourceCamera *_cameraSource = (BeeVideoSourceCamera*)_videoSource;
        [_cameraSource switchCamera];
    }
}

#pragma mark - Data handler

- (void)handleData:(NSString*)data {
    if (data == nil) {
        NSLog(@"[WA] Video room data nil.");
        return;
    }
    
    NSData *nsData = [data dataUsingEncoding:NSUTF8StringEncoding];
    NSError *nsError = nil;
    id json = [NSJSONSerialization JSONObjectWithData:nsData options:NSJSONReadingAllowFragments error:&nsError];
    if (!json || ![json isKindOfClass:[NSDictionary class]] || nsError) {
        NSLog(@"[ER] Invalid video room data format.");
        return;
    }
    
    NSString *roomName = nil;
    NSString *uid = nil;
    NSString *streamName = nil;
    BeeErrorCode err = kBeeErrorCode_Success;
    NSString *msg = nil;
    VideoRoomMsgType eType = eVideoRoomMsgType_Count;
    
    NSDictionary *dictionary = (NSDictionary*)json;
    NSNumber *nsType = [dictionary objectForKey:@"type"];
    if (nsType != nil) {
        int type = [nsType intValue];
        eType = [self toMessageType:type];
        if (eType == eVideoRoomMsgType_Count) {
            NSLog(@"Got invalid video room notify type %d.", type);
            return;
        }
    } else {
        NSLog(@"[ER] No message type in data.");
        return;
    }
    
    NSNumber *nsEc = [dictionary objectForKey:@"ec"];
    if (nsEc != nil) {
        int ec = [nsEc intValue];
        err = [self toBeeErrorCode:ec];
        if (err == kBeeErrorCode_Not_Compatible) {
            NSLog(@"Invalid error code %d.", ec);
            return;
        }
    }
    
    roomName = [dictionary objectForKey:@"room_name"];
    streamName = [dictionary objectForKey:@"stream_name"];
    uid = [dictionary objectForKey:@"uid"];
    msg = [dictionary objectForKey:@"msg"];
    
    switch (eType) {
        case eVideoRoomMsgType_Local_Party_Join: {
            [self handleLocalMemberJoined:streamName error:err msg:msg];
            break;
        }
        case eVideoRoomMsgType_Remote_Party_Join: {
            [self handleRemoteMemberJoined:uid streamName:streamName error:err msg:msg];
            break;
        }
        case eVideoRoomMsgType_Local_Leave: {
            [self handleLocalMemberLeaved:streamName error:err msg:msg];
            break;
        }
        case eVideoRoomMsgType_Remote_Leave: {
            [self handleRemoteMemberLeaved:uid streamName:streamName error:err msg:msg];
            break;
        }
        case eVideoRoomMsgType_Remote_Members: {
            NSArray *membersJsonArray = [dictionary objectForKey:@"members"];
            [self handleRemoteMembers:membersJsonArray];
            break;
        }
        case eVideoRoomMsgType_Audio_Input_Level: {
            int level = -1;
            NSNumber *nsLevel = [dictionary objectForKey:@"level"];
            if (nsLevel != nil) {
                level = [nsLevel intValue];
            }
            [self handleAudioInputLevel:err msg:msg uid:uid streamName:streamName level:level];
            break;
        }
        case eVideoRoomMsgType_Audio_Output_Level: {
            int level = -1;
            NSNumber *nsLevel = [dictionary objectForKey:@"level"];
            if (nsLevel != nil) {
                level = [nsLevel intValue];
            }
            [self handleAudioOutputLevel:err msg:msg uid:uid streamName:streamName level:level];
            break;
        }
        default:
            break;
    }
}

- (void)handleLocalMemberJoined:(NSString*)streamName
                          error:(BeeErrorCode)error
                            msg:(NSString*)msg {
    dispatch_async([[BeeSDK sharedInstance] getDispatchQueue], ^{
        if (_sink != nil) {
            [_sink onJoin:streamName error:error msg:msg];
        }
    });
}

- (void)handleRemoteMemberJoined:(NSString*)uid
                      streamName:(NSString*)streamName
                           error:(BeeErrorCode)error
                             msg:(NSString*)msg {
    dispatch_async([[BeeSDK sharedInstance] getDispatchQueue], ^{
        if (_sink != nil) {
            [_sink onConnect:uid streamName:streamName error:error msg:msg];
        }
    });
}

- (void)handleLocalMemberLeaved:(NSString*)streamName
                          error:(BeeErrorCode)error
                            msg:(NSString*)msg {
    dispatch_async([[BeeSDK sharedInstance] getDispatchQueue], ^{
        if (_sink != nil) {
            [_sink onLeave:streamName error:error msg:msg];
        }
    });
}

- (void)handleRemoteMemberLeaved:(NSString*)uid
                      streamName:(NSString*)streamName
                           error:(BeeErrorCode)error
                             msg:(NSString*)msg {
    dispatch_async([[BeeSDK sharedInstance] getDispatchQueue], ^{
        if (_sink != nil) {
            [_sink onDisconnect:uid streamName:streamName error:kBeeErrorCode_Success msg:msg];
        }
    });
}

- (void)handleLocalMemberSlowLink:(NSString*)streamName
                             info:(NSString*)info {
    dispatch_async([[BeeSDK sharedInstance] getDispatchQueue], ^{
        if (_sink != nil) {
            [_sink onSlowLink:_uid streamName:streamName partyType:eVideoRoomPartyType_Local info:info];
        }
    });
}

- (void)handleRemoteMemberSlowLink:(NSString*)uid
                        streamName:(NSString*)streamName
                              info:(NSString*)info  {
    dispatch_async([[BeeSDK sharedInstance] getDispatchQueue], ^{
        if (_sink != nil) {
            [_sink onSlowLink:uid streamName:streamName partyType:eVideoRoomPartyType_Remote info:info];
        }
    });
}

- (void)handleRemoteMembers:(NSArray*)membersJsonArray {
    dispatch_async([[BeeSDK sharedInstance] getDispatchQueue], ^{
        if (membersJsonArray == nil) {
            return;
        }
        
        int count = [membersJsonArray count];
        if (count == 0) {
            return;
        }
        
        NSMutableArray *members = [[NSMutableArray alloc] initWithCapacity:count];
        for (NSDictionary *dictionary in membersJsonArray) {
            NSString *uid = nil;
            NSString *nickName = nil;
            NSString *streamName = nil;
            int32_t mediaType = 0;
            VideoRoomRole role = eVideoRoomRole_None;
            
            uid = [dictionary objectForKey:@"uid"];
            if (uid == nil) {
                NSLog(@"[WA] Member info without uid.");
                continue;
            }
            
            nickName = [dictionary objectForKey:@"nick_name"];
            streamName = [dictionary objectForKey:@"stream_name"];
            if (streamName == nil) {
                NSLog(@"[WA] Member info without stream name.");
                continue;
            }
            
            NSNumber *nsMediaType = [dictionary objectForKey:@"media_type"];
            if (nsMediaType != nil) {
                mediaType = [nsMediaType intValue];
            }
            
            NSNumber *nsRole = [dictionary objectForKey:@"role"];
            if (nsRole != nil) {
                role = [self toRole:[nsRole intValue]];
            }            
            
            BeeVideoRoomMemberInfo *memberInfo = [[BeeVideoRoomMemberInfo alloc] initWithParam:uid
                                                                                      nickName:nickName
                                                                                    streamName:streamName
                                                                                     mediaType:mediaType
                                                                                     partyType:eVideoRoomPartyType_Remote
                                                                                          role:role];
            [members addObject:memberInfo];
        }
        
        if (_sink != nil && [members count] > 0) {
            [_sink onMembers:members];
        }
    });
}

- (void)handleAudioInputLevel:(BeeErrorCode)err
                          msg:(NSString*)msg
                          uid:(NSString*)uid
                   streamName:(NSString*)streamName
                        level:(int)level {
    dispatch_async([[BeeSDK sharedInstance] getDispatchQueue], ^{
        if (_sink != nil) {
            [_sink onAudioInputLevel:err
                                 msg:msg
                          streamName:streamName
                               level:level];
        }
    });
}

- (void)handleAudioOutputLevel:(BeeErrorCode)err
                           msg:(NSString*)msg
                           uid:(NSString*)uid
                    streamName:(NSString*)streamName
                         level:(int)level {
    dispatch_async([[BeeSDK sharedInstance] getDispatchQueue], ^{
        if (_sink != nil) {
            [_sink onAudioOutputLevel:err
                                  msg:msg
                                  uid:uid
                           streamName:streamName
                                level:level];
        }
    });
}

#pragma mark - Audio Session

- (void)initAudioSession {
    dispatch_async(dispatch_get_main_queue(), ^{
        RTCAudioSessionConfiguration *webRTCConfig = [RTCAudioSessionConfiguration webRTCConfiguration];
        webRTCConfig.categoryOptions = webRTCConfig.categoryOptions | AVAudioSessionCategoryOptionDefaultToSpeaker;
        [RTCAudioSessionConfiguration setWebRTCConfiguration:webRTCConfig];
        [self configureAudioSession];
    });
}

- (void)configureAudioSession {
    RTCAudioSessionConfiguration *configuration = [[RTCAudioSessionConfiguration alloc] init];
    configuration.category = AVAudioSessionCategoryAmbient;
    configuration.categoryOptions = AVAudioSessionCategoryOptionDuckOthers;
    configuration.mode = AVAudioSessionModeDefault;
    
    RTCAudioSession *session = [RTCAudioSession sharedInstance];
    [session lockForConfiguration];
    BOOL hasSucceeded = NO;
    NSError *error = nil;
    if (session.isActive) {
        hasSucceeded = [session setConfiguration:configuration error:&error];
    } else {
        hasSucceeded = [session setConfiguration:configuration active:YES error:&error];
    }
    if (!hasSucceeded) {
        NSLog(@"Error setting configuration: %@", error.localizedDescription);
    }
    //For speaker.
    [session.session setCategory:AVAudioSessionCategoryPlayback error:nil];
    [session.session setActive:YES error:nil];
    session.useManualAudio = NO;
    [session unlockForConfiguration];
}

- (void)enableAudioSession {
    dispatch_async(dispatch_get_main_queue(), ^{
        RTCAudioSession *session = [RTCAudioSession sharedInstance];
        [session addDelegate:self];
        session.isAudioEnabled = YES;
    });
}

- (void)disableAudioSession {
    dispatch_async(dispatch_get_main_queue(), ^{
        RTCAudioSession *session = [RTCAudioSession sharedInstance];
        [session removeDelegate:self];
        session.isAudioEnabled = NO;
    });
}

#pragma mark - RTCAudioSessionDelegate

- (void)audioSessionDidStartPlayOrRecord:(RTCAudioSession *)session {
    NSLog(@"Audio session started.");
}

- (void)audioSessionDidStopPlayOrRecord:(RTCAudioSession *)session {
    NSLog(@"Audio session stopped.");
}

- (void)audioSession:(RTCAudioSession *)audioSession didDetectPlayoutGlitch:(int64_t)totalNumberOfGlitches {
    if (_sink != nil && [_sink respondsToSelector:@selector(onAudioGlitchDetected:)]) {
        [_sink onAudioGlitchDetected:(bee_int64_t)totalNumberOfGlitches];
    }
}

#pragma mark - Private methods

- (bool)willPushAudio {
    return _mediaType & eVideoRoomMediaType_Audio;
}

- (bool)willPushVideo {
    return _mediaType & eVideoRoomMediaType_Video;
}

- (bool)willPullAudio:(int)mediaType {
    return mediaType & eVideoRoomMediaType_Audio;
}

- (bool)willPullVideo:(int)mediaType {
    return mediaType & eVideoRoomMediaType_Video;
}

- (VideoRoomMsgType)toMessageType:(int)type {
    if (type >= eVideoRoomMsgType_Local_Party_Join && type < eVideoRoomMsgType_Count) {
        return static_cast<VideoRoomMsgType>(type);
    } else {
        return eVideoRoomMsgType_Count;
    }
}

- (BeeErrorCode)toBeeErrorCode:(int)ec {
    if (ec >= kBeeErrorCode_Success && ec < kBeeErrorCode_Count) {
        return static_cast<BeeErrorCode>(ec);
    } else {
        return kBeeErrorCode_Not_Compatible;
    }
}

- (VideoRoomRole)toRole:(int)role {
    if (role > eVideoRoomRole_None && role <= eVideoRoomRole_Party) {
        return static_cast<VideoRoomRole>(role);
    } else {
        return eVideoRoomRole_None;
    }
}

@end
