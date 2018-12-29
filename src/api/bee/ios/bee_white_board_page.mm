#import "bee/ios/bee_white_board_page.h"
#import "bee/ios/bee_white_board_line.h"
#import "bee/ios/bee_white_board_text.h"
#import "platform/ios/CAShapeLayer+BezierPath.h"
#import <SDWebImage/UIImageView+WebCache.h>
#import <CoreText/CoreText.h>
#pragma mark - BeeWhiteBoardPage.

#define EPSILON 1e-6

@implementation BeeWhiteBoardPage {
    CALayer *_localLayer;
    NSMutableDictionary *_remoteLayers;
    UIImageView *_docView;
    CGSize _targetSize;
    NSMutableArray *_undoItems;
    NSMutableArray *_redoItems;
    BOOL _loaded;
}

@synthesize undoStack = _undoStack;
@synthesize redoStack = _redoStack;
@synthesize backgroundColor = _backgroundColor;
@synthesize src = _src;
@synthesize url = _url;
@synthesize canvasSize = _canvasSize;
@synthesize refPos = _refPos;
@synthesize image = _image;

- (instancetype)init {
    if (self = [super init]) {
        _undoStack = [[NSMutableArray alloc] init];
        _redoStack = [[NSMutableArray alloc] init];
        _undoItems = [[NSMutableArray alloc] init];
        _redoItems = [[NSMutableArray alloc] init];
        _backgroundColor = [UIColor whiteColor];
        _src = nil;
        _url = nil;
        _canvasSize = CGSizeZero;
        _refPos = CGPointZero;
        _targetSize = CGSizeZero;
        _docView = nil;
        _image = nil;
        _localLayer = nil;
        _remoteLayers = [[NSMutableDictionary alloc] init];
        _loaded = NO;
    }
    return self;
}

- (instancetype)initWithParam:(NSString*)src
                          url:(NSString*)url
                   canvasSize:(CGSize)canvasSize
                       refPos:(CGPoint)refPos
                   targetSize:(CGSize)targetSize {
    if (self = [super init]) {
        _undoStack = [[NSMutableArray alloc] init];
        _redoStack = [[NSMutableArray alloc] init];
        _undoItems = [[NSMutableArray alloc] init];
        _redoItems = [[NSMutableArray alloc] init];
        _backgroundColor = [UIColor whiteColor];
        _src = src;
        _url = url;
        _canvasSize = canvasSize;
        _refPos = refPos;
        _targetSize = targetSize;
        [self adaptSizeToCanvas];
        _docView = nil;
        _image = nil;
        _localLayer = nil;
        _remoteLayers = [[NSMutableDictionary alloc] init];
        _loaded = NO;
    }    
    return self;
}

- (void)load:(DocReadyHandler)docHandler loadHandler:(LoadHandler)loadHandler {
    if (_loaded) {
        if (docHandler != nil) {
            docHandler(_image);
        }
        
        if (loadHandler != nil) {
            loadHandler(nil);
        }
        return;
    }
    
    if (_url == nil) {
        [self loadWithoutDoc:loadHandler];
        return;
    }
    
    [self loadWithDoc:docHandler loadHandler:loadHandler];
}

- (void)loadWithDoc:(DocReadyHandler)docHandler loadHandler:(LoadHandler)loadHandler {
    if (_docView == nil) {
        _docView = [[UIImageView alloc] init];
    }
    
    [_docView sd_setImageWithURL:[NSURL URLWithString:_url]
                placeholderImage:nil
                         options:SDWebImageCacheMemoryOnly
                       completed:^(UIImage *image, NSError *error, SDImageCacheType cacheType, NSURL *imageURL) {
                           dispatch_async(dispatch_get_main_queue(), ^{
                               if (error == nil) {
                                   NSLog(@"Doc load from %ld", cacheType);
                                   _image = image;
                                   if (docHandler != nil) {
                                       docHandler(_image);
                                   }
                                   [self loadItems];
                               } else {
                                   NSLog(@"Load image %@ failed with error code:%ld desc:%@", _url, error.code, error.localizedDescription);
                               }
                               if (loadHandler != nil) {
                                   loadHandler(error);
                               }
                           });
                       }
     ];
}

- (void)loadWithoutDoc:(LoadHandler)loadHandler {
    [self loadItems];
    if (loadHandler != nil) {
        loadHandler(nil);
    }
}

- (void)loadItems {
    for (BeeWhiteBoardItem *item in _undoItems) {
        [self loadItemToStack:item stack:eBeeWhiteBoardStack_Undo];
    }
    
    for (BeeWhiteBoardItem *item in _redoItems) {
        [self loadItemToStack:item stack:eBeeWhiteBoardStack_Redo];
    }
    
    _loaded = YES;
}

- (void)loadItemToStack:(BeeWhiteBoardItem*)item
                  stack:(BeeWhiteBoardStack)stack {
    if (item == nil) {
        return;
    }
    
    switch (item.mode) {
        case bee::eBeeDrawingMode_Text:
            [self loadTextToStack:(BeeWhiteBoardText*)item stack:stack];
            break;
        default:
            [self loadLineToStack:(BeeWhiteBoardLine*)item stack:stack];
            break;
    }
}

- (void)loadLineToStack:(BeeWhiteBoardLine*)line
                  stack:(BeeWhiteBoardStack)stack {
    if (line.mode == eBeeDrawingMode_Laser) {
        return;
    }
    
    if (line == nil || line.path == nil || line.path.count < 2) {
        return;
    }
    
    CGPoint startPoint = CGPointMake([[line.path objectAtIndex:0] doubleValue], [[line.path objectAtIndex:1] doubleValue]);
    startPoint = [self adaptPointToResolution:startPoint];
    UIBezierPath *path = [self bezierPathWithLineWidth:line.width startPoint:startPoint];
    CAShapeLayer *layer = [CAShapeLayer layer];
    
    layer.bezierPath = path;
    layer.path = path.CGPath;
    layer.backgroundColor = [UIColor clearColor].CGColor;
    layer.fillColor = [UIColor clearColor].CGColor;
    layer.lineCap = kCALineCapRound;
    layer.lineJoin = kCALineJoinRound;
    layer.lineWidth = line.width;
    if (line.mode == eBeeDrawingMode_Pen) {
        layer.strokeColor = line.color;
    } else if (line.mode == eBeeDrawingMode_Eraser) {
        layer.strokeColor = _backgroundColor.CGColor;
    }
    
    NSUInteger maxCount = line.path.count;
    if (maxCount % 2 != 0) {
        --maxCount;
    }
    
    for (NSUInteger i = 0; i < maxCount; i += 2) {
        CGPoint point = CGPointMake([[line.path objectAtIndex:i] doubleValue], [[line.path objectAtIndex:i + 1] doubleValue]);
        point = [self adaptPointToResolution:point];
        [path addLineToPoint:point];
        layer.path = path.CGPath;
    }
    
    if (stack == eBeeWhiteBoardStack_Undo) {
        [self pushUndo:layer];
    } else if (stack == eBeeWhiteBoardStack_Redo) {
        [self pushRedo:layer];
    }
}

- (void)loadTextToStack:(BeeWhiteBoardText*)text
                  stack:(BeeWhiteBoardStack)stack {
    if (text == nil) {
        return;
    }
    
    CATextLayer *layer = [[CATextLayer alloc] init];
    layer.string = text.data;
    layer.backgroundColor = [UIColor clearColor].CGColor;
    layer.contentsScale = [UIScreen mainScreen].scale;
    layer.wrapped = NO;
    layer.truncationMode = kCATruncationNone;
    layer.foregroundColor = text.color;
    layer.alignmentMode = kCAAlignmentLeft;
    
    UIFont *font = [UIFont fontWithName:@"Arial" size:[self pxSizeToFontSize:text.width]];
    CFStringRef fontName = (__bridge CFStringRef)font.fontName;
    CGFontRef fontRef = CGFontCreateWithFontName(fontName);
    layer.font = fontRef;
    layer.fontSize = font.pointSize;
    CGSize textSize = [text.data sizeWithAttributes:@{NSFontAttributeName:font}];
    CGPoint pos = [self adaptPointToResolution:text.pos];
    layer.frame = CGRectMake(pos.x, pos.y, ceil(textSize.width), ceil(textSize.height));
    CGFontRelease(fontRef);
    
    if (stack == eBeeWhiteBoardStack_Undo) {
        [self pushUndo:layer];
    } else if (stack == eBeeWhiteBoardStack_Redo) {
        [self pushRedo:layer];
    }
}

- (CGFloat)pxSizeToFontSize:(CGFloat)pxSize {
    //96 is pixel per inch for css, 72 is point per inch for ios.
    CGFloat pt = pxSize * 72 / 96;
    return pt;
}

- (void)pushUndo:(CALayer *)layer {
    [_undoStack addObject:layer];
}

-(CALayer *)popUndo {
    if (_undoStack.count == 0) {
        return nil;
    }
    
    NSInteger lastObjIndex = _undoStack.count - 1;
    CALayer *lastObj = [_undoStack objectAtIndex:lastObjIndex];
    [_undoStack removeObjectAtIndex:lastObjIndex];
    return lastObj;
}

- (void)pushRedo:(CALayer*)layer {
    [_redoStack addObject:layer];
}

- (CALayer *)popRedo {
    if (_redoStack.count == 0) {
        return nil;
    }
    
    NSInteger lastObjIndex = _redoStack.count - 1;
    CALayer *lastObj = [_redoStack objectAtIndex:lastObjIndex];
    [_redoStack removeObjectAtIndex:lastObjIndex];
    return lastObj;
}

- (void)clearUndoStack {
    [_undoStack makeObjectsPerformSelector:@selector(removeFromSuperlayer)];
    [_undoStack removeAllObjects];
}

- (void)clearRedoStack {
    [_redoStack makeObjectsPerformSelector:@selector(removeFromSuperlayer)];
    [_redoStack removeAllObjects];
}

- (void)cacheItem:(BeeWhiteBoardItem*)item
            stack:(BeeWhiteBoardStack)stack {
    switch (stack) {
        case eBeeWhiteBoardStack_Undo:
            [_undoItems addObject:item];
            break;
        case eBeeWhiteBoardStack_Redo:
            [_redoItems insertObject:item atIndex:0];
        default:
            break;
    }
}

- (CGPoint)adaptPointToResolution:(CGPoint)point {
    if (_canvasSize.width == 0 || _canvasSize.height == 0 || _targetSize.width == 0 || _targetSize.height == 0) {
        return point;
    }
    
    CGFloat x = (CGFloat)(point.x + _refPos.x) * _targetSize.width / _canvasSize.width;
    CGFloat y = (CGFloat)(point.y + _refPos.y) * _targetSize.height / _canvasSize.height;
    CGPoint newPoint = CGPointMake(x, y);
    return newPoint;
}

-(UIBezierPath * )bezierPathWithLineWidth:(CGFloat)width startPoint:(CGPoint)startP {
    UIBezierPath *path = [[UIBezierPath alloc] init];
    path.lineWidth = width;
    path.lineCapStyle = kCGLineCapRound;
    path.lineJoinStyle = kCGLineJoinRound;
    [path moveToPoint:startP];
    return path;
}

- (void)adaptSizeToCanvas {
    if (_canvasSize.width == 0 || _canvasSize.height == 0) {
        return;
    }

    CGFloat width = _targetSize.width;
    CGFloat height = _targetSize.height;
    CGFloat canvasAspectRatio = _canvasSize.width / _canvasSize.height;
    CGFloat localAspectRatio = width / height;

    //The same aspect ratio.
    if (fabs(canvasAspectRatio - localAspectRatio) <= EPSILON) {
        return;
    }

    //Get max show area.
    CGFloat fixedWidthHeight = width * _canvasSize.height / _canvasSize.width;
    CGFloat fixedHeightWidth = height * _canvasSize.width / _canvasSize.height;
    if (fixedWidthHeight <= height) {
        height = fixedWidthHeight;
    } else {
        width = fixedHeightWidth;
    }
    _targetSize.width = width;
    _targetSize.height = height;
}

#pragma mark - CALayer Storage.

- (void)refLayer:(BOOL)local
          drawer:(NSString*)drawer
           layer:(CALayer*)layer {
    do {
        if (layer == nil) {
            break;
        }
        
        if (local) {
            _localLayer = layer;
            break;
        }
        
        if (drawer == nil) {
            break;
        }
        
        [_remoteLayers setValue:layer forKey:drawer];
    } while (false);
}

- (CALayer*)getLayer:(BOOL)local
              drawer:(NSString*)drawer {
    CALayer *layer = nil;
    do {
        if (local) {
            layer = _localLayer;
            break;
        }
        
        if (drawer == nil) {
            break;
        }
        
        layer = [_remoteLayers valueForKey:drawer];
    } while (false);
    return layer;
}

- (void)unrefLayer:(BOOL)local
            drawer:(NSString*)drawer {
    do {
        if (local) {
            _localLayer = nil;
        }
        
        if (drawer == nil) {
            break;
        }
        
        [_remoteLayers removeObjectForKey:drawer];
    } while (false);
}

@end
