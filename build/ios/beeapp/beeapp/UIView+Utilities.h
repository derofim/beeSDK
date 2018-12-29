#import <UIKit/UIKit.h>

@interface UIView (Utilities)

//Modify position and size.
@property (nonatomic, assign) CGFloat height;
@property (nonatomic, assign) CGFloat width;
@property (nonatomic, assign) CGFloat x;
@property (nonatomic, assign) CGFloat y;

//For click event.
- (void)addClickedBlock:(void(^)(id obj))tapAction;

//For moving view.
- (void)handlePan:(UIPanGestureRecognizer*)pgr;
@end
