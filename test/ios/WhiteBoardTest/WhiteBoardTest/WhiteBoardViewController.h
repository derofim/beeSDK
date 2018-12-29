#import <UIKit/UIKit.h>

@class WhiteBoardViewController;

@protocol WhiteBoardViewControllerDelegate <NSObject>

- (void)whiteBoardViewControllerDidFinish:(WhiteBoardViewController *)viewController;

@end

@interface WhiteBoardViewController : UIViewController

@property(nonatomic, weak) id<WhiteBoardViewControllerDelegate> delegate;

- (instancetype)initForRoom:(NSString *)room
                     create:(BOOL)create
                   delegate:(id<WhiteBoardViewControllerDelegate>)delegate;

@end
