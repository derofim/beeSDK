#import "bee/ios/bee_white_board_item.h"

@implementation BeeWhiteBoardItem

@synthesize color = _color;
@synthesize width = _width;
@synthesize mode = _mode;

- (instancetype)initWithParam:(CGColorRef)color
                        width:(NSInteger)width
                         mode:(BeeDrawingMode)mode {
    if (self = [super init]) {
        _color = color;
        _width = width;
        _mode = mode;
    }
    return self;
}

@end
