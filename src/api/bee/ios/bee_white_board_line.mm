#import "bee/ios/bee_white_board_line.h"

@implementation BeeWhiteBoardLine

@synthesize path = _path;

- (instancetype)initWithParam:(CGColorRef)strokeColor
                  strokeWidth:(NSInteger)strokeWidth
                         mode:(BeeDrawingMode)mode
                         path:(NSArray*)path {
    if (self = [super initWithParam:strokeColor
                             width:strokeWidth
                              mode:mode]) {
        _path = path;
    }
    return self;
}

@end
