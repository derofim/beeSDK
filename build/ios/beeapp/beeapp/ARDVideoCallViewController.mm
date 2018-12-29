/*
 *  Copyright 2015 The WebRTC Project Authors. All rights reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#import "ARDVideoCallViewController.h"
#import "ARDVideoCallView.h"
#import "ARDSettingsModel.h"
#import "bee/ios/bee_sdk.h"
#import "bee/ios/bee_video_room.h"
#import "bee/ios/bee_video_source_camera.h"

//////////////////////////////////User////////////////////////////////////
#pragma mark - User

typedef enum UserState {
    eUserState_Idle,
    eUserState_Connecting,
    eUserState_Connected,
    eUserState_DisConnecting
}UserState;

@interface User : NSObject
@property(nonatomic) BeeVideoRoom *beeVideoRoom;
@property(nonatomic, copy) NSString *uid;
@property(nonatomic, copy) NSString *uName;
@property(nonatomic, copy) NSString *streamName;
@property(nonatomic, copy) NSString *extraData;
@property(nonatomic, assign) bee_int32_t mediaType;
@property(nonatomic, assign) VideoRoomPartyType partyType;
@property(nonatomic, assign) UserState userState;
@property(nonatomic) BeeVideoRenderer *videoRenderer;
@end

@implementation User
@synthesize beeVideoRoom = _beeVideoRoom;
@synthesize uid = _uid;
@synthesize uName = _uName;
@synthesize streamName = _streamName;
@synthesize extraData = _extraData;
@synthesize mediaType = _mediaType;
@synthesize partyType = _partyType;
@synthesize userState = _userState;
@synthesize videoRenderer = _videoRenderer;

- (void)connect {
    if (_beeVideoRoom != nil) {
        [_beeVideoRoom connect:_uid
                    streamName:_streamName
                     mediaType:_mediaType
                 videoRenderer:_videoRenderer];
        _userState = eUserState_Connecting;
    }
}
    
- (void)disconnect {
    if (_beeVideoRoom != nil) {
        [_beeVideoRoom disconnect:_uid
                       streamName:_streamName
                           reason:0];
        _userState = eUserState_DisConnecting;
    }
}
    
- (void)onConnected {
    _userState = eUserState_Connected;
}
    
- (void)onDisConnected {
    _userState = eUserState_Idle;
}

- (BOOL)hasVideo {
    return (_mediaType * eVideoRoomMediaType_Video)?YES:NO;
}

- (BOOL)hasAudio {
    return (_mediaType * eVideoRoomMediaType_Audio)?YES:NO;
}

@end

//////////////////////////////////BeeTimerProxy////////////////////////////////////
#pragma mark - ARDVideoCallViewController

@interface ARDVideoCallViewController () <BeeVideoRoomSink, ARDVideoCallViewDelegate>

@property(nonatomic, assign) BeePlatformType platformType;
@property(nonatomic, copy) NSString *appName;
@property(nonatomic, copy) NSString *appVersion;
@property(nonatomic, copy) NSString *systemInfo;
@property(nonatomic, copy) NSString *machineCode;
@property(nonatomic, copy) NSString *logPath;
@property(nonatomic, assign) BeeLogLevel logLevel;
@property(nonatomic, assign) bee_int32_t logMaxLine;
@property(nonatomic, assign) bee_int32_t logVolumeCount;
@property(nonatomic, assign) bee_int32_t logVolumeSize;
@property(nonatomic, assign) bee_uint32_t sessionCount;
@property(nonatomic, assign) bool enableStatusd;
@property(nonatomic, copy) NSString *roomId;
@property(nonatomic, copy) NSString *uid;
@property(nonatomic, copy) NSString *uName;
@property(nonatomic, copy) NSString *token;
@property(nonatomic, copy) NSString *streamName;
@property(nonatomic, assign) int mediaType;
@property(nonatomic, copy) NSString *extraData;
@property(nonatomic, readonly) ARDVideoCallView *videoCallView;

@end

@implementation ARDVideoCallViewController {
    BeeVideoRoom *_beeVideoRoom;
    AVAudioSessionPortOverride _portOverride;
    BOOL _beeInitialized;
    BOOL _statsDisabled;
    NSTimeInterval STAT_CALLBACK_PERIOD;
    NSMutableDictionary *_userTable;
    ARDSettingsModel *_settingsModel;
    BOOL _createRoom;
    bee_handle _handle;    
}

@synthesize platformType = _platformType;
@synthesize appName = _appName;
@synthesize appVersion = _appVersion;
@synthesize systemInfo = _systemInfo;
@synthesize machineCode = _machineCode;
@synthesize logPath = _logPath;
@synthesize logLevel = _logLevel;
@synthesize logMaxLine = _logMaxLine;
@synthesize logVolumeCount = _logVolumeCount;
@synthesize logVolumeSize = _logVolumeSize;
@synthesize sessionCount = _sessionCount;
@synthesize enableStatusd  = _enableStatusd;
@synthesize roomId = _roomId;
@synthesize uid = _uid;
@synthesize uName = _uName;
@synthesize token = _token;
@synthesize streamName = _streamName;
@synthesize mediaType = _mediaType;
@synthesize extraData = _extraData;
@synthesize videoCallView = _videoCallView;
@synthesize delegate = _delegate;

- (instancetype)initForRoom:(NSString *)room
                       name:(NSString*)name
                     create:(BOOL)create
                   delegate:(id<ARDVideoCallViewControllerDelegate>)delegate {
  if (self = [super init]) {
    _delegate = delegate;
    _settingsModel = [[ARDSettingsModel alloc] init];
    _platformType = kPlatformType_IPhone;
    _appName = @"BeeDemo";
    _appVersion = @"1.0";
    _systemInfo = @"ios11.3";
    _machineCode = [[NSUUID UUID] UUIDString]; //[_settingsModel currentUidSettingFromStore];
    _logPath = [self getLogPath];
    _logLevel = kLogLevel_Debug;
    _logMaxLine = 0;
    _logVolumeCount = 5;
    _logVolumeSize = 1024;
    _sessionCount = 16;
    _enableStatusd = false;
    _roomId = room;
    _uid = [[NSUUID UUID] UUIDString];;
    _uName = name;
    _mediaType = eVideoRoomMediaType_Audio | eVideoRoomMediaType_Video;
    _token = @"5eb73bb4918354cee213903c3940c0e6183f289d";
    _streamName = [[NSUUID UUID] UUIDString];//随机字符串
    _beeInitialized = NO;
    _statsDisabled = YES;
    STAT_CALLBACK_PERIOD = 1.0f;
    _createRoom = create;
    _handle = -1;
    _userTable = [[NSMutableDictionary alloc] init];
  }
  return self;
}

- (void)loadView {
    _videoCallView = [[ARDVideoCallView alloc] initWithFrame:CGRectZero];
    _videoCallView.delegate = self;
    self.view = _videoCallView;
    
    BeeSDKParam *param = [[BeeSDKParam alloc] initWithParam:_platformType
                                                    appName:_appName
                                                 appVersion:_appVersion
                                                 systemInfo:_systemInfo
                                                machineCode:_machineCode
                                                    logPath:_logPath
                                                   logLevel:_logLevel
                                                 logMaxLine:_logMaxLine
                                             logVolumeCount:_logVolumeCount
                                              logVolumeSize:_logVolumeSize
                                               sessionCount:_sessionCount
                                              enableStatusd:_enableStatusd];
    
    [[BeeSDK sharedInstance] initialize:param timeout:20000 sink:nil handler:^(BeeErrorCode ec1) {
        NSLog(@"BeeSDK init result %d.", ec1);
        if(ec1 != kBeeErrorCode_Success) {
            return;
        }
        
        [[BeeSDK sharedInstance] openSession:^(BeeErrorCode ec2, bee_handle handle, NSArray<BeeSDKCapability *> *capabilities) {
            NSLog(@"BeeSDK open session result %d.", ec2);
            if (ec2 != kBeeErrorCode_Success) {
                return;
            }
            
            _handle = handle;
            _beeVideoRoom = [[BeeVideoRoom alloc] initWithParam:_handle
                                                             token:_token
                                                           timeout:20000
                                                              sink:self];
            int fps = [[_settingsModel currentFpsSettingFromStore] intValue];
            int width = [_settingsModel currentVideoResolutionWidthFromStore];
            int height = [_settingsModel currentVideoResolutionHeightFromStore];
            
            BeeVideoSourceCamera *videoSource = [[BeeVideoSourceCamera alloc] initWithParam:fps
                                                                                      width:width
                                                                                     height:height
                                                                                     fixRes:NO];
            dispatch_async(dispatch_get_main_queue(), ^{
                if (_videoCallView.localVideoView != nil) {
                    _videoCallView.localVideoView.mirror = [videoSource isFrontCamera];
                }
            });

            [_beeVideoRoom setupPushStream:_uid
                                 mediaType:_mediaType
                               audioSource:nil
                               videoSource:videoSource
                             videoRenderer:_videoCallView.localVideoView
                                   handler:^(BeeErrorCode ec3, NSString *msg) {
                                       NSLog(@"setupPushStream result %d.", ec3);
                                       if (ec3 != kBeeErrorCode_Success) {
                                           return;
                                       }
                                       
                                       [_beeVideoRoom join:_roomId
                                                       uid:_uid
                                                  nickName:_uName
                                                   creator:_createRoom
                                                      role:_createRoom?eVideoRoomRole_Manager:eVideoRoomRole_Party];
                                   }];
        }];
        
    }];
}

- (UIInterfaceOrientationMask)supportedInterfaceOrientations {
  return UIInterfaceOrientationMaskAll;
}
                  
- (NSString*)getLogPath {
  NSArray *paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
  NSString *documentDirectory = [paths objectAtIndex:0];
  return documentDirectory;
}

#pragma mark - ARDVideoCallViewDelegate

- (void)videoCallViewDidHangup:(ARDVideoCallView *)view {
  [self hangup];
}

- (void)videoCallViewDidSwitchCamera:(ARDVideoCallView *)view {
  // TODO(tkchin): Rate limit this so you can't tap continously on it.
  // Probably through an animation.
  if (_beeVideoRoom != nil) {
      [_beeVideoRoom switchCamera];
      if (_videoCallView.localVideoView != nil) {
          _videoCallView.localVideoView.mirror = !_videoCallView.localVideoView.mirror;
      }
  }
}

- (void)videoCallViewDidChangeRoute:(ARDVideoCallView *)view {
    if (_beeVideoRoom != nil) {
        [_beeVideoRoom changeAudioRoute];
    }
}

- (void)videoCallViewDidEnableStats:(ARDVideoCallView *)view {
  _statsDisabled = !_statsDisabled;
  if (!_statsDisabled) {
      if (_beeVideoRoom) {
          [_beeVideoRoom enableStatsEvents:nil
                                streamName:nil
                                 partyType:eVideoRoomPartyType_Local
                                    period:STAT_CALLBACK_PERIOD];
      }
  } else {
      if (_beeVideoRoom) {
          [_beeVideoRoom disableStatsEvents:nil
                                 streamName:nil
                                  partyType:eVideoRoomPartyType_Local];
      }
  }
  _videoCallView.statsView.hidden = !_videoCallView.statsView.hidden;
}

#pragma mark - Private
- (void)hangup {
  //_videoCallView.localVideoView.captureSession = nil;
  [_beeVideoRoom leave];
  [_delegate viewControllerDidFinish:self];
}

- (void)showAlertWithMessage:(NSString*)message {
  UIAlertController *alert =
      [UIAlertController alertControllerWithTitle:nil
                                          message:message
                                   preferredStyle:UIAlertControllerStyleAlert];

  UIAlertAction *defaultAction = [UIAlertAction actionWithTitle:@"OK"
                                                          style:UIAlertActionStyleDefault
                                                        handler:^(UIAlertAction *action){
                                                        }];

  [alert addAction:defaultAction];
  [self presentViewController:alert animated:YES completion:nil];
}

#pragma mark - BeeVideoRoomSink

- (void)onJoin:(NSString*)streamName
         error:(BeeErrorCode)error
           msg:(NSString*)msg {
    dispatch_async(dispatch_get_main_queue(), ^{
        NSLog(@"Video room onJoin error:%d.", error);
    });
}

- (void)onLeave:(NSString*)streamName
          error:(BeeErrorCode)error
            msg:(NSString*)msg {
    dispatch_async(dispatch_get_main_queue(), ^{
        NSLog(@"Video room onLeave error:%d.", error);
        [[BeeSDK sharedInstance] closeSession:_handle handler:^(BeeErrorCode ec) {
            NSLog(@"Session closed %d error:%d.", _handle, ec);
            _handle = -1;
        }];
    });
}

- (void)onMembers:(NSArray<BeeVideoRoomMemberInfo*>*)remoteMembers {
    dispatch_async(dispatch_get_main_queue(), ^{
        NSLog(@"Report %lu members.", (unsigned long)[remoteMembers count]);
        for (BeeVideoRoomMemberInfo *memberInfo in remoteMembers) {
            if (1) { //memberInfo.svcType == eBeeSvcType_Media) {
                [self createUser:memberInfo];
            }
        }
    });
}

- (void)onConnect:(NSString*)uid
       streamName:(NSString*)streamName
            error:(BeeErrorCode)error
              msg:(NSString*)msg {
    dispatch_async(dispatch_get_main_queue(), ^{
        NSLog(@"User %@ connect result %d.", streamName, error);
        User *user = [_userTable objectForKey:streamName];
        if (user != nil) {
            if (error == kBeeErrorCode_Success) {
                if (user.hasVideo && user.videoRenderer != nil && _videoCallView != nil) {
                    [_videoCallView onRendererStarted:user.videoRenderer];
                }
                [user onConnected];
            } else {
                if ([user hasVideo] && user.videoRenderer != nil && _videoCallView != nil) {
                    [_videoCallView onRendererStopped:user.videoRenderer];
                    [self recycleVideoRenderer:user.videoRenderer];
                }

                [_userTable removeObjectForKey:streamName];
                [user onDisConnected];
            }
        }
    });
}

- (void)onDisconnect:(NSString*)uid
          streamName:(NSString*)streamName
               error:(BeeErrorCode)error
                 msg:(NSString*)msg {
    dispatch_async(dispatch_get_main_queue(), ^{
        NSLog(@"User %@ disconnect result %d.", streamName, error);
        User *user = [_userTable objectForKey:streamName];
        if (user != nil) {
            [user onDisConnected];
            if ([user hasVideo] && user.videoRenderer != nil && _videoCallView != nil) {
                [_videoCallView onRendererStopped:user.videoRenderer];
                [self recycleVideoRenderer:user.videoRenderer];
            }
            
            [_userTable removeObjectForKey:streamName];
        }
    });
}

- (void)onSetupPushStream:(BeeErrorCode)error
                      msg:(NSString*)msg {
    
}

- (void)onAudioInputLevel:(BeeErrorCode)error
                      msg:(NSString*)msg
               streamName:(NSString*)streamName
                    level:(int)level {
    
}

- (void)onAudioOutputLevel:(BeeErrorCode)error
                       msg:(NSString*)msg
                       uid:(NSString*)uid
                streamName:(NSString*)streamName
                     level:(int)level {
    
}

- (void)onSlowLink:(NSString*)uid
        streamName:(NSString*)streamName
         partyType:(VideoRoomPartyType)partyType
              info:(NSString*)info {
    /*
     dispatch_async(dispatch_get_main_queue(), ^{
     NSString *message = [NSString stringWithFormat:@"Slow link in stream:%@ info:%@.", streamName, info];
     UIAlertView *toast = [[UIAlertView alloc] initWithTitle:nil
     message:message
     delegate:nil
     cancelButtonTitle:nil
     otherButtonTitles:nil, nil];
     [toast show];
     int duration = 2; // duration in seconds
     dispatch_after(dispatch_time(DISPATCH_TIME_NOW, duration * NSEC_PER_SEC), dispatch_get_main_queue(), ^{
     [toast dismissWithClickedButtonIndex:0 animated:YES];
     });
     });
     */
}

- (void)onAudioGlitchDetected:(bee_int64_t)totalNumberOfGlitches {
    
}

/*
- (void)onStats:(NSString*)uid
     streanName:(NSString*)streamName
      partyType:(VideoRoomPartyType)partyType
          stats:(NSArray<RTCLegacyStatsReport *> *)stats {
    dispatch_async(dispatch_get_main_queue(), ^{
        int index = -2;
        
        //Only show stats of user with video in main place now.
        if (partyType == ePartyType_Local) {
            if (_mediaType & eMediaType_Video) {
                index = -1;
            }
        } else {
            User *user = [_userTable objectForKey:streamName];
            if (user != nil && user.hasVideo && user.videoRendererWrapper != nil) {
                index = user.videoRendererWrapper.remoteIndex;
            }
        }
        
        if (_videoCallView != nil) {
            [_videoCallView onStats:index stats:stats];
        }
    });
}
*/

#pragma mark - BeeWhiteBoardSink

- (void)onJoin:(BeeErrorCode)error
           msg:(NSString*)msg {
    dispatch_async(dispatch_get_main_queue(), ^{
        NSLog(@"White board onJoin error:%d.", error);
    });
}

- (void)onLeave:(BeeErrorCode)error
            msg:(NSString*)msg {
    dispatch_async(dispatch_get_main_queue(), ^{
        NSLog(@"White board onLeave error:%d.", error);
    });
}

#pragma mark - Private

- (void)createUser:(BeeVideoRoomMemberInfo*)memberInfo {
    if (memberInfo == nil) {
        return;
    }
    
    if ([_userTable objectForKey:memberInfo.streamName]) {
        return;
    }
    
    User *user = [[User alloc] init];
    user.beeVideoRoom = _beeVideoRoom;
    user.uid = memberInfo.uid;
    user.uName = memberInfo.nickName;
    user.streamName = memberInfo.streamName;
    user.partyType = memberInfo.partyType;
    user.mediaType = memberInfo.mediaType;
    user.userState = eUserState_Idle;
    
    if (user.hasVideo) {
        BeeVideoRenderer *videoRenderer = [self applyVideoRenderer];
        if (videoRenderer != nil) {
            user.videoRenderer = videoRenderer;
        }
    }
    
    [_userTable setObject:user forKey:user.streamName];
    
    //TBD, add app logic to decide whether to pull video/audio.
    [user connect];
}

- (BeeVideoRenderer*)applyVideoRenderer {
    if (_videoCallView != nil) {
        return [_videoCallView applyVideoRenderer];
    } else {
        return nil;
    }
}

- (void)recycleVideoRenderer:(BeeVideoRenderer*)videoRenderer{
    if (_videoCallView != nil) {
        return [_videoCallView recycleVideoRenderer:videoRenderer];
    }
}

@end
