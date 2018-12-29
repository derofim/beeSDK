#import "EnterView.h"

static CGFloat const kRoomTextFieldHeight = 40;
static CGFloat const kRoomTextFieldMargin = 8;
static CGFloat const kCallControlMargin = 8;

#pragma mark - BeeTextField

@interface BeeTextField : UIView <UITextFieldDelegate>
@end

@implementation BeeTextField {
  UITextField *_textFiled;
}

- (instancetype)initWithFrame:(CGRect)frame {
    if (self = [super initWithFrame:frame]) {
        _textFiled = [[UITextField alloc] initWithFrame:CGRectZero];
        _textFiled.borderStyle = UITextBorderStyleNone;
        _textFiled.font = [UIFont fontWithName:@"Roboto" size:12];
        _textFiled.placeholder = @"Room name";
        _textFiled.autocorrectionType = UITextAutocorrectionTypeNo;
        _textFiled.autocapitalizationType = UITextAutocapitalizationTypeNone;
        _textFiled.clearButtonMode = UITextFieldViewModeAlways;
        _textFiled.delegate = self;
        _textFiled.text = @"0";
        [self addSubview:_textFiled];

        // Give rounded corners and a light gray border.
        self.layer.borderWidth = 1;
        self.layer.borderColor = [[UIColor lightGrayColor] CGColor];
        self.layer.cornerRadius = 2;
    }
    return self;
}

- (void)layoutSubviews {
    _textFiled.frame = CGRectMake(kRoomTextFieldMargin, 0, CGRectGetWidth(self.bounds) - kRoomTextFieldMargin, kRoomTextFieldHeight);
}

- (CGSize)sizeThatFits:(CGSize)size {
    size.height = kRoomTextFieldHeight;
    return size;
}

- (NSString *)text {
    return _textFiled.text;
}

- (void)setText:(NSString *)text {
    _textFiled.text = text;
}

#pragma mark - UITextFieldDelegate

- (BOOL)textFieldShouldReturn:(UITextField *)textField {
  // There is no other control that can take focus, so manually resign focus
  // when return (Join) is pressed to trigger |textFieldDidEndEditing|.
  [textField resignFirstResponder];
  return YES;
}

@end

#pragma mark - EnterView

@interface EnterView ()<NSXMLParserDelegate>

@end

@implementation EnterView {
  BeeTextField *_roomText;
  UIButton *_createButton;
  UIButton *_joinButton;
  NSMutableString *_currenXmlString;
}

@synthesize delegate = _delegate;

- (instancetype)initWithFrame:(CGRect)frame {
  if (self = [super initWithFrame:frame]) {
    _roomText = [[BeeTextField alloc] initWithFrame:CGRectZero];
    [self addSubview:_roomText];

    UIFont *controlFont = [UIFont fontWithName:@"Roboto" size:20];
    CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
      
    _createButton = [UIButton buttonWithType:UIButtonTypeSystem];
    [_createButton setTitle:@"Create" forState:UIControlStateNormal];
    _createButton.titleLabel.font = controlFont;
    [_createButton sizeToFit];
    [_createButton addTarget:self action:@selector(onCreateRoom:) forControlEvents:UIControlEventTouchUpInside];
    _createButton.layer.cornerRadius = 4.0;
    _createButton.layer.borderWidth = 1.5f;
    _createButton.layer.borderColor = CGColorCreate(colorSpace,(CGFloat[]){ 0, 0, 1, 1 });
    _createButton.contentEdgeInsets = UIEdgeInsetsMake(0,10, 0, 0);
    [self addSubview:_createButton];
      
    _joinButton = [UIButton buttonWithType:UIButtonTypeSystem];
    [_joinButton setTitle:@"Join" forState:UIControlStateNormal];
    _joinButton.titleLabel.font = controlFont;
    [_joinButton sizeToFit];
    [_joinButton addTarget:self action:@selector(onJoinRoom:) forControlEvents:UIControlEventTouchUpInside];
    _joinButton.layer.cornerRadius = 4.0;
    _joinButton.layer.borderWidth = 1.5f;
    _joinButton.layer.borderColor = CGColorCreate(colorSpace,(CGFloat[]){ 0, 0, 1, 1 });
    _joinButton.contentEdgeInsets = UIEdgeInsetsMake(0,10, 0, 0);
    [self addSubview:_joinButton];

    self.backgroundColor = [UIColor whiteColor];
  }
  return self;
}

- (void)layoutSubviews {
  CGRect bounds = self.bounds;
  CGFloat roomTextWidth = bounds.size.width - 2 * kRoomTextFieldMargin;
  CGFloat roomTextHeight = [_roomText sizeThatFits:bounds.size].height;
  _roomText.frame = CGRectMake(kRoomTextFieldMargin, kRoomTextFieldMargin, roomTextWidth, roomTextHeight);

  CGFloat createRoomTop = CGRectGetMaxY(_roomText.frame) + kCallControlMargin;
  _createButton.frame = CGRectMake(kCallControlMargin,
                                   createRoomTop,
                                   _roomText.frame.size.width / 2 - kCallControlMargin,
                                   _createButton.frame.size.height);
  
  CGFloat joinRoomLeft = CGRectGetMaxX(_createButton.frame) + kCallControlMargin * 2;
  _joinButton.frame = CGRectMake(joinRoomLeft,
                                 createRoomTop,
                                 _roomText.frame.size.width / 2 - kCallControlMargin,
                                 _joinButton.frame.size.height);
    
  [self loadLoginConfig];
}

#pragma mark - Private

- (void)onCreateRoom:(id)sender {
    [_delegate enterView:self didInputRoom:_roomText.text create:YES];
    [self saveLoginConfig];
}

- (void)onJoinRoom:(id)sender {
    [_delegate enterView:self didInputRoom:_roomText.text create:NO];
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
    NSString *filePath = [documentPath stringByAppendingPathComponent:@"login.xml"];
    NSString *content = [NSString stringWithContentsOfFile:filePath encoding:NSUTF8StringEncoding error:nil];
    
    NSData *data = [content dataUsingEncoding:NSUTF8StringEncoding];
    NSXMLParser *XMLParser = [[NSXMLParser alloc] initWithData:data];
    [XMLParser setDelegate:self];
    [XMLParser parse];
}

- (void)saveLoginConfig {
    NSString *documentPath = [self getDocumentPath];
    NSString *filePath = [documentPath stringByAppendingPathComponent:@"login.xml"];
    NSString *content = [[NSString alloc] initWithFormat:@"<roomId>%@</roomId>\n",_roomText.text];
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
        _roomText.text = self.currenXmlString;
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
