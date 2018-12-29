#import "WhiteBoardViewController.h"
#import "bee/ios/bee_sdk.h"
#import "bee/ios/bee_video_room.h"
#import "bee/ios/bee_white_board.h"
#import "bee/ios/bee_white_board_view.h"

#pragma mark - User

@interface User : NSObject
@property(nonatomic, copy) NSString *uid;
@property(nonatomic, copy) NSString *nickName;
@property(nonatomic, assign) BOOL creator;
@property(nonatomic, copy) NSString *streamName;
@property(nonatomic, assign) bee_int32_t mediaType;
@property(nonatomic, assign) VideoRoomPartyType partyType;
@property(nonatomic, strong) BeeVideoRenderer *videoRenderer;
@property(nonatomic, assign) BeeWhiteBoardRole role;
@end

@implementation User
@synthesize uid = _uid;
@synthesize nickName = _nickName;
@synthesize creator = _creator;
@synthesize streamName = _streamName;
@synthesize mediaType = _mediaType;
@synthesize partyType = _partyType;
@synthesize videoRenderer = _videoRenderer;
@synthesize role = _role;
@end

#pragma mark - ViewController

@interface WhiteBoardViewController () <BeeVideoRoomSink, BeeWhiteBoardSink, BeeVideoRendererDelegate>
@property(nonatomic, assign) BeePlatformType platformType;
@property(nonatomic, assign) BeeNetType netType;
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
@property(nonatomic, copy) NSString *nickName;
@property(nonatomic, copy) NSString *token;
@property(nonatomic, copy) NSString *streamName;
@property(nonatomic, assign) int mediaType;
@property(nonatomic, assign) BOOL creator;
@end

@implementation WhiteBoardViewController {
    BeeVideoRoom *_beeVideoRoom;
    BeeWhiteBoard *_beeWhiteBoard;
    UIView *_whiteBoardContainer;
    NSMutableDictionary *_userTable;
    BeeVideoRenderer *_videoRenderer;
    CGSize _mainVideoSize;
    bee_handle _handle;
    NSArray *_colors;
    CGFloat _pptTop;
    CGFloat _pptLeft;
    CGFloat _pptWidth;
    CGFloat _pptHeight;
    CGFloat _videoLeft;
    CGFloat _videoTop;
    CGFloat _videoWidth;
    CGFloat _videoHeight;
    int _myDrawColorRGB;
    int _currentColorIndex;
    CGFloat _buttonSpacing;
    UIWebView *_gifView;
    UIButton *_colorButton;
    UIButton *_penButton;
    UIButton *_eraserButton;
    UIButton *_undoButton;
    UIButton *_redoButton;
    UIButton *_clearButton;
    UIButton *_backButton;
    UIButton *_lockButton;
    UIButton *_unlockButton;
    UIButton *_resetButton;
    BOOL _videoRoomLeaved;
    BOOL _whiteBoardLeaved;
}

@synthesize platformType = _platformType;
@synthesize netType = _netType;
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
@synthesize nickName = _nickName;
@synthesize token = _token;
@synthesize streamName = _streamName;
@synthesize mediaType = _mediaType;
@synthesize delegate = _delegate;
@synthesize creator = _creator;

- (instancetype)initForRoom:(NSString *)room
                     create:(BOOL)create
                   delegate:(id<WhiteBoardViewControllerDelegate>)delegate {
    if (self = [super init]) {
        //Properties
        _delegate = delegate;
        _platformType = kPlatformType_IPhone;
        _netType = kNetType_Wifi;
        _appName = @"WhiteBoardTest";
        _appVersion = @"1.0";
        _systemInfo = @"ios 11.3";
        _machineCode = [[NSUUID UUID] UUIDString];
        _logPath = [self getLogPath];
        _logLevel = kLogLevel_Debug;
        _logMaxLine = 0;
        _logVolumeCount = 0;
        _logVolumeSize = 0;
        _sessionCount = 16;
        _roomId = room;
        _uid = [[NSUUID UUID] UUIDString];
        _nickName = @"HeZhen";
        _token = @"5eb73bb4918354cee213903c3940c0e6183f289d";
        _streamName = [[NSUUID UUID] UUIDString];
        _mediaType = eVideoRoomMediaType_Audio | eVideoRoomMediaType_Video;
        _creator = create;
        _videoRoomLeaved = NO;
        _whiteBoardLeaved = NO;
        
        //Private members.
        _userTable = [[NSMutableDictionary alloc] init];
        _colors = @[@(0x000000), @(0xd1021c), @(0xfddc01), @(0x7dd21f), @(0x228bf7), @(0x9b0df5)];
        _currentColorIndex = 4;
        _myDrawColorRGB = [_colors[_currentColorIndex] intValue];
        _buttonSpacing = 20;
        
        CGRect screenRect = self.view.frame;
        _pptTop = 10;
        _pptLeft = 30;
        _pptHeight = screenRect.size.height - 70;
        _pptWidth = (CGFloat)_pptHeight * 728 / 520;
    }
    return self;
}

- (void)viewDidLoad {
    [super viewDidLoad];
    
    [self.view setBackgroundColor:[UIColor whiteColor]];
    
    _videoRenderer = [[BeeVideoRenderer alloc] init];
    _videoRenderer.delegate = self;
    [self.view addSubview:_videoRenderer.view];
    
    _gifView = [[UIWebView alloc] initWithFrame:CGRectZero];
    NSString *path = [[NSBundle mainBundle] pathForResource:@"ads" ofType:@"gif"];
    NSURL *url = [NSURL URLWithString:path];
    [_gifView loadRequest:[NSURLRequest requestWithURL:url]];
    [_gifView setScalesPageToFit:YES];
    _gifView.userInteractionEnabled = NO;
    
    [self.view addSubview:_gifView];
    
    [[BeeSDK sharedInstance] initialize:[self createBeeParam]
                                timeout:20000
                                   sink:nil
                                handler:^(BeeErrorCode ec1) {
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
            [_beeVideoRoom join:_roomId
                            uid:_uid
                       nickName:_nickName
                        creator:_creator
                           role:eVideoRoomRole_Party];
            
            dispatch_async(dispatch_get_main_queue(), ^{
                CGRect screenRect = self.view.frame;
                CGRect pptRect = CGRectMake(screenRect.origin.x + _pptLeft, screenRect.origin.y + _pptTop, _pptWidth, _pptHeight);
                _whiteBoardContainer = [[UIView alloc] initWithFrame:pptRect];
                _whiteBoardContainer.clipsToBounds = YES;
                _whiteBoardContainer.layer.cornerRadius = 2.0;
                _whiteBoardContainer.layer.borderColor = [UIColor blackColor].CGColor;
                _whiteBoardContainer.layer.borderWidth = 1.0f;
                [self.view addSubview:_whiteBoardContainer];
                
                BeeWhiteBoardView *whiteBoardView = [[BeeWhiteBoardView alloc] initWithFrame:_whiteBoardContainer.bounds];
                _beeWhiteBoard = [[BeeWhiteBoard alloc] initWithParam:whiteBoardView
                                                                  handle:_handle
                                                                   token:_token
                                                                 timeout:20000
                                                                    sink:self];
                
                [_beeWhiteBoard setLineColor:_myDrawColorRGB];
                [_whiteBoardContainer addSubview:_beeWhiteBoard.view];
                
                [_beeWhiteBoard join:_roomId
                                 uid:_uid
                            nickName:_nickName
                             creator:NO
                                role:eBeeWhiteBoardRole_Student];
                
                _videoLeft = _pptLeft + _pptWidth + 10;
                _videoTop = _pptTop;
                _videoWidth = screenRect.size.width - _videoLeft - 10;
                _videoHeight = (CGFloat)_videoWidth * 480 / 640;
                _videoRenderer.view.frame = CGRectMake(_videoLeft, _videoTop, _videoWidth, _videoHeight);
            });
        }];
    }];
}

- (void)viewDidLayoutSubviews {
    [super viewDidLayoutSubviews];
    
    //_colorButton
    CGRect rect = CGRectMake(_pptLeft, _pptTop + _pptHeight + 6, 35.f, 35.f);
    _colorButton = [[UIButton alloc] initWithFrame:rect];
    _colorButton.layer.cornerRadius = 35.f / 2.f;
    [_colorButton setBackgroundColor:UIColorFromRGB(_myDrawColorRGB)];
    [_colorButton setBackgroundImage:[UIImage imageNamed:@"btn_whiteboard_select_color_disabled"] forState:UIControlStateDisabled];
    [_colorButton addTarget:self action:@selector(onColorButtonPressed:) forControlEvents:UIControlEventTouchUpInside];
    UIView *circle = [[UIView alloc] initWithFrame:CGRectMake(9.f, 9.f, 17.f, 17.f)];
    circle.layer.cornerRadius = 17.f / 2.f;
    circle.layer.borderColor = [UIColor whiteColor].CGColor;
    circle.layer.borderWidth = 1;
    [circle setUserInteractionEnabled:NO];
    [_colorButton addSubview:circle];
    [self.view addSubview:_colorButton];
    
    
    //_penButton
    rect = CGRectMake(_colorButton.frame.origin.x + _colorButton.frame.size.width + _buttonSpacing, _colorButton.frame.origin.y, 35.f, 35.f);
    _penButton = [[UIButton alloc] initWithFrame:rect];
    [_penButton setBackgroundImage:[UIImage imageNamed:@"btn_whiteboard_pen_normal"] forState:UIControlStateNormal];
    [_penButton setBackgroundImage:[UIImage imageNamed:@"btn_whiteboard_pen_pressed"] forState:UIControlStateHighlighted];
    [_penButton setBackgroundImage:[UIImage imageNamed:@"btn_whiteboard_pen_disabled"] forState:UIControlStateDisabled];
    [_penButton addTarget:self action:@selector(onPenButtonPresssed:) forControlEvents:UIControlEventTouchUpInside];
    _penButton.layer.cornerRadius = 2.0;
    _penButton.layer.borderColor = [UIColor blackColor].CGColor;
    _penButton.layer.borderWidth = 1.0f;
    [self.view addSubview:_penButton];
    
    
    //_eraserButton
    rect = CGRectMake(_penButton.frame.origin.x + _penButton.frame.size.width + _buttonSpacing, _penButton.frame.origin.y, 35.f, 35.f);
    _eraserButton = [[UIButton alloc] initWithFrame:rect];
    [_eraserButton setBackgroundImage:[UIImage imageNamed:@"btn_whiteboard_eraser_normal"] forState:UIControlStateNormal];
    [_eraserButton setBackgroundImage:[UIImage imageNamed:@"btn_whiteboard_eraser_pressed"] forState:UIControlStateHighlighted];
    [_eraserButton setBackgroundImage:[UIImage imageNamed:@"btn_whiteboard_eraser_disabled"] forState:UIControlStateDisabled];
    [_eraserButton addTarget:self action:@selector(onEraserButtonPressed:) forControlEvents:UIControlEventTouchUpInside];
    _eraserButton.layer.cornerRadius = 2.0;
    _eraserButton.layer.borderColor = [UIColor blackColor].CGColor;
    _eraserButton.layer.borderWidth = 0.0f;
    [self.view addSubview:_eraserButton];
    
    
    //_undoButton
    rect = CGRectMake(_eraserButton.frame.origin.x + _eraserButton.frame.size.width + _buttonSpacing, _eraserButton.frame.origin.y, 35.f, 35.f);
    _undoButton = [[UIButton alloc] initWithFrame:rect];
    [_undoButton setBackgroundImage:[UIImage imageNamed:@"btn_whiteboard_cancel_normal"] forState:UIControlStateNormal];
    [_undoButton setBackgroundImage:[UIImage imageNamed:@"btn_whiteboard_cancel_pressed"] forState:UIControlStateHighlighted];
    [_undoButton setBackgroundImage:[UIImage imageNamed:@"btn_whiteboard_cancel_disabled"] forState:UIControlStateDisabled];
    [_undoButton addTarget:self action:@selector(onUndoButtonPressed:) forControlEvents:UIControlEventTouchUpInside];
    [self.view addSubview:_undoButton];
    
    
    //_redoButton
    rect = CGRectMake(_undoButton.frame.origin.x + _undoButton.frame.size.width + _buttonSpacing, _undoButton.frame.origin.y, 35.f, 35.f);
    _redoButton = [[UIButton alloc] initWithFrame:rect];
    [_redoButton setBackgroundImage:[UIImage imageNamed:@"btn_whiteboard_redo_normal"] forState:UIControlStateNormal];
    [_redoButton setBackgroundImage:[UIImage imageNamed:@"btn_whiteboard_redo_pressed"] forState:UIControlStateHighlighted];
    [_redoButton setBackgroundImage:[UIImage imageNamed:@"btn_whiteboard_redo_disabled"] forState:UIControlStateDisabled];
    [_redoButton addTarget:self action:@selector(onRedoButtonPressed:) forControlEvents:UIControlEventTouchUpInside];
    [self.view addSubview:_redoButton];
    
    
    //_clearButton
    rect = CGRectMake(_redoButton.frame.origin.x + _redoButton.frame.size.width + _buttonSpacing, _redoButton.frame.origin.y, 35.f, 35.f);
    _clearButton = [[UIButton alloc] initWithFrame:rect];
    [_clearButton setBackgroundImage:[UIImage imageNamed:@"btn_whiteboard_clear_normal"] forState:UIControlStateNormal];
    [_clearButton setBackgroundImage:[UIImage imageNamed:@"btn_whiteboard_clear_pressed"] forState:UIControlStateHighlighted];
    [_clearButton setBackgroundImage:[UIImage imageNamed:@"btn_whiteboard_clear_disabled"] forState:UIControlStateDisabled];
    [_clearButton addTarget:self action:@selector(onClearButtonPressed:) forControlEvents:UIControlEventTouchUpInside];
    [self.view addSubview:_clearButton];
    
    //_lockButton
    rect = CGRectMake(_clearButton.frame.origin.x + _clearButton.frame.size.width + _buttonSpacing, _clearButton.frame.origin.y, 35.f, 35.f);
    _lockButton = [[UIButton alloc] initWithFrame:rect];
    [_lockButton setImage:[UIImage imageNamed:@"btn_whiteboard_unlock"]
                 forState:UIControlStateNormal];
    [_lockButton addTarget:self action:@selector(onLockButtonPressed:) forControlEvents:UIControlEventTouchUpInside];
    _lockButton.hidden = NO;
    [self.view addSubview:_lockButton];
    
    //_unlockButton
    _unlockButton = [[UIButton alloc] initWithFrame:_lockButton.frame];
    [_unlockButton setBackgroundImage:[UIImage imageNamed:@"btn_whiteboard_lock"] forState:UIControlStateNormal];
    [_unlockButton addTarget:self action:@selector(onUnlockButtonPressed:) forControlEvents:UIControlEventTouchUpInside];
    _unlockButton.hidden = YES;
    [self.view addSubview:_unlockButton];
    
    //_resetButton
    rect = CGRectMake(_lockButton.frame.origin.x + _lockButton.frame.size.width + _buttonSpacing + 10, _lockButton.frame.origin.y, 35.f, 35.f);
    _resetButton = [[UIButton alloc] initWithFrame:rect];
    [_resetButton setBackgroundImage:[UIImage imageNamed:@"btn_whiteboard_reset"] forState:UIControlStateNormal];
    [_resetButton addTarget:self action:@selector(onResetButtonPressed:) forControlEvents:UIControlEventTouchUpInside];
    [self.view addSubview:_resetButton];
    
    rect = CGRectMake(_pptLeft, _pptTop, 45.f, 45.f);
    _backButton = [[UIButton alloc] initWithFrame:rect];
    [_backButton setBackgroundImage:[UIImage imageNamed:@"btn_whiteboard_back"] forState:UIControlStateNormal];
    [_backButton addTarget:self action:@selector(onBackButtonPressed:) forControlEvents:UIControlEventTouchUpInside];
    [self.view addSubview:_backButton];
    
    //_gifView
    CGFloat gifTop = _videoTop + _videoHeight + 10;
    rect = CGRectMake(_videoLeft, gifTop, _videoWidth, self.view.frame.size.height - gifTop - 10);
    _gifView.frame = rect;
    CGSize actualSize = [_gifView sizeThatFits:CGSizeZero];
    CGRect newFrame = _gifView.frame;
    newFrame.size.height = actualSize.height;
    _gifView.frame = newFrame;
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
        _videoRoomLeaved = YES;
        if (_videoRoomLeaved && _whiteBoardLeaved) {
            [[BeeSDK sharedInstance] closeSession:_handle handler:^(BeeErrorCode ec) {
                NSLog(@"Session closed %d error:%d.", _handle, ec);
                _handle = -1;
            }];
        }
    });
}

- (void)onMembers:(NSArray<BeeVideoRoomMemberInfo*>*)remoteMembers {
    dispatch_async(dispatch_get_main_queue(), ^{
        NSLog(@"Report %lu members.", (unsigned long)[remoteMembers count]);
        for (BeeVideoRoomMemberInfo *memberInfo in remoteMembers) {
            if (memberInfo.role == eVideoRoomRole_Manager) {
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
    });
}

- (void)onDisconnect:(NSString*)uid
          streamName:(NSString*)streamName
               error:(BeeErrorCode)error
                 msg:(NSString*)msg {
    dispatch_async(dispatch_get_main_queue(), ^{
        NSLog(@"User %@ disconnect result %d.", streamName, error);
        [self removeUser:uid];
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

}

- (void)onAudioGlitchDetected:(bee_int64_t)totalNumberOfGlitches {
    
}

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
        _whiteBoardLeaved = YES;
        if (_videoRoomLeaved && _whiteBoardLeaved) {
            [[BeeSDK sharedInstance] closeSession:_handle handler:^(BeeErrorCode ec) {
                NSLog(@"Session closed %d error:%d.", _handle, ec);
                _handle = -1;
            }];
        }
    });
}

#pragma mark - BeeVideoRendererDelegate

- (void)didChangeVideoSize:(BeeVideoRenderer*)videoRenderer size:(CGSize)size {
    _mainVideoSize = size;
    
    CGRect bounds = _videoRenderer.view.bounds;
    if (_mainVideoSize.width > 0 && _mainVideoSize.height > 0) {
        // Aspect fill remote video into bounds.
        CGRect mainVideoFrame = AVMakeRectWithAspectRatioInsideRect(_mainVideoSize, bounds);
        CGFloat scale = 1;
        if (mainVideoFrame.size.width > mainVideoFrame.size.height) {
            // Scale by height.
            scale = bounds.size.height / mainVideoFrame.size.height;
        } else {
            // Scale by width.
            scale = bounds.size.width / mainVideoFrame.size.width;
        }
        mainVideoFrame.size.height *= scale;
        mainVideoFrame.size.width *= scale;
        _videoRenderer.view.frame = mainVideoFrame;
        _videoRenderer.view.center = CGPointMake(CGRectGetMidX(bounds), CGRectGetMidY(bounds));
    } else {
        _videoRenderer.view.frame = bounds;
    }
}

#pragma mark - Private

- (NSString*)getLogPath {
    NSArray *paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
    NSString *documentDirectory = [paths objectAtIndex:0];
    return documentDirectory;
}

- (BeeSDKParam*)createBeeParam {
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
    return param;
}

- (void)createUser:(BeeVideoRoomMemberInfo*)memberInfo {
    if (memberInfo == nil) {
        return;
    }
    
    if ([_userTable objectForKey:memberInfo.uid]) {
        return;
    }
    
    User *user = [[User alloc] init];
    user.uid = memberInfo.uid;
    user.nickName = memberInfo.nickName;
    user.streamName = memberInfo.streamName;
    user.partyType = memberInfo.partyType;
    user.mediaType = memberInfo.mediaType;
    
    [_userTable setObject:user forKey:user.uid];
    
    if (_beeVideoRoom != nil) {
        [_beeVideoRoom connect:user.uid
                    streamName:user.streamName
                     mediaType:user.mediaType
                 videoRenderer:_videoRenderer];
    }
}

- (void)removeUser:(NSString*)uid {
    if (uid == nil) {
        return;
    }
    
    [_userTable removeObjectForKey:uid];
}

#pragma mark - Control buttons.

- (void)onColorButtonPressed:(id)sender {
    _currentColorIndex = (_currentColorIndex + 1) % _colors.count;
    _myDrawColorRGB = [_colors[_currentColorIndex] intValue];
    [_colorButton setBackgroundColor:UIColorFromRGB(_myDrawColorRGB)];
    
    if (_beeWhiteBoard != nil) {
        [_beeWhiteBoard setLineColor:_myDrawColorRGB];
    }
}

- (void)onPenButtonPresssed:(id)sender {
    _penButton.layer.cornerRadius = 2.0;
    _penButton.layer.borderColor = [UIColor blackColor].CGColor;
    _penButton.layer.borderWidth = 1.0f;
    _eraserButton.layer.borderWidth = 0.0f;
    
    if (_beeWhiteBoard != nil) {
        [_beeWhiteBoard setDrawingMode:eBeeDrawingMode_Pen];
    }
}

- (void)onEraserButtonPressed:(id)sender {
    _eraserButton.layer.cornerRadius = 2.0;
    _eraserButton.layer.borderColor = [UIColor blackColor].CGColor;
    _eraserButton.layer.borderWidth = 1.0f;
    _penButton.layer.borderWidth = 0.0f;
    
    if (_beeWhiteBoard != nil) {
        [_beeWhiteBoard setDrawingMode:eBeeDrawingMode_Eraser];
    }
}

- (void)onUndoButtonPressed:(id)sender {
    if (_beeWhiteBoard != nil) {
        [_beeWhiteBoard undo];
    }
}

- (void)onRedoButtonPressed:(id)sender {
    if (_beeWhiteBoard != nil) {
        [_beeWhiteBoard redo];
    }
}

- (void)onClearButtonPressed:(id)sender {
    if (_beeWhiteBoard != nil) {
        [_beeWhiteBoard clearAll];
    }
}

- (void)onLockButtonPressed:(id)sender {
    if (_beeWhiteBoard != nil) {
        [_beeWhiteBoard lockDrawing];
    }
    _lockButton.hidden = YES;
    _unlockButton.hidden = NO;
}

- (void)onUnlockButtonPressed:(id)sender {
    if (_beeWhiteBoard != nil) {
        [_beeWhiteBoard unlockDrawing];
    }
    _lockButton.hidden = NO;
    _unlockButton.hidden = YES;
}

- (void)onResetButtonPressed:(id)sender {
    if (_beeWhiteBoard != nil) {
        [_beeWhiteBoard resetFrame];
    }
}

- (void)onBackButtonPressed:(id)sender {
    if (_beeWhiteBoard != nil) {
        [_beeWhiteBoard leave];
        _beeWhiteBoard = nil;
    }
    
    if (_beeVideoRoom != nil) {
        [_beeVideoRoom leave];
        _beeVideoRoom = nil;
    }
    
    [_delegate whiteBoardViewControllerDidFinish:self];
}

@end

