#import "bee/ios/bee_white_board.h"
#import "bee/ios/bee_sdk.h"
#import "bee/ios/bee_white_board_page.h"
#import "bee/ios/bee_white_board_line.h"
#import "bee/ios/bee_white_board_text.h"


#pragma mark - BeeWhiteBoardUser

@interface BeeWhiteBoardUser : NSObject

@property(nonatomic, copy) NSString *uid;
@property(nonatomic, copy) NSString *nickName;
@property(nonatomic, assign) BeeWhiteBoardRole role;
@property(nonatomic, assign) BOOL newLine;

@end

@implementation BeeWhiteBoardUser

@synthesize uid = _uid;
@synthesize nickName = _nickName;
@synthesize role = _role;
@synthesize newLine = _newLine;

- (instancetype)initWithParam:(NSString*)uid
                     nickName:(NSString*)nickName
                         role:(BeeWhiteBoardRole)role {
    if (self = [super init]) {
        _uid = uid;
        _nickName = nickName;
        _role = role;
        _newLine = YES;
    }
    return self;
}

@end

#pragma mark - BeeWhiteBoard

@implementation BeeWhiteBoard {
    //Service must register to a session.
    bee_handle _handle;
    
    //Room name must be unique and published by app.
    NSString *_roomName;
    
    //App should bind token for validating service.
    NSString *_token;
    
    //Local user info.
    BOOL _creator;
    BeeWhiteBoardRole _role;
    NSString *_uid;
    NSString *_nickName;
    
    bee_int32_t _timeout;
    __weak id<BeeWhiteBoardSink> _sink;
    NSMutableDictionary *_userTable;
}

@synthesize view = _view;

#pragma mark - Instance method

- (instancetype)initWithParam:(BeeWhiteBoardView*)view
                       handle:(bee_handle)handle
                        token:(NSString*)token
                      timeout:(bee_int32_t)timeout
                         sink:(id<BeeWhiteBoardSink>)sink {
    if (self = [super initWithSvcCode:kBeeSvcType_WhiteBoard]) {
        _roomName = nil;
        _handle = handle;
        _token = token;
        _timeout = timeout;
        _sink = sink;
        _creator = YES;
        _role = eBeeWhiteBoardRole_None;
        _uid = nil;
        _nickName = nil;
        _userTable = [[NSMutableDictionary alloc] init];
        _view = view;
        if (_view != nil) {
            [_view setWhiteBoardDelegate:self];
        }
    }
    return self;
}

#pragma mark - Export methods

- (void)join:(NSString*)roomName
         uid:(NSString*)uid
    nickName:(NSString*)nickName
     creator:(BOOL)creator
        role:(BeeWhiteBoardRole)role {
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

- (void)sendMessage:(NSString *)type
                 to:(NSArray *)targets
                msg:(NSString*)msg
            handler:(void(^)(BeeErrorCode error))handler {
    dispatch_async([[BeeSDK sharedInstance] getDispatchQueue], ^{
        [self sendMessageInternal:type
                               to:targets
                              msg:msg
                          handler:handler];
    });
}

- (void)clearAll {
    dispatch_async([[BeeSDK sharedInstance] getDispatchQueue], ^{
        if (_view != nil) {
            [_view clearScreen];
        }
    
        [self notifyClearAll];
    });
}

- (void)undo {
    dispatch_async([[BeeSDK sharedInstance] getDispatchQueue], ^{
        if (_view != nil) {
            [_view undo];
        }

        [self notifyUndo];
    });
}

- (void)redo {
    dispatch_async([[BeeSDK sharedInstance] getDispatchQueue], ^{
        if (_view != nil) {
            [_view redo];
        }
        
        [self notifyRedo];
    });
}

- (void)setDrawingMode:(BeeDrawingMode)mode {
    if (_view != nil) {
        [_view setDrawingMode:mode];
    }
}

- (void)setLineColor:(int)rgbColor {
    if (_view != nil) {
        [_view setLineColor:rgbColor];
    }
}

- (void)seLineWidth:(CGFloat)width {
    if (_view != nil) {
        [_view setLineWidth:width];
    }
}

- (void)lockDrawing {
    if (_view != nil) {
        [_view lockDrawing];
    }
}

- (void)unlockDrawing {
    if (_view != nil) {
        [_view unlockDrawing];
    }
}

- (void)resetFrame {
    if (_view != nil) {
        [_view resetFrame];
    }
}

#pragma mark - Internal methods.

- (void)joinInternal:(NSString*)roomName
                 uid:(NSString*)uid
            nickName:(NSString*)nickName
             creator:(BOOL)creator
                role:(BeeWhiteBoardRole)role {
    BeeErrorCode ret = kBeeErrorCode_Success;
    do {
        if (roomName == nil || uid == nil || nickName == nil || _token == nil) {
            ret = kBeeErrorCode_Invalid_Param;
            break;
        }
        
        NSLog(@"Joining white board:%@, creator:%@, uid:%@, nick name:%@.", roomName, creator?@"YES":@"NO", uid, nickName);
        
        if (![self isRegistered]) {
            ret = [self Register:_handle];
            if (ret != kBeeErrorCode_Success) {
                NSLog(@"[ER] White board service register failed.");
                break;
            }
        }
        
        _roomName = roomName;
        _uid = uid;
        _nickName = nickName;
        _creator = creator;
        _role = role;
        
        NSMutableDictionary *dictionary = [[NSMutableDictionary alloc] init];
        [dictionary setValue:_roomName forKey:@"room_name"];
        [dictionary setValue:[NSNumber numberWithBool:_creator] forKey:@"create"];
        [dictionary setValue:_uid forKey:@"uid"];
        [dictionary setValue:_nickName forKey:@"nick_name"];
        [dictionary setValue:_token forKey:@"token"];
        [dictionary setValue:[NSNumber numberWithInt:_role] forKey:@"role"];
        
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
    
    NSLog(@"Join white board %@ return %d.", roomName, ret);
    
    if (ret != kBeeErrorCode_Success) {
        if (_sink != nil) {
            [_sink onJoin:ret msg:[[BeeSDK sharedInstance] errorToString:ret]];
        }
    }
}

- (void)leaveInternal {
    BeeErrorCode ret = kBeeErrorCode_Success;
    do {
        NSLog(@"Leaving from white board.");
        
        if (![self isRegistered]) {
            NSLog(@"[ER] White board service not registered.");
            ret = kBeeErrorCode_Service_Not_Registered;
            break;
        }
        
        NSMutableDictionary *dictionary = [[NSMutableDictionary alloc] init];
        [dictionary setValue:_roomName forKey:@"room_name"];
        
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
    } while (false);
    
    if ([self isRegistered]) {
        [self unRegister];
    }
    
    NSLog(@"Leave white board %@ return %d.", _roomName, ret);
    
    if (_sink != nil) {
        [_sink onLeave:ret msg:[[BeeSDK sharedInstance] errorToString:ret]];
        _sink = nil;
    }
}

- (void)sendMessageInternal:(NSString*)cmd
                         to:(NSArray*)targets
                        msg:(NSString*)msg
                    handler:(void(^)(BeeErrorCode error))handler {
    BeeErrorCode ret = kBeeErrorCode_Success;
    do {
        if (![self isRegistered]) {
            NSLog(@"[ER] White board service not registered.");
            ret = kBeeErrorCode_Service_Not_Registered;
            break;
        }
        
        if (cmd == nil || targets == nil || msg == nil) {
            break;
        }
        
        NSMutableDictionary *dictionary = [[NSMutableDictionary alloc] init];
        [dictionary setValue:cmd forKey:@"cmd"];
        [dictionary setValue:_uid forKey:@"uid"];
        [dictionary setObject:targets forKey:@"uidlist"];
        [dictionary setValue:msg forKey:@"msg"];
        
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
        NSString *cmd = @"SendMessage";
        
        ret = [self execute:cmd args:args timeout:_timeout];
        if (ret != kBeeErrorCode_Success) {
            NSLog(@"[ER] SendMessage failed with error %d.", ret);
        }
    } while (false);
    
    if (handler != nil) {
        handler(ret);
    }
}

- (void)sendDataInternal:(NSString*)cmd
                      to:(NSArray*)targets
                    data:(NSMutableDictionary*)data
                 handler:(void(^)(BeeErrorCode error))handler {
    BeeErrorCode ret = kBeeErrorCode_Success;
    do {
        if (![self isRegistered]) {
            NSLog(@"[ER] White board service not registered.");
            ret = kBeeErrorCode_Service_Not_Registered;
            break;
        }
        
        if (cmd == nil || targets == nil || data == nil) {
            break;
        }
        
        NSMutableDictionary *dictionary = [[NSMutableDictionary alloc] init];
        [dictionary setValue:cmd forKey:@"cmd"];
        [dictionary setValue:_uid forKey:@"uid"];
        [dictionary setObject:targets forKey:@"uidlist"];
        [dictionary setObject:data forKey:@"msg"];
        
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
        NSString *cmd = @"SendMessage";
        
        ret = [self execute:cmd args:args timeout:_timeout];
        if (ret != kBeeErrorCode_Success) {
            NSLog(@"[ER] SendMessage failed with error %d.", ret);
        }
    } while (false);
    
    if (handler != nil) {
        handler(ret);
    }
}

- (void)notifyClearAll {
    NSString *cmd = @"clear";
    NSString *type = @"line";
    
    NSMutableDictionary *dictionary = [[NSMutableDictionary alloc] init];
    [dictionary setValue:cmd forKey:@"cmd"];
    [dictionary setValue:type forKey:@"type"];
    
    NSArray *targets = [NSArray arrayWithObjects:@"all",nil];
    [self sendDataInternal:@"bcast"
                        to:targets
                      data:dictionary
                   handler:^(BeeErrorCode error) {
                       if (error != kBeeErrorCode_Success) {
                           NSLog(@"Send notifyClearAll failed");
                       }
                   }];
}

- (void)notifyUndo {
    NSString *cmd = @"undo";
    NSString *type = @"line";
    
    NSMutableDictionary *dictionary = [[NSMutableDictionary alloc]init];
    [dictionary setValue:cmd forKey:@"cmd"];
    [dictionary setValue:type forKey:@"type"];
    
    NSArray *targets = [NSArray arrayWithObjects:@"all", nil];
    [self sendDataInternal:@"bcast"
                        to:targets
                      data:dictionary
                   handler:^(BeeErrorCode error) {
                       if (error != kBeeErrorCode_Success) {
                           NSLog(@"Send notifyUndo failed");
                       }
                   }];
}

- (void)notifyRedo {
    NSString *cmd = @"redo";
    NSString *type = @"line";
    
    NSMutableDictionary *dictionary = [[NSMutableDictionary alloc]init];
    [dictionary setValue:cmd forKey:@"cmd"];
    [dictionary setValue:type forKey:@"type"];
    
    NSArray *targets = [NSArray arrayWithObjects:@"all", nil];
    [self sendDataInternal:@"bcast"
                        to:targets
                      data:dictionary
                   handler:^(BeeErrorCode error) {
                       if (error != kBeeErrorCode_Success) {
                           NSLog(@"Send notifyRedo failed");
                       }
                   }];
}

- (void)getBoardInfo:(NSString*)teacher {
    NSString *cmd = @"boardinfo";
    NSString *type = @"line";
    
    NSMutableDictionary *dictionary = [[NSMutableDictionary alloc] init];
    [dictionary setValue:cmd forKey:@"cmd"];
    [dictionary setValue:type forKey:@"type"];

    NSArray *targets = [NSArray arrayWithObjects:teacher, nil];
    
    //Called from service thread, so call sendDataInternal directly.
    [self sendDataInternal:@"bcast"
                        to:targets
                      data:dictionary
                   handler:^(BeeErrorCode error) {
                       if (error != kBeeErrorCode_Success) {
                           NSLog(@"getBoardInfo failed");
                       }
                   }];
}

#pragma mark - Data handler

- (void)handleData:(NSString*)data {
    if (data == nil) {
        NSLog(@"[WA] White board data nil.");
        return;
    }
    
    NSData *nsData = [data dataUsingEncoding:NSUTF8StringEncoding];
    NSError *nsError = nil;
    id json = [NSJSONSerialization JSONObjectWithData:nsData options:NSJSONReadingAllowFragments error:&nsError];
    if (!json || ![json isKindOfClass:[NSDictionary class]] || nsError) {
        NSLog(@"[ER] Invalid white board data format.");
        return;
    }
    
    NSString *roomName = nil;
    NSString *uid = nil;
    NSString *nickName = nil;
    BeeErrorCode err = kBeeErrorCode_Success;
    NSString *msg = nil;
    WhiteBoardMsgType eType = eWhiteBoardMsgType_Count;
    BeeWhiteBoardRole eRole = eBeeWhiteBoardRole_None;
    
    NSDictionary *dictionary = (NSDictionary*)json;
    NSNumber *nsType = [dictionary objectForKey:@"type"];
    if (nsType != nil) {
        int type = [nsType intValue];
        eType = [self toMessageType:type];
        if (eType == eWhiteBoardMsgType_Count) {
            NSLog(@"Got invalid white board notify type %d.", type);
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
    
    NSNumber *nsRole = [dictionary objectForKey:@"role"];
    if (nsRole != nil) {
        int role = [nsRole intValue];
        eRole = [self toRole:role];
        if (eRole == eBeeWhiteBoardRole_None) {
            NSLog(@"Invalid white board role %d.", role);
            return;
        }
    }
    
    roomName = [dictionary objectForKey:@"room_name"];
    uid = [dictionary objectForKey:@"uid"];
    nickName = [dictionary objectForKey:@"nick_name"];
    msg = [dictionary objectForKey:@"msg"];
    
    switch (eType) {
        case eWhiteBoardMsgType_Local_Join: {
            [self handleLocalMemberJoined:err msg:msg];
            break;
        }
        case eWhiteBoardMsgType_Local_Leave: {
            [self handleLocalMemberLeaved:err msg:msg];
            break;
        }
        case eWhiteBoardMsgType_Remote_Join: {
            [self handleRemoteMemberJoined:uid nickName:nickName role:eRole];
            break;
        }
        case eWhiteBoardMsgType_Remote_Leave: {
            [self handleRemoteMemberLeaved:uid];
            break;
        }
        case eWhiteBoardMsgType_Message: {
            [self handleMessage:dictionary];
        }
        default:
            break;
    }
}

- (void)handleLocalMemberJoined:(BeeErrorCode)error
                            msg:(NSString*)msg {
    dispatch_async([[BeeSDK sharedInstance] getDispatchQueue], ^{
        if (_sink != nil) {
            [_sink onJoin:error msg:msg];
        }
    });
}

- (void)handleLocalMemberLeaved:(BeeErrorCode)error
                            msg:(NSString*)msg {
    dispatch_async([[BeeSDK sharedInstance] getDispatchQueue], ^{
        if (_sink != nil) {
            [_sink onLeave:error msg:msg];
        }
    });
}

- (void)handleRemoteMemberJoined:(NSString*)uid
                        nickName:(NSString*)nickName
                            role:(BeeWhiteBoardRole)role {
    dispatch_async([[BeeSDK sharedInstance] getDispatchQueue], ^{
        if (uid != nil) {
            BeeWhiteBoardUser *user = [[BeeWhiteBoardUser alloc] initWithParam:uid nickName:nickName role:role];
            [_userTable setObject:user forKey:uid];
            
            if (user.role == eBeeWhiteBoardRole_Teacher) {
                [self getBoardInfo:user.uid];
            }
        }
    });
}

- (void)handleRemoteMemberLeaved:(NSString*)uid {
    dispatch_async([[BeeSDK sharedInstance] getDispatchQueue], ^{
        if (uid != nil) {
            [_userTable removeObjectForKey:uid];
        }
    });
}

- (void)handleMessage:(NSDictionary*)data {
    NSString *from = nil;
    NSDictionary *msg = nil;
    do {
        if (data == nil) {
            break;
        }
        
        from = [data objectForKey:@"from"];
        if (from == nil) {
            NSLog(@"[Warn] No message source present");
            break;
        }
        
        NSDictionary *whiteBoardData = [data objectForKey:@"message"];
        if (whiteBoardData == nil) {
            NSLog(@"[ER] No message present");
            break;
        }
        
        msg = [whiteBoardData objectForKey:@"msg"];
    } while (false);
    if (from != nil && msg != nil) {
        [self handleWhiteBoardMessage:from msg:msg];
    }
}

- (void)handleWhiteBoardMessage:(NSString*)from
                            msg:(NSDictionary*)msg {
    dispatch_async(dispatch_get_main_queue(), ^{
        NSString *cmd = [msg valueForKey:@"cmd"];
        if (cmd == nil) {
            NSLog(@"cmd not exist in message.");
            return;
        }
        
        BeeWhiteBoardUser *user = [_userTable objectForKey:from];
        if (user == nil) {
            NSLog(@"White board message received from invalid source %@.", from);
            return;
        }

        if ([cmd isEqualToString:@"drawing"]) {
            BeeDrawingMode mode = [self handleDrawing:from data:msg newLine:user.newLine];
            if (mode == eBeeDrawingMode_Pen || mode == eBeeDrawingMode_Eraser) {
                user.newLine = NO;
            }
        } else if ([cmd isEqualToString:@"savedata"]) {
            [self handleSaveData:from];
            user.newLine = YES;
        } else if ([cmd isEqualToString:@"clear"]) {
            [self handleClear];
        } else if ([cmd isEqualToString:@"undo"]) {
            [self handleUndo];
        } else if ([cmd isEqualToString:@"redo"]) {
            [self handleRedo];
        } else if ([cmd isEqualToString:@"boardinfoback"]) {
            [self handleBoardInfo:from data:msg];
        } else if ([cmd isEqualToString:@"nextpage"]) {
            [self handleNextPage];
        } else if ([cmd isEqualToString:@"prepage"]) {
            [self handlePrePage];
        }
    });
}

#pragma mark - WhiteBoard interface.

- (BeeDrawingMode)handleDrawing:(NSString*)from
                           data:(NSDictionary*)dictionary
                        newLine:(BOOL)newLine {
    if (_view == nil) {
        return eBeeDrawingMode_None;
    }
    
    NSString *x0 = [dictionary valueForKey:@"x0"];
    NSString *y0 = [dictionary valueForKey:@"y0"];
    NSString *x1 = [dictionary valueForKey:@"x1"];
    NSString *y1 = [dictionary valueForKey:@"y1"];
    if (x0 == nil || y0 == nil || x1 == nil || y1 == nil) {
        return eBeeDrawingMode_None;
    }
    
    NSString *color = [dictionary valueForKey:@"color"];
    NSString *strokeWidth = [dictionary valueForKey:@"strokeWidth"];
    NSString *tool = [dictionary valueForKey:@"tool"];
    BeeDrawingMode mode = [self getDrawingModeFromTool:tool];
    switch (mode) {
        case bee::eBeeDrawingMode_Text: {
            NSString *data = [dictionary valueForKey:@"data"];
            CGPoint pos = CGPointMake([x0 doubleValue], [y0 doubleValue]);
            [_view drawText:data
                         pos:pos
                       color:[self getCGColorRefFromString:color]
                        size:[self getStokeWidthFromString:strokeWidth]
                      drawer:from];
            break;
        }
        default: {
            if (newLine) {
                CGPoint begin = CGPointMake([x0 doubleValue], [y0 doubleValue]);
                [_view lineBegin:begin
                     strokeColor:[self getCGColorRefFromString:color]
                     strokeWidth:[self getStokeWidthFromString:strokeWidth]
                            mode:mode
                          drawer:from];
                
                CGPoint point = CGPointMake([x1 doubleValue], [y1 doubleValue]);
                [_view lineMove:point
                           mode:mode
                         drawer:from];
            } else {
                CGPoint point;
                point.x = [x1 doubleValue];
                point.y = [y1 doubleValue];
                [_view lineMove:point
                           mode:mode
                         drawer:from];
            }
            break;
        }
    
    }
    
    return mode;
}

- (void)handleSaveData:(NSString*)from {
    if (_view != nil) {
        [_view lineEnd:from];
    }
}

- (void)handleClear {
    if (_view != nil) {
        [_view clearScreen];
    }
}

- (void)handleUndo {
    if (_view != nil) {
        [_view undo];
    }
}

- (void)handleRedo {
    if (_view != nil) {
        [_view redo];
    }
}

- (void)handleBoardInfo:(NSString*)from
                   data:(NSDictionary*)dictionary {
    NSLog(@"handleBoardInfo in");
    NSMutableArray *whiteBoardPages = nil;
    NSInteger currentPageIndex = 0;
    CGSize canvasSize = CGSizeZero;
    CGPoint refPos = CGPointZero;
    do {
        if (_view == nil) {
            NSLog(@"White board view not set");
            break;
        }
        
        NSDictionary *data = [dictionary objectForKey:@"data"];
        if (data == nil || data.count == 0) {
            NSLog(@"No data in board info");
            break;
        }
        
        NSString *imgBaseUrl = nil;
        NSString *imgName = nil;
        NSString *imgFormat = nil;
        NSInteger startIdx = 0;
        NSInteger endIdx = 0;
        
        NSDictionary *imgInfo = [data objectForKey:@"imgjson"];
        if (imgInfo != nil) {
            imgBaseUrl = [imgInfo valueForKey:@"baseurl"];
            if (imgBaseUrl != nil) {
                imgName = [imgInfo valueForKey:@"name"];
                if (imgName == nil) {
                    NSLog(@"No image name in board info");
                    break;
                }
                
                imgFormat = [imgInfo valueForKey:@"format"];
                if (imgFormat == nil) {
                    NSLog(@"No image format in board info");
                    break;
                }
                
                NSString *startIdxStr = [imgInfo valueForKey:@"startidx"];
                if (startIdxStr == nil) {
                    NSLog(@"No image startidx in board info");
                    return;
                }
                startIdx = [startIdxStr integerValue];
                
                NSString *endIdxStr = [imgInfo valueForKey:@"endidx"];
                if (endIdxStr == nil) {
                    NSLog(@"No image endIdx in board info");
                    break;
                }
                endIdx = [endIdxStr integerValue];
            } else {
                NSLog(@"No image base url in board info");
            }
        } else {
            NSLog(@"No image in board info");
        }
        
        NSDictionary *resInfo = [data objectForKey:@"resinfo"];
        if (resInfo == nil) {
            NSLog(@"No resolution in board info");
            break;
        }
        
        NSString *widthStr = [resInfo valueForKey:@"w"];
        if (widthStr == nil) {
            NSLog(@"No width in resolution info");
            break;
        }
        
        NSString *heightStr = [resInfo valueForKey:@"h"];
        if (heightStr == nil) {
            NSLog(@"No height in resolution info");
            break;
        }
        
        NSDictionary *locInfo = [resInfo objectForKey:@"locinfo"];
        if (locInfo != nil) {
            NSString *xStr = [locInfo valueForKey:@"x"];
            if (xStr != nil) {
                refPos.x = [xStr floatValue];
            }
            NSString *yStr = [locInfo valueForKey:@"y"];
            if (yStr != nil) {
                refPos.y = [yStr floatValue];
            }
        }

        NSInteger count = endIdx - startIdx + 1;
        whiteBoardPages = [[NSMutableArray alloc] initWithCapacity:count];
        NSArray *pages = [data objectForKey:@"reducerinfo"];
        canvasSize.width = [widthStr floatValue];
        canvasSize.height = [heightStr floatValue];
        
        for (NSUInteger i = 0; i < count; ++i) {
            NSString *imgUrl = nil;
            if (imgBaseUrl != nil && imgName != nil && imgFormat != nil) {
                imgUrl = [[NSString alloc] initWithFormat:@"%@%@-%lu.%@",imgBaseUrl,imgName,i,imgFormat];
            }
            BeeWhiteBoardPage *whiteBoardPage =
                [[BeeWhiteBoardPage alloc] initWithParam:from
                                                     url:imgUrl
                                              canvasSize:canvasSize
                                                  refPos:refPos
                                              targetSize:_view.frame.size];
            [whiteBoardPages insertObject:whiteBoardPage atIndex:i];
            
            if (pages != nil && pages.count > 0 && i < pages.count) {
                NSObject *obj = [pages objectAtIndex:i];
                if (obj == nil || ![obj isKindOfClass:[NSDictionary class]]) { //Reach unread page.
                    continue;
                }
                
                NSDictionary *page = (NSDictionary*)obj;
                if ([page objectForKey:@"past"] == nil ||
                    [page objectForKey:@"present"] == nil ||
                    [page objectForKey:@"future"] == nil) {
                    continue;
                }
                
                NSArray *pastItems = [page objectForKey:@"past"];
                [self cacheWhiteBoardItems:pastItems
                                   toStack:eBeeWhiteBoardStack_Undo
                                    inPage:whiteBoardPage];
                
                NSArray *futureItems = [page objectForKey:@"future"];
                [self cacheWhiteBoardItems:futureItems
                                   toStack:eBeeWhiteBoardStack_Redo
                                    inPage:whiteBoardPage];
            }
        }
        
        NSString *currentPageIndexStr = [data valueForKey:@"page"];
        if (currentPageIndexStr != nil) {
            currentPageIndex = [currentPageIndexStr integerValue];
        }
        NSLog(@"Current page index %ld", currentPageIndex);
    } while (false);
    
    [_view setBoardInfo:whiteBoardPages
       currentPageIndex:currentPageIndex
             canvasSize:canvasSize
                 refPos:refPos];
}

- (void)cacheWhiteBoardItems:(NSArray*)items
                     toStack:(BeeWhiteBoardStack)stack
                      inPage:(BeeWhiteBoardPage*)whiteBoardPage {
    if (items == nil || items.count == 0 || whiteBoardPage == nil) {
        return;
    }
    
    for (NSDictionary *item in items) {
        if (item == nil) {
            continue;
        }
        NSString *color = [item valueForKey:@"lc"];
        NSString *width = [item valueForKey:@"lw"];
        NSString *tool = [item valueForKey:@"tool"];
        NSArray *path = [item objectForKey:@"path"];
        if (color == nil || width == nil || tool == nil || path == nil) {
            continue;
        }
        
        BeeDrawingMode mode = [self getDrawingModeFromTool:tool];
        switch (mode) {
            case bee::eBeeDrawingMode_Text: {
                NSString *data = [item objectForKey:@"data"];
                if (data != nil && path.count >= 2) {
                    CGFloat x = [[path objectAtIndex:0] doubleValue];
                    CGFloat y = [[path objectAtIndex:1] doubleValue];
                    CGPoint pos = CGPointMake(x, y);
                    
                    BeeWhiteBoardText *text =
                        [[BeeWhiteBoardText alloc] initWithParam:[self getCGColorRefFromString:color]
                                                        fontSize:[self getStokeWidthFromString:width]
                                                            mode:mode
                                                            data:data
                                                             pos:pos];
                    [whiteBoardPage cacheItem:text stack:stack];
                }
                break;
            }
            default:{
                BeeWhiteBoardLine *line =
                    [[BeeWhiteBoardLine alloc] initWithParam:[self getCGColorRefFromString:color]
                                                 strokeWidth:[self getStokeWidthFromString:width]
                                                        mode:mode
                                                        path:path];
                [whiteBoardPage cacheItem:line stack:stack];
                break;
            }
        }
    }
}

- (void)handleNextPage {
    if (_view != nil) {
        [_view nextPage];
    }
}

- (void)handlePrePage {
    if (_view != nil) {
        [_view prePage];
    }
}

#pragma mark - BeeWhiteBoardViewDelegate

- (void)onDrawingLine:(CGPoint)begin
                  end:(CGPoint)end
                color:(CGColorRef)color
                width:(CGFloat)width
                 mode:(BeeDrawingMode)mode {
    dispatch_async([[BeeSDK sharedInstance] getDispatchQueue], ^{
        NSString *cmd = @"drawing";
        NSString *colorStr = [self getStringFromCGColorRef:color];
        NSString *strokeWidth = [[NSString alloc] initWithFormat:@"%d", (int)width];
        NSString *type = @"line";

        NSMutableDictionary *dictionary = [[NSMutableDictionary alloc]init];
        [dictionary setValue:cmd forKey:@"cmd"];
        [dictionary setValue:colorStr forKey:@"color"];
        [dictionary setValue:strokeWidth forKey:@"strokeWidth"];
        [dictionary setValue:[NSNumber numberWithInt:(int)begin.x] forKey:@"x0"];
        [dictionary setValue:[NSNumber numberWithInt:(int)begin.y] forKey:@"y0"];
        [dictionary setValue:[NSNumber numberWithInt:(int)end.x] forKey:@"x1"];
        [dictionary setValue:[NSNumber numberWithInt:(int)end.y] forKey:@"y1"];
        [dictionary setValue:type forKey:@"type"];
        [dictionary setValue:[self getToolFromDrawingMode:mode] forKey:@"tool"];

        NSArray *targets = [NSArray arrayWithObjects:@"all",nil];
        [self sendDataInternal:@"bcast"
                            to:targets
                          data:dictionary
                       handler:^(BeeErrorCode error) {
                           if (error != kBeeErrorCode_Success) {
                               NSLog(@"Send drawing failed");
                           }
                       }];
    });
}

- (void)onDrawLineEnd {
    dispatch_async([[BeeSDK sharedInstance] getDispatchQueue], ^{
        NSString *cmd = @"savedata";
        NSString *type = @"line";

        NSMutableDictionary *dictionary = [[NSMutableDictionary alloc]init];
        [dictionary setValue:cmd forKey:@"cmd"];
        [dictionary setValue:type forKey:@"type"];

        NSArray *targets = [NSArray arrayWithObjects:@"all",nil];
        [self sendDataInternal:@"bcast"
                            to:targets
                          data:dictionary
                       handler:^(BeeErrorCode error) {
                           if (error != kBeeErrorCode_Success) {
                               NSLog(@"Send savedata failed");
                           }
                       }];
    });
}

#pragma mark - Private methods

- (WhiteBoardMsgType)toMessageType:(int)type {
    if (type >= eWhiteBoardMsgType_Local_Join && type < eWhiteBoardMsgType_Count) {
        return static_cast<WhiteBoardMsgType>(type);
    } else {
        return eWhiteBoardMsgType_Count;
    }
}

- (BeeWhiteBoardRole)toRole:(int)role {
    if (role >= eBeeWhiteBoardRole_None && role <= eBeeWhiteBoardRole_Student) {
        return static_cast<BeeWhiteBoardRole>(role);
    } else {
        return eBeeWhiteBoardRole_None;
    }
}

- (BeeErrorCode)toBeeErrorCode:(int)ec {
    if (ec >= kBeeErrorCode_Success && ec < kBeeErrorCode_Count) {
        return static_cast<BeeErrorCode>(ec);
    } else {
        return kBeeErrorCode_Not_Compatible;
    }
}

- (CGColorRef) getCGColorRefFromString:(NSString *)colorStr {
    //Default black.
    unsigned int r = 0;
    unsigned int g = 0;
    unsigned int b = 0;
    
    if (colorStr != nil) {
        if ([colorStr hasPrefix:@"#"])
            colorStr = [colorStr substringFromIndex:1];
        
        NSRange range;
        range.length = 2;
        
        range.location = 0;
        NSString *rString = [colorStr substringWithRange:range];
        
        range.location = 2;
        NSString *gString = [colorStr substringWithRange:range];
        
        range.location = 4;
        NSString *bString = [colorStr substringWithRange:range];
        
        [[NSScanner scannerWithString:rString] scanHexInt:&r];
        [[NSScanner scannerWithString:gString] scanHexInt:&g];
        [[NSScanner scannerWithString:bString] scanHexInt:&b];
    }
    
    CGColorSpaceRef rgbSapceRef = CGColorSpaceCreateDeviceRGB();
    CGFloat rgbComponents[] = {(float)r/255.0f, (float)g/255.0f, (float)b/255.0f, 1};
    CGColorRef rgbColorRef = CGColorCreate(rgbSapceRef, rgbComponents);
    return rgbColorRef;
}

- (NSString *)getStringFromCGColorRef:(CGColorRef)rgbColorRef {
    CGFloat r = 0.0f;
    CGFloat g = 0.0f;
    CGFloat b = 0.0f;
    if (rgbColorRef != nil) {
        const CGFloat *colorComponents = CGColorGetComponents(rgbColorRef);
        r = colorComponents[0];
        g = colorComponents[1];
        b = colorComponents[2];
    }
    NSString *colorStr = [[NSString alloc] initWithFormat:@"#%02x%02x%02x",(int)(r * 255), (int)(g * 255), (int)(b * 255)];
    return colorStr;
}

- (BeeDrawingMode)getDrawingModeFromTool:(NSString*)tool {
    static const NSDictionary *dic = @{@"pen":[NSNumber numberWithInt:eBeeDrawingMode_Pen],
                                       @"eraser":[NSNumber numberWithInt:eBeeDrawingMode_Eraser],
                                       @"laserpen":[NSNumber numberWithInt:eBeeDrawingMode_Laser],
                                       @"text":[NSNumber numberWithInt:eBeeDrawingMode_Text]};
    BeeDrawingMode mode = eBeeDrawingMode_None;
    do {
        NSNumber *nsTool = [dic valueForKey:tool];
        if (nsTool == nil) {
            break;
        }
        
        int nTool = [nsTool intValue];
        if (nTool < eBeeDrawingMode_Pen || nTool > eBeeDrawingMode_None) {
            break;
        }
        
        mode = static_cast<BeeDrawingMode>(nTool);
    } while (false);
    return mode;
}

- (NSString *)getToolFromDrawingMode:(BeeDrawingMode)mode {
    NSString *tool = [[NSString alloc] init];
    switch (mode) {
        case eBeeDrawingMode_Pen:
            tool = @"pen";
            break;
        case eBeeDrawingMode_Eraser:
            tool = @"eraser";
            break;
        case eBeeDrawingMode_Laser:
            tool = @"laserpen";
        default:
            break;
    }
    return tool;
}

- (CGFloat)getStokeWidthFromString:(NSString*)width {
    //Default 4.0.
    CGFloat strokeWidth = 4.0f;
    if (width != nil) {
        strokeWidth = [width doubleValue];
    }
    return strokeWidth;
}

@end
