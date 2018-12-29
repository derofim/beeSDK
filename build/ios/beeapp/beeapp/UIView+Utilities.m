#import "UIView+Utilities.h"
#import <objc/message.h>

@interface UIView ()

@property void(^clickedAction)(id);

@end


@implementation UIView (Utilities)

- (CGFloat) height {
    return self.frame.size.height;
}

- (CGFloat) width {
    return self.frame.size.width;
}

- (CGFloat) x {
    return self.frame.origin.x;
}

- (CGFloat) y {
    return self.frame.origin.y;
}

- (CGFloat) centerY {
    return self.center.y;
}

- (CGFloat) centerX {
    return self.center.x;
}

- (void) setHeight:(CGFloat) newHeight {
    CGRect frame = self.frame;
    frame.size.height = newHeight;
    self.frame = frame;
}

- (void) setWidth:(CGFloat) newWidth {
    CGRect frame = self.frame;
    frame.size.width = newWidth;
    self.frame = frame;
}

- (void) setX:(CGFloat) newX {
    CGRect frame = self.frame;
    frame.origin.x = newX;
    self.frame = frame;
}

- (void) setY:(CGFloat) newY {
    CGRect frame = self.frame;
    frame.origin.y = newY;
    self.frame = frame;
}

- (void)setClickedAction:(void (^)(id))clickedAction{
    objc_setAssociatedObject(self, @"AddClickedEvent", clickedAction, OBJC_ASSOCIATION_COPY_NONATOMIC);
}

- (void (^)(id))clickedAction{
    return objc_getAssociatedObject(self, @"AddClickedEvent");
}

- (void)addClickedBlock:(void(^)(id obj))clickedAction{
    self.clickedAction = clickedAction;
    if (![self gestureRecognizers]) {
        self.userInteractionEnabled = YES;
        UITapGestureRecognizer *tap = [[UITapGestureRecognizer alloc] initWithTarget:self action:@selector(tap)];
        UIPanGestureRecognizer* pgr = [[UIPanGestureRecognizer alloc] initWithTarget:self action:@selector(handlePan:)];
        [self addGestureRecognizer:tap];
        [self addGestureRecognizer:pgr];
    }
}

- (void)tap{
    if (self.clickedAction) {
        self.clickedAction(self);
    }
}

- (void)handlePan:(UIPanGestureRecognizer*)pgr {
    if (pgr.state == UIGestureRecognizerStateChanged) {
        CGPoint center = pgr.view.center;
        CGPoint translation = [pgr translationInView:pgr.view];
        center = CGPointMake(center.x + translation.x,
                             center.y + translation.y);
        pgr.view.center = center;
        [pgr setTranslation:CGPointZero inView:pgr.view];
    }
}

@end
