#import "bee/ios/bee_white_board_text.h"

@implementation BeeWhiteBoardText

@synthesize data = _data;
@synthesize pos = _pos;

- (instancetype)initWithParam:(CGColorRef)fontColor
                     fontSize:(NSInteger)fontSize
                         mode:(BeeDrawingMode)mode
                         data:(NSString*)data
                          pos:(CGPoint)pos {
    if (self = [super initWithParam:fontColor
                             width:fontSize
                              mode:mode]) {
        _data = data;
        _pos = pos;
    }
    return self;
}

@end
