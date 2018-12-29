#import "EnterViewController.h"
#import "WhiteBoardViewController.h"
#import "EnterView.h"
#import <AVFoundation/AVFoundation.h>


@interface EnterViewController () <EnterViewDelegate, WhiteBoardViewControllerDelegate>
@end

@implementation EnterViewController {
  EnterView *_enterView;
}

- (void)viewDidLoad {
  [super viewDidLoad];
}

- (void)loadView {
    self.title = @"Bee VideoRoom";
    _enterView = [[EnterView alloc] initWithFrame:CGRectZero];
    _enterView.delegate = self;
    self.view = _enterView;
}

#pragma mark - ARDMainViewDelegate

- (void)enterView:(EnterView *)enterView didInputRoom:(NSString *)room create:(BOOL)create{
    if (!room.length) {
        [self showAlertWithMessage:@"Missing room name."];
        return;
    }

    WhiteBoardViewController *whiteBoardViewController =
      [[WhiteBoardViewController alloc] initForRoom:room
                                             create:create
                                           delegate:self];
    whiteBoardViewController.modalTransitionStyle = UIModalTransitionStyleCrossDissolve;
    [self presentViewController:whiteBoardViewController
                     animated:YES
                   completion:nil];
}

#pragma mark - WhiteBoardViewControllerDelegate

- (void)whiteBoardViewControllerDidFinish:(WhiteBoardViewController *)viewController {
    if (![viewController isBeingDismissed]) {
        [self dismissViewControllerAnimated:YES completion:^{
            //Nothing to do.
        }];
    }
}

#pragma mark - Private

- (void)showAlertWithMessage:(NSString*)message {
  UIAlertController *alert =
      [UIAlertController alertControllerWithTitle:nil
                                          message:message
                                   preferredStyle:UIAlertControllerStyleAlert];

  UIAlertAction *defaultAction = [UIAlertAction actionWithTitle:@"OK"
                                                          style:UIAlertActionStyleDefault
                                                        handler:^(UIAlertAction *action){
                                                        }];

  [alert addAction:defaultAction];
  [self presentViewController:alert animated:YES completion:nil];
}

@end
