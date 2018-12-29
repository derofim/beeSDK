#import "CAShapeLayer+BezierPath.h"
#import <objc/runtime.h>

static const char *kBezierPathKey = "kBezierPathKey";

@implementation CAShapeLayer (BezierPath)

- (void)setBezierPath:(UIBezierPath *)path {
    objc_setAssociatedObject(self,
                             kBezierPathKey,
                             path,
                             OBJC_ASSOCIATION_RETAIN_NONATOMIC);
}

- (UIBezierPath *)bezierPath{
    return objc_getAssociatedObject(self, kBezierPathKey);
}

@end
