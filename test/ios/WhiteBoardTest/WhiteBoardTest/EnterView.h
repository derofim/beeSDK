#import <UIKit/UIKit.h>

@class EnterView;

@protocol EnterViewDelegate <NSObject>

- (void)enterView:(EnterView *)enterView didInputRoom:(NSString *)room create:(BOOL)create;

@end

@interface EnterView : UIView

@property(nonatomic, weak) id<EnterViewDelegate> delegate;

@end
