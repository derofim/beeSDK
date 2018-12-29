#import "bee/ios/bee_white_board_view.h"
#import "bee/ios/bee_white_board_page.h"
#import "platform/ios/CAShapeLayer+BezierPath.h"

#import <AVFoundation/AVFoundation.h>
#import <AVKit/AVKit.h>

const CGFloat defaultLineWidth = 3.0;
const int defaultLineColor = 0;
#define EPSILON 1e-6

@implementation BeeWhiteBoardView {
    __weak id<BeeWhiteBoardViewDelegate> _whiteBoardDelegate;
    NSMutableArray *_lines;
    UIColor *_lineColor;
    CGFloat _lineWidth;
    CGPoint _lastPoint;
    UIView *_laserView;
    NSArray *_pages;
    NSInteger _currentPageIndex;
    BeeWhiteBoardPage *_currentPage;
    UIImageView *_docView;
    BeeDrawingMode _drawingMode;
    UIColor *_drawingColor;
    CGFloat _drawingWidth;
    CGRect _oriFrame;
    CGAffineTransform _oriTransform;
    BOOL _localDrawing;
    BOOL _lockDrawing;
    AVPlayer *_avPlayer;
    AVPlayerLayer *_avPlayerLayer;
    id _playTimeObserver;
    CGSize _canvasSize;
    CGPoint _refPos;
}

#pragma mark - Public method
- (instancetype)initWithFrame:(CGRect)frame {
    if (self = [super initWithFrame:frame]) {
        self.backgroundColor = [UIColor whiteColor];
        self.layer.borderWidth = 1;
        self.layer.borderColor = UIColorFromRGB(0xd7dade).CGColor;
        [self setDrawingMode:eBeeDrawingMode_Pen];
        
        _lines = [NSMutableArray array];
        _lineColor = UIColorFromRGB(defaultLineColor);
        _lineWidth = defaultLineWidth;
        _drawingColor = _lineColor;
        _drawingWidth = _lineWidth;
        _drawingMode = eBeeDrawingMode_Pen;
        
        _laserView = [[UIView alloc]initWithFrame:CGRectZero];
        CGRect frame = _laserView.frame;
        frame.size.width = 7;
        frame.size.height = 7;
        _laserView.frame = frame;
        _laserView.backgroundColor = [UIColor redColor];
        _laserView.layer.cornerRadius = 3.5;
        _laserView.layer.masksToBounds = YES;
        [_laserView setHidden:YES];
        [self addSubview:_laserView];
        
        _pages = nil;
        _currentPageIndex = -1;
        _currentPage = [[BeeWhiteBoardPage alloc] init];
        
        _docView = [[UIImageView alloc] initWithFrame:self.bounds];
        UIPinchGestureRecognizer *pinchRecognizer = [[UIPinchGestureRecognizer alloc] initWithTarget:self action:@selector(onPinchAction:)];
        [self addGestureRecognizer:pinchRecognizer];
        
        UIPanGestureRecognizer *panRecognizer = [[UIPanGestureRecognizer alloc] initWithTarget:self action:@selector(onPanAction:)];
        [panRecognizer setMinimumNumberOfTouches:2];
        [panRecognizer setMaximumNumberOfTouches:3];
        [self addGestureRecognizer:panRecognizer];
        _oriFrame = self.frame;
        _oriTransform = self.transform;
        _localDrawing = NO;
        _lockDrawing = NO;
        _avPlayer = nil;
        _avPlayerLayer = nil;
        _playTimeObserver = nil;
        _canvasSize = CGSizeZero;
        _refPos = CGPointZero;
    }
    return self;
}

- (void)setWhiteBoardDelegate:(id<BeeWhiteBoardViewDelegate>)delegate {
    _whiteBoardDelegate = delegate;
}

- (void)setBoardInfo:(NSArray<BeeWhiteBoardPage*>*)pages
    currentPageIndex:(NSInteger)currentPageIndex
          canvasSize:(CGSize)canvasSize
              refPos:(CGPoint)refPos {
    dispatch_async(dispatch_get_main_queue(), ^{
        _pages = pages;
        _currentPageIndex = currentPageIndex;
        [self adaptSizeToCanvas:canvasSize refPos:refPos];
        if (pages == nil || pages.count == 0 || currentPageIndex >= pages.count) {
            [self clearBackground];
        } else {
            [self loadFromPage:_currentPageIndex];
        }
    });
}

- (void)nextPage {
    dispatch_async(dispatch_get_main_queue(), ^{
        if (_pages != nil && _currentPageIndex + 1 < _pages.count) {
            _currentPageIndex++;
            [self loadFromPage:_currentPageIndex];
        }
    });
}

- (void)prePage {
    dispatch_async(dispatch_get_main_queue(), ^{
        if (_pages != nil && _currentPageIndex - 1 >= 0) {
            _currentPageIndex--;
            [self loadFromPage:_currentPageIndex];
        }
    });
}

- (void)clearScreen {
    dispatch_async(dispatch_get_main_queue(), ^{
        [self innerClearScreen:YES];
    });
}

- (void)undo {
    dispatch_async(dispatch_get_main_queue(), ^{
        NSLog(@"Undo on page %ld", _currentPageIndex);
        CALayer *layer = [_currentPage popUndo];
        if (layer != nil) {
            [layer removeFromSuperlayer];
            [_currentPage pushRedo:layer];
        }
    });
}

- (void)redo {
    dispatch_async(dispatch_get_main_queue(), ^{
        NSLog(@"Redo on page %ld", _currentPageIndex);
        CALayer *layer = [_currentPage popRedo];
        if (layer != nil) {
            [self.layer addSublayer:layer];
            [_currentPage pushUndo:layer];
        }
    });
}

- (void)lineBegin:(CGPoint)point
      strokeColor:(CGColorRef)strokeColor
      strokeWidth:(CGFloat)strokeWidth
             mode:(BeeDrawingMode)mode
           drawer:(NSString*)drawer {
    dispatch_async(dispatch_get_main_queue(), ^{
        [self onLineBegin:point
              strokeColor:strokeColor
              strokeWidth:strokeWidth
                     mode:mode
                    local:NO
                   drawer:drawer];
    });
}

- (void)lineMove:(CGPoint)point
            mode:(BeeDrawingMode)mode
          drawer:(NSString*)drawer {
    dispatch_async(dispatch_get_main_queue(), ^{
        [self onLineMove:point
                    mode:mode
                   local:NO
                  drawer:drawer];
    });
}

- (void)lineEnd:(NSString*)drawer {
    dispatch_async(dispatch_get_main_queue(), ^{
        [self onLineEnd:NO drawer:drawer];
    });
}

- (void)drawText:(NSString*)text
             pos:(CGPoint)point
           color:(CGColorRef)color
            size:(CGFloat)size
          drawer:(NSString*)drawer {
    dispatch_async(dispatch_get_main_queue(), ^{
        CATextLayer *textLayer = [[CATextLayer alloc] init];
        textLayer.string = text;
        textLayer.backgroundColor = [UIColor clearColor].CGColor;
        textLayer.contentsScale = [UIScreen mainScreen].scale;
        textLayer.wrapped = NO;
        textLayer.truncationMode = kCATruncationNone;
        textLayer.foregroundColor = color;
        textLayer.alignmentMode = kCAAlignmentLeft;

        UIFont *font = [UIFont fontWithName:@"Arial" size:[self pxSizeToFontSize:size]];
        CFStringRef fontName = (__bridge CFStringRef)font.fontName;
        CGFontRef fontRef = CGFontCreateWithFontName(fontName);
        textLayer.font = fontRef;
        textLayer.fontSize = font.pointSize;
        CGSize textSize = [text sizeWithAttributes:@{NSFontAttributeName:font}];
        CGPoint pos = [self adaptPointToLocalResolution:point];
        textLayer.frame = CGRectMake(pos.x, pos.y, ceil(textSize.width), ceil(textSize.height));
        CGFontRelease(fontRef);

        [self.layer addSublayer:textLayer];
        [_lines addObject:textLayer];
        [_currentPage clearRedoStack];
        [_currentPage pushUndo:textLayer];
    });
}

- (CGFloat)pxSizeToFontSize:(CGFloat)pxSize {
    //96 is pixel per inch for css, 72 is point per inch for ios.
    CGFloat pt = pxSize * 72 / 96;
    return pt;
}

#pragma mark - System Delegate
- (void)touchesBegan:(NSSet<UITouch *> *)touches withEvent:(UIEvent *)event {
    //NSLog(@"touchesBegan %d", [event allTouches].count);
    CGPoint startP = [self pointWithTouchs:touches];
    if ([event allTouches].count > 1 || _lockDrawing) {
        [self.superview touchesBegan:touches withEvent:event];
    } else {
        [self onLineBegin:startP
              strokeColor:_drawingColor.CGColor
              strokeWidth:_drawingWidth
                     mode:_drawingMode
                    local:YES
                   drawer:nil];
        _lastPoint = startP;
    }
}

- (void)touchesMoved:(NSSet<UITouch *> *)touches withEvent:(UIEvent *)event {
    //NSLog(@"touchesMoved %d", [event allTouches].count);
    if ([event allTouches].count > 1 || _lockDrawing) {
        [self.superview touchesMoved:touches withEvent:event];
    } else if ([event allTouches].count == 1) {
        CGPoint moveP = [self pointWithTouchs:touches];
        [self onLineMove:moveP
                    mode:_drawingMode
                   local:YES
                  drawer:nil];
        if (_whiteBoardDelegate != nil) {
            CGPoint fromPoint = [self adaptPointToRemoteResolution:_lastPoint];
            CGPoint toPoint = [self adaptPointToRemoteResolution:moveP];
            [_whiteBoardDelegate onDrawingLine:fromPoint
                                 end:toPoint
                               color:_drawingColor.CGColor
                               width:_drawingWidth
                                mode:_drawingMode];
        }
        _lastPoint = moveP;
        _localDrawing = YES;
    }
}

- (void)touchesEnded:(NSSet<UITouch *> *)touches withEvent:(UIEvent *)event {
    //NSLog(@"touchesEnded %d", [event allTouches].count);
    if ([event allTouches].count > 1 || _lockDrawing){
        [self.superview touchesMoved:touches
                           withEvent:event];
    } else if ([event allTouches].count == 1) {
        CGPoint moveP = [self pointWithTouchs:touches];
        [self onLineMove:moveP
                    mode:_drawingMode
                   local:YES
                  drawer:nil];
        if (_whiteBoardDelegate != nil) {
            CGPoint fromPoint = [self adaptPointToRemoteResolution:_lastPoint];
            CGPoint toPoint = [self adaptPointToRemoteResolution:moveP];
            [_whiteBoardDelegate onDrawingLine:fromPoint
                                           end:toPoint
                                         color:_drawingColor.CGColor
                                         width:_drawingWidth
                                          mode:_drawingMode];
        }
        _lastPoint = moveP;
        _localDrawing = YES;
    }
    
    if (_localDrawing) {
        [self onLineEnd:YES
                 drawer:nil];
        if (_whiteBoardDelegate != nil) {
            [_whiteBoardDelegate onDrawLineEnd];
        }
        _localDrawing = NO;
    }
    

}

- (void)adjustAnchorPointForGestureRecognizer:(UIGestureRecognizer *)gestureRecognizer {
    if (gestureRecognizer.state == UIGestureRecognizerStateBegan) {
        UIView *piece = gestureRecognizer.view;
        CGPoint locationInView = [gestureRecognizer locationInView:piece];
        CGPoint locationInSuperview = [gestureRecognizer locationInView:piece.superview];

        piece.layer.anchorPoint = CGPointMake(locationInView.x / piece.bounds.size.width, locationInView.y / piece.bounds.size.height);
        piece.center = locationInSuperview;
    }
}

- (void)onPinchAction:(UIPinchGestureRecognizer*)recognizer {
    if (_localDrawing) {
        [self onLineEnd:YES
                 drawer:nil];
        if (_whiteBoardDelegate != nil) {
            [_whiteBoardDelegate onDrawLineEnd];
        }
        _localDrawing = NO;
    }
    
    [self adjustAnchorPointForGestureRecognizer:recognizer];

    if ([recognizer state] == UIGestureRecognizerStateBegan || [recognizer state] == UIGestureRecognizerStateChanged) {
        [recognizer view].transform = CGAffineTransformScale([[recognizer view] transform], [recognizer scale], [recognizer scale]);
        [recognizer setScale:1];
    }
}

- (void)onPanAction:(UIPanGestureRecognizer *)recognizer {
    if (_localDrawing) {
        [self onLineEnd:YES
                 drawer:nil];
        if (_whiteBoardDelegate != nil) {
            [_whiteBoardDelegate onDrawLineEnd];
        }
        _localDrawing = NO;
    }
    
    CGPoint point = [recognizer translationInView:self.superview];
    recognizer.view.center = CGPointMake(recognizer.view.center.x + point.x, recognizer.view.center.y + point.y);
    [recognizer setTranslation:CGPointMake(0, 0) inView:self];
}

#pragma mark - Inner drawing method
- (void)onLineBegin:(CGPoint)point
        strokeColor:(CGColorRef)strokeColor
        strokeWidth:(CGFloat)strokeWidth
               mode:(BeeDrawingMode)mode
              local:(BOOL)local
             drawer:(NSString*)drawer {
    if (mode == eBeeDrawingMode_Laser) {
        return;
    }
    
    if (!local) {
        point = [self adaptPointToLocalResolution:point];
    }
    
    UIBezierPath *path = [self paintPathWithLineWidth:strokeWidth startPoint:point];
    CAShapeLayer *layer = [CAShapeLayer layer];

    layer.bezierPath = path;
    layer.path = path.CGPath;
    layer.backgroundColor = [UIColor clearColor].CGColor;
    layer.fillColor = [UIColor clearColor].CGColor;
    layer.lineCap = kCALineCapRound;
    layer.lineJoin = kCALineJoinRound;
    if (mode == eBeeDrawingMode_Pen) {
        layer.strokeColor = strokeColor;
    } else if (mode == eBeeDrawingMode_Eraser) {
        layer.strokeColor = self.backgroundColor.CGColor;
    }
    
    layer.lineWidth = strokeWidth;
    [self.layer addSublayer:layer];
    [_lines addObject:layer];
    [_currentPage clearRedoStack];
    [_currentPage refLayer:local drawer:drawer layer:layer];
}

- (void)onLineMove:(CGPoint)point
              mode:(BeeDrawingMode)mode
             local:(BOOL)local
            drawer:(NSString*)drawer {
    if (!local) {
        point = [self adaptPointToLocalResolution:point];
    }
    
    if (mode == eBeeDrawingMode_Laser) {
        [_laserView setHidden:NO];
        _laserView.center = point;
    } else {
        CAShapeLayer *layer = (CAShapeLayer*)[_currentPage getLayer:local drawer:drawer];
        if (layer != nil) {
            UIBezierPath *path = layer.bezierPath;
            if (path != nil) {
                [path addLineToPoint:point];
                layer.path = path.CGPath;
            }
        }
    }
    
    if (_laserView != nil) {
        [self bringSubviewToFront:_laserView];
    }
}

- (void)onLineEnd:(BOOL)local
           drawer:(NSString*)drawer {
    CAShapeLayer *layer = (CAShapeLayer*)[_currentPage getLayer:local drawer:drawer];
    if (layer != nil) {
        [_currentPage pushUndo:layer];
    }
    
    if (_laserView != nil) {
        [_laserView setHidden:YES];
    }
    
    [_currentPage unrefLayer:local drawer:drawer];
}

#pragma mark - Private method
-(UIBezierPath * )paintPathWithLineWidth:(CGFloat)width startPoint:(CGPoint)startP {
    UIBezierPath *path = [[UIBezierPath alloc]init];
    path.lineWidth = width;
    path.lineCapStyle = kCGLineCapRound;
    path.lineJoinStyle = kCGLineJoinRound;
    [path moveToPoint:startP];
    return path;
}

- (CGPoint )pointWithTouchs:(NSSet *)touches {
    UITouch *touch = [touches anyObject];
    return [touch locationInView:self];
}

- (void)loadFromPage:(NSInteger)pageIndex {
    if (_pages != nil && _pages.count > 0 && pageIndex >= 0 && pageIndex < _pages.count) {
        __block BeeWhiteBoardPage *whiteBoardPage = [_pages objectAtIndex:pageIndex];
        if (whiteBoardPage == nil) {
            NSLog(@"Invalid page index %ld", pageIndex);
            return;
        }
        
        DocReadyHandler docHandler = [^(UIImage *image) {
            if (image != nil) {
                //清除背景。
                [self clearBackground];
                //创建一个新的Context。
                UIGraphicsBeginImageContext(self.frame.size);
                //获得当前Context
                CGContextRef context = UIGraphicsGetCurrentContext();
                //CTM变换，调整坐标系，*重要*，否则橡皮擦使用的背景图片会发生翻转。
                CGContextScaleCTM(context, 1, -1);
                CGContextTranslateCTM(context, 0, -self.bounds.size.height);
                //背景底色用白色矩形填充。
                [[UIColor whiteColor] set];
                UIRectFill(self.frame);
                //获得背景图片的矩形，按照原比例居中。
                CGRect rect = [self getDocDrawRect:image];
                //绘制背景图片，。
                [image drawInRect:rect];
                //获取Context中绘制的整体图片，也就是底层用白色矩形填充，上面又按照原比例画了一层图片，由于图片
                //宽高比和View宽高比不一样，在上下或者左右两侧会有留白。
                UIImage *stretchedImg = UIGraphicsGetImageFromCurrentImageContext();
                //为橡皮擦设置背景色。
                [self setBackgroundColor:[[UIColor alloc] initWithPatternImage:stretchedImg]];
                //View的图层设置为原始图片，这里会自动翻转，经过这步后图层显示和橡皮背景都设置为正确的图片。
                self.layer.contents = (_Nullable id)image.CGImage;
                //居中不拉伸。
                self.layer.contentsGravity = kCAGravityResizeAspect;
                //白板页的背景也设置为该背景。
                whiteBoardPage.backgroundColor = self.backgroundColor;
                //结束绘制。
                UIGraphicsEndImageContext();
            } else {
                //Set background color to white color.
                [self setBackgroundColor:[UIColor whiteColor]];
            }
        } copy];
        
        LoadHandler loadHandler = [^(NSError *error) {
            if (error == nil) {
                NSLog(@"Page %ld %@ load success.", pageIndex, whiteBoardPage.url);
            } else {
                NSLog(@"Page %ld load fail.", pageIndex);
            }
            
            [self innerClearScreen:NO];
            _currentPage = whiteBoardPage;
            for (CALayer *layer in _currentPage.undoStack) {
                [self.layer addSublayer:layer];
                [_lines addObject:layer];
                [_currentPage refLayer:NO drawer:whiteBoardPage.src layer:layer];
            }            
        } copy];
        
        [whiteBoardPage load:docHandler loadHandler:loadHandler];
    }
}

- (CGRect)getDocDrawRect:(UIImage*)image {
    CGRect frame = self.frame;
    CGFloat width = frame.size.width;
    CGFloat height = frame.size.height;
    CGFloat xOffset = 0.0f;
    CGFloat yOffset = 0.0f;

    //Get max show area.
    if (image != nil && image.size.width > 0 && image.size.height > 0) {
        CGFloat fixedWidthHeight = width * image.size.height / image.size.width;
        CGFloat fixedHeightWidth = height * image.size.width/ image.size.height;
        if (fixedWidthHeight <= height) {
            yOffset = (height - fixedWidthHeight) / 2;
            height = fixedWidthHeight;
        } else {
            xOffset = (width - fixedHeightWidth) / 2;
            width = fixedHeightWidth;
        }
    }

    CGRect rect = CGRectMake(xOffset, yOffset, width, height);
    return rect;
}

- (void)clearBackground {
    [self setBackgroundColor:[UIColor whiteColor]];
    self.layer.contents = nil;
}

- (void)innerClearScreen:(BOOL)clearContext {
    if (_lines.count > 0) {
        [_lines makeObjectsPerformSelector:@selector(removeFromSuperlayer)];
        [_lines removeAllObjects];
    }
    
    if (clearContext) {
        [_currentPage clearUndoStack];
        [_currentPage clearRedoStack];
    }
}

- (CGPoint)adaptPointToLocalResolution:(CGPoint)point {
    if (_canvasSize.width == 0 || _canvasSize.height == 0) {
        return point;
    }
    
    CGFloat x = (CGFloat)(point.x + _refPos.x) * self.bounds.size.width / _canvasSize.width;
    CGFloat y = (CGFloat)(point.y + _refPos.y) * self.bounds.size.height / _canvasSize.height;
    CGPoint newPoint = CGPointMake(x, y);
    return newPoint;
}

- (CGPoint)adaptPointToRemoteResolution:(CGPoint)point {
    if (_canvasSize.width == 0 || _canvasSize.height == 0) {
        return point;
    }
    
    CGFloat x = (CGFloat)point.x * _canvasSize.width / self.bounds.size.width - _refPos.x;
    CGFloat y = (CGFloat)point.y * _canvasSize.height / self.bounds.size.height - _refPos.y;
    CGPoint newPoint = CGPointMake(x, y);
    return newPoint;
}

- (CGSize)adaptSizeToLocalResolution:(CGSize)size {
    if (_canvasSize.width == 0 || _canvasSize.height == 0) {
        return size;
    }
    
    CGFloat width = (CGFloat)size.width * self.bounds.size.width / _canvasSize.width;
    CGFloat height = (CGFloat)size.height * self.bounds.size.height / _canvasSize.height;
    CGSize newSize = CGSizeMake(width, height);
    return newSize;
}

- (void)adaptSizeToCanvas:(CGSize)canvasSize
                   refPos:(CGPoint)refPos {
    if (canvasSize.width == 0 || canvasSize.height == 0) {
        return;
    }

    _canvasSize = canvasSize;
    _refPos = refPos;

    CGRect frame = self.frame;
    CGFloat width = frame.size.width;
    CGFloat height = frame.size.height;
    CGFloat xOffset = 0.0f;
    CGFloat yOffset = 0.0f;

    CGFloat canvasAspectRatio = canvasSize.width / canvasSize.height;
    CGFloat localAspectRatio = width / height;

    //The same aspect ratio.
    if (fabs(canvasAspectRatio - localAspectRatio) <= EPSILON) {
        return;
    }

    //Get max show area.
    CGFloat fixedWidthHeight = width * canvasSize.height / canvasSize.width;
    CGFloat fixedHeightWidth = height * canvasSize.width / canvasSize.height;
    if (fixedWidthHeight <= height) {
        yOffset = (height - fixedWidthHeight) / 2;
        height = fixedWidthHeight;
    } else {
        xOffset = (width - fixedHeightWidth) / 2;
        width = fixedHeightWidth;
    }
    frame.origin.x += xOffset;
    frame.origin.y += yOffset;
    frame.size.width = width;
    frame.size.height = height;
    self.frame = frame;
    _oriFrame = frame;
    _oriTransform = self.transform;
    CGRect bounds = self.bounds;
    bounds.size.width = width;
    bounds.size.height = height;
    self.bounds = bounds;
}

#pragma mark - Set/Get
- (void)setDrawingMode:(BeeDrawingMode)drawingMode {
    dispatch_async(dispatch_get_main_queue(), ^{
        _drawingMode = drawingMode;
        [_laserView setHidden:YES];
        switch (drawingMode) {
            case eBeeDrawingMode_Pen: {
                _drawingWidth = _lineWidth;
                _drawingColor = _lineColor;
                break;
            }
            case eBeeDrawingMode_Eraser: {
                _drawingWidth = _lineWidth * 2.5;
                _drawingColor = self.backgroundColor;
                break;
            }
            case eBeeDrawingMode_None: {
                _drawingWidth = _lineWidth * 0;
                _drawingColor = [UIColor clearColor];
                break;
            }
            default:
                break;
        }
    });
}

- (BeeDrawingMode)getDrawingMode {
    return _drawingMode;
}

- (void)setLineColor:(int)rgbColor {
    dispatch_async(dispatch_get_main_queue(), ^{
        _lineColor = UIColorFromRGB(rgbColor);
        _drawingColor = _lineColor;
    });
}

- (UIColor*)getLineColor {
    return _lineColor;
}

- (void)setLineWidth:(CGFloat)width {
    dispatch_async(dispatch_get_main_queue(), ^{
        _lineWidth = width;
    });
}

- (CGFloat)getLineWidth {
    return _lineWidth;
}

- (void)lockDrawing {
    dispatch_async(dispatch_get_main_queue(), ^{
        _lockDrawing = YES;
    });
}

- (void)unlockDrawing {
    dispatch_async(dispatch_get_main_queue(), ^{
        _lockDrawing = NO;
    });
}

- (void)resetFrame {
    dispatch_async(dispatch_get_main_queue(), ^{
        self.transform = _oriTransform;
        self.frame = _oriFrame;
    });
}

- (void)playVideo:(NSString*)url
              pos:(CGPoint)pos
             size:(CGSize)size {
    dispatch_async(dispatch_get_main_queue(), ^{
        if (_avPlayer == nil) {
            _avPlayer = [[AVPlayer alloc] init];
        }

        NSURL *nsUrl = [NSURL URLWithString:url];
        AVPlayerItem *playerItem = [[AVPlayerItem alloc] initWithURL:nsUrl];
        [_avPlayer replaceCurrentItemWithPlayerItem:playerItem];

        if (_avPlayerLayer == nil) {
            _avPlayerLayer = [AVPlayerLayer playerLayerWithPlayer:_avPlayer];
            _avPlayerLayer.videoGravity = AVLayerVideoGravityResizeAspect;
            [self.layer addSublayer:_avPlayerLayer];
        }

        CGPoint localPos = [self adaptPointToLocalResolution:pos];
        CGSize localSize = [self adaptSizeToLocalResolution:size];
        _avPlayerLayer.frame = CGRectMake(localPos.x, localPos.y, localSize.width, localSize.height);        
        
        [self addObserverForCurrentPlayItem];
    });
}

- (void)stopVideo {
    dispatch_async(dispatch_get_main_queue(), ^{
        if (_avPlayer != nil) {
            [_avPlayer seekToTime:CMTimeMake(0, 1)];
            [_avPlayer pause];
            [self removeObserverForCurrentPlayItem];
            _avPlayer = nil;
            [_avPlayerLayer removeFromSuperlayer];
            _avPlayerLayer = nil;
        }
    });
}

- (void)pauseVideo {
    dispatch_async(dispatch_get_main_queue(), ^{
        if (_avPlayer != nil) {
            [_avPlayer pause];
        }
    });
}

- (void)resumeVideo {
    dispatch_async(dispatch_get_main_queue(), ^{
        if (_avPlayer != nil) {
            [_avPlayer play];
        }
    });
}

- (void)addObserverForCurrentPlayItem {
    if (_avPlayer == nil || _avPlayer.currentItem == nil) {
        return;
    }

    [_avPlayer.currentItem addObserver:self
                            forKeyPath:@"status"
                               options:(NSKeyValueObservingOptionOld | NSKeyValueObservingOptionNew)
                               context:nil];
    [_avPlayer.currentItem addObserver:self
                            forKeyPath:@"loadedTimeRanges"
                               options:NSKeyValueObservingOptionOld | NSKeyValueObservingOptionNew
                               context:nil];
    [_avPlayer.currentItem addObserver:self
                            forKeyPath:@"playbackBufferEmpty"
                               options:NSKeyValueObservingOptionNew
                               context:nil];
    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(playbackFinished:)
                                                 name:AVPlayerItemDidPlayToEndTimeNotification
                                               object:_avPlayer.currentItem];
    /* Start a timer to get play time.
    _playTimeObserver = [_avPlayer addPeriodicTimeObserverForInterval:CMTimeMake(1, 1)
                                                                queue:dispatch_get_main_queue()
                                                           usingBlock:^(CMTime time) {
                                                               NSLog(@"Video loaded %lf", CMTimeGetSeconds(time));
                                                           }];
    */
}

- (void)removeObserverForCurrentPlayItem {
    if (_avPlayer == nil || _avPlayer.currentItem == nil) {
        return;
    }

    [_avPlayer.currentItem removeObserver:self forKeyPath:@"status"];
    [_avPlayer.currentItem removeObserver:self forKeyPath:@"loadedTimeRanges"];
    [_avPlayer.currentItem removeObserver:self forKeyPath:@"playbackBufferEmpty"];
    [[NSNotificationCenter defaultCenter] removeObserver:self
                                                    name:AVPlayerItemDidPlayToEndTimeNotification
                                                  object:nil];
    //[_avPlayer removeTimeObserver:_playTimeObserver];
}

#pragma mark - KVO
    
- (void)observeValueForKeyPath:(NSString *)keyPath
                      ofObject:(id)object
                        change:(NSDictionary<NSKeyValueChangeKey,id> *)change
                       context:(void *)context {
    AVPlayerItem *playerItem = object;
    if ([keyPath isEqualToString:@"status"]) {
        AVPlayerItemStatus status = (AVPlayerItemStatus)[change[@"new"] integerValue];
        switch (status) {
            case AVPlayerItemStatusReadyToPlay: {
                NSLog(@"AVPlayerItemStatusReadyToPlay");
                [_avPlayer play];
                break;
            }
            case AVPlayerItemStatusFailed: {
                NSLog(@"AVPlayerItemStatusFailed");
                break;
            }
            case AVPlayerItemStatusUnknown: {
                NSLog(@"AVPlayerItemStatusUnknown");
                break;
            }
            default: {
                break;
            }
        }
    } else if ([keyPath isEqualToString:@"loadedTimeRanges"]) {
        NSArray *array = playerItem.loadedTimeRanges;
        CMTimeRange timeRange = [array.firstObject CMTimeRangeValue];
        float startSeconds = CMTimeGetSeconds(timeRange.start);
        float durationSeconds = CMTimeGetSeconds(timeRange.duration);
        NSTimeInterval totalBuffer = startSeconds + durationSeconds;
        NSLog(@"Loaded：%.2f", totalBuffer);
    } else if ([keyPath isEqualToString:@"playbackBufferEmpty"]) {
        NSLog(@"playbackBufferEmpty");
    }
}

- (void)playbackFinished:(NSNotification *)notify {
    NSLog(@"Play finished");
}

@end

