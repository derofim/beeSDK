/*
 *  Copyright 2015 The WebRTC Project Authors. All rights reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#import "ARDMainView.h"

//#import "UIImage+ARDUtilities.h"

static CGFloat const kRoomTextFieldHeight = 40;
static CGFloat const kRoomTextFieldMargin = 8;
static CGFloat const kCallControlMargin = 8;

// Helper view that contains a text field and a clear button.
@interface ARDRoomTextField : UIView <UITextFieldDelegate>
@property(nonatomic, readonly) NSString *roomText;
@end

@implementation ARDRoomTextField {
  UITextField *_roomText;
}

- (instancetype)initWithFrame:(CGRect)frame
                  placeholder:(NSString*)placeholder
                         text:(NSString*)text {
  if (self = [super initWithFrame:frame]) {
    _roomText = [[UITextField alloc] initWithFrame:CGRectZero];
    _roomText.borderStyle = UITextBorderStyleNone;
    _roomText.font = [UIFont fontWithName:@"Roboto" size:12];
    _roomText.placeholder = placeholder;
    _roomText.autocorrectionType = UITextAutocorrectionTypeNo;
    _roomText.autocapitalizationType = UITextAutocapitalizationTypeNone;
    _roomText.clearButtonMode = UITextFieldViewModeAlways;
    _roomText.delegate = self;
    _roomText.text = text;
    [self addSubview:_roomText];

    // Give rounded corners and a light gray border.
    self.layer.borderWidth = 1;
    self.layer.borderColor = [[UIColor lightGrayColor] CGColor];
    self.layer.cornerRadius = 2;
  }
  return self;
}

- (void)layoutSubviews {
  _roomText.frame =
      CGRectMake(kRoomTextFieldMargin, 0, CGRectGetWidth(self.bounds) - kRoomTextFieldMargin,
                 kRoomTextFieldHeight);
}

- (CGSize)sizeThatFits:(CGSize)size {
  size.height = kRoomTextFieldHeight;
  return size;
}

- (NSString *)roomText {
  return _roomText.text;
}

- (void)setRoomText:(NSString *)text {
    _roomText.text = text;
}

#pragma mark - UITextFieldDelegate

- (BOOL)textFieldShouldReturn:(UITextField *)textField {
  // There is no other control that can take focus, so manually resign focus
  // when return (Join) is pressed to trigger |textFieldDidEndEditing|.
  [textField resignFirstResponder];
  return YES;
}

@end

@interface ARDMainView ()<NSXMLParserDelegate>
@end

@implementation ARDMainView {
  ARDRoomTextField *_roomText;
  ARDRoomTextField *_nameText;
  UISwitch *_loopbackSwitch;
  UILabel *_loopbackLabel;
  UIButton *_createButton;
  UIButton *_startCallButton;
  //UIButton *_audioLoopButton;
  NSMutableString *_currenXmlString;
}

@synthesize delegate = _delegate;
@synthesize isAudioLoopPlaying = _isAudioLoopPlaying;

- (instancetype)initWithFrame:(CGRect)frame {
  if (self = [super initWithFrame:frame]) {
    _roomText = [[ARDRoomTextField alloc] initWithFrame:CGRectZero placeholder:@"Room Name" text:@"0"];
    [self addSubview:_roomText];

    _nameText = [[ARDRoomTextField alloc] initWithFrame:CGRectZero placeholder:@"Nick Name" text:@"大聪明"];
    [self addSubview:_nameText];

    UIFont *controlFont = [UIFont fontWithName:@"Roboto" size:20];
    CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
    _startCallButton.layer.borderColor = CGColorCreate(colorSpace,(CGFloat[]){ 0, 0, 1, 1 });
      
    _createButton = [UIButton buttonWithType:UIButtonTypeSystem];
    [_createButton setTitle:@"Create"
                   forState:UIControlStateNormal];
    _createButton.titleLabel.font = controlFont;
    [_createButton sizeToFit];
    [_createButton addTarget:self
                      action:@selector(onCreateRoom:)
            forControlEvents:UIControlEventTouchUpInside];
    _createButton.layer.cornerRadius = 4.0;
    _createButton.layer.borderWidth = 1.5f;
    _createButton.contentEdgeInsets = UIEdgeInsetsMake(0,10, 0, 0);
      
    [self addSubview:_createButton];
      
    _startCallButton = [UIButton buttonWithType:UIButtonTypeSystem];
    [_startCallButton setTitle:@"Join"
                      forState:UIControlStateNormal];
    _startCallButton.titleLabel.font = controlFont;
    [_startCallButton sizeToFit];
    [_startCallButton addTarget:self
                         action:@selector(onJoinRoom:)
               forControlEvents:UIControlEventTouchUpInside];
    _startCallButton.layer.cornerRadius = 4.0;
    _startCallButton.layer.borderWidth = 1.5f;
    _startCallButton.contentEdgeInsets = UIEdgeInsetsMake(0,10, 0, 0);
    [self addSubview:_startCallButton];

    self.backgroundColor = [UIColor whiteColor];
  }
  return self;
}

- (void)setIsAudioLoopPlaying:(BOOL)isAudioLoopPlaying {
  if (_isAudioLoopPlaying == isAudioLoopPlaying) {
    return;
  }
  _isAudioLoopPlaying = isAudioLoopPlaying;
  [self updateAudioLoopButton];
}

- (void)layoutSubviews {
  CGRect bounds = self.bounds;
  CGFloat roomTextWidth = bounds.size.width - 2 * kRoomTextFieldMargin;
  CGFloat roomTextHeight = [_roomText sizeThatFits:bounds.size].height;
  _roomText.frame = CGRectMake(kRoomTextFieldMargin, kRoomTextFieldMargin, roomTextWidth, roomTextHeight);

  CGFloat nameTextTop = CGRectGetMaxY(_roomText.frame) + kCallControlMargin;
  _nameText.frame = CGRectMake(kRoomTextFieldMargin, nameTextTop, roomTextWidth, roomTextHeight);

  CGFloat createRoomTop = CGRectGetMaxY(_nameText.frame) + kCallControlMargin;
  _createButton.frame = CGRectMake(kCallControlMargin,
                                   createRoomTop,
                                   _roomText.frame.size.width / 2 - kCallControlMargin,
                                   _createButton.frame.size.height);
  
  CGFloat startCallLeft = CGRectGetMaxX(_createButton.frame) + kCallControlMargin * 2;
  _startCallButton.frame = CGRectMake(startCallLeft,
                                      createRoomTop,
                                      _roomText.frame.size.width / 2 - kCallControlMargin,
                                      _startCallButton.frame.size.height);
    
  [self loadLoginConfig];
}

#pragma mark - Private

- (void)updateAudioLoopButton {
  /*
  if (_isAudioLoopPlaying) {
    [_audioLoopButton setTitle:@"Stop sound"
                      forState:UIControlStateNormal];
    [_audioLoopButton sizeToFit];
  } else {
    [_audioLoopButton setTitle:@"Play sound"
                      forState:UIControlStateNormal];
    [_audioLoopButton sizeToFit];
  }
  */
}

- (void)onToggleAudioLoop:(id)sender {
  [_delegate mainViewDidToggleAudioLoop:self];
}

- (void)onCreateRoom:(id)sender {
  [_delegate mainView:self didInputRoom:_roomText.roomText name:_nameText.roomText create:YES];
  [self saveLoginConfig];
}

- (void)onJoinRoom:(id)sender {
    [_delegate mainView:self didInputRoom:_roomText.roomText name:_nameText.roomText create:NO];
    [self saveLoginConfig];
}

#pragma mark - Login config

- (NSString*)getDocumentPath {
    NSArray *paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
    NSString *documentDirectory = [paths objectAtIndex:0];
    return documentDirectory;
}

- (void)loadLoginConfig {
    NSString *documentPath = [self getDocumentPath];
    if (documentPath == nil) {
        return;
    }

    NSString *filePath = [documentPath stringByAppendingPathComponent:@"login.xml"];
    if (filePath == nil) {
        return;
    }

    NSString *content = [NSString stringWithContentsOfFile:filePath encoding:NSUTF8StringEncoding error:nil];
    if (content == nil) {
        return;
    }

    NSData *data = [content dataUsingEncoding:NSUTF8StringEncoding];
    if (data == nil) {
        return;
    }

    NSXMLParser *XMLParser = [[NSXMLParser alloc] initWithData:data];
    if (XMLParser == nil) {
        return;
    }

    [XMLParser setDelegate:self];
    [XMLParser parse];
}

- (void)saveLoginConfig {
    NSString *documentPath = [self getDocumentPath];
    NSString *filePath = [documentPath stringByAppendingPathComponent:@"login.xml"];
    NSString *content = [[NSString alloc] initWithFormat:@"<login><roomId>%@</roomId><nickName>%@</nickName></login>",_roomText.roomText, _nameText.roomText];
    [content writeToFile:filePath atomically:YES encoding:NSUTF8StringEncoding error:nil];
}

#pragma mark - NSXMLParserDelegate

-(void)parserDidStartDocument:(NSXMLParser *)parser{
    
}

-(void)parser:(NSXMLParser *)parser didStartElement:(NSString *)elementName namespaceURI:(NSString *)namespaceURI qualifiedName:(NSString *)qName attributes:(NSDictionary<NSString *,NSString *> *)attributeDict{
    
}

- (void)parser:(NSXMLParser *)parser foundCharacters:(NSString *)string {
    [self.currenXmlString appendString:string];
}

-(void)parser:(NSXMLParser *)parser didEndElement:(NSString *)elementName namespaceURI:(NSString *)namespaceURI qualifiedName:(NSString *)qName{
    if ([elementName isEqualToString:@"roomId"]) {
        _roomText.roomText = self.currenXmlString;
    } else if ([elementName isEqualToString:@"nickName"]) {
        _nameText.roomText = self.currenXmlString;
    }
    
    self.currenXmlString.string = @"";
}

-(void)parserDidEndDocument:(NSXMLParser *)parser{
    
}

- (NSMutableString *)currenXmlString {
    if (_currenXmlString == nil) {
        _currenXmlString = [NSMutableString string];
    }
    return _currenXmlString;
}

@end
