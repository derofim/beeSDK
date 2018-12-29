/*
 *  Copyright 2015 The WebRTC Project Authors. All rights reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#import "ARDMainViewController.h"
#import <AVFoundation/AVFoundation.h>

#import "ARDSettingsModel.h"
#import "ARDMainView.h"
#import "ARDSettingsViewController.h"
#import "ARDVideoCallViewController.h"
#import "AsiHttpRequest/ASIFormDataRequest.h"
#import "SSZipArchive/ZipArchive.h"
#import <sys/utsname.h>

static NSString *const barSettingButtonImageString = @"ic_settings_black_24dp.png";
static NSString *const barUploadButtonImageString = @"ic_upload_blue_24dp.png";

// Launch argument to be passed to indicate that the app should start loopback immediatly
static NSString *const loopbackLaunchProcessArgument = @"loopback";

@interface ARDMainViewController () <ARDMainViewDelegate, ARDVideoCallViewControllerDelegate>
@end

@implementation ARDMainViewController {
  ARDMainView *_mainView;
  AVAudioPlayer *_audioPlayer;
  BOOL _useManualAudio;
    
}

- (void)viewDidLoad {
  [super viewDidLoad];
  if ([[[NSProcessInfo processInfo] arguments] containsObject:loopbackLaunchProcessArgument]) {
    [self mainView:nil didInputRoom:@"" name:@"" create:YES];
  }
}

- (void)loadView {
  self.title = @"Bee VideoRoom";
  _mainView = [[ARDMainView alloc] initWithFrame:CGRectZero];
  _mainView.delegate = self;
  self.view = _mainView;
  [self addSettingsBarButton];
  [self addUploadBarButton];
}

- (void)addSettingsBarButton {
  UIBarButtonItem *settingsButton =
      [[UIBarButtonItem alloc] initWithImage:[UIImage imageNamed:barSettingButtonImageString]
                                       style:UIBarButtonItemStylePlain
                                      target:self
                                      action:@selector(showSettings:)];
  self.navigationItem.rightBarButtonItem = settingsButton;
}

- (void)addUploadBarButton {
  UIBarButtonItem *uploadButton =
      [[UIBarButtonItem alloc] initWithImage:[UIImage imageNamed:barUploadButtonImageString]
                                       style:UIBarButtonItemStylePlain
                                      target:self
                                      action:@selector(uploadLog:)];
    self.navigationItem.leftBarButtonItem = uploadButton;
}

+ (NSString *)loopbackRoomString {
  NSString *loopbackRoomString =
      [[NSUUID UUID].UUIDString stringByReplacingOccurrencesOfString:@"-" withString:@""];
  return loopbackRoomString;
}

#pragma mark - ARDMainViewDelegate

- (void)mainView:(ARDMainView *)mainView didInputRoom:(NSString *)room name:(NSString*)name create:(BOOL)create{
  if (!room.length) {
    [self showAlertWithMessage:@"Missing room name."];
    return;
  }
  // Trim whitespaces.
  NSCharacterSet *whitespaceSet = [NSCharacterSet whitespaceCharacterSet];
  NSString *trimmedRoom = [room stringByTrimmingCharactersInSet:whitespaceSet];

  // Check that room name is valid.
  NSError *error = nil;
  NSRegularExpressionOptions options = NSRegularExpressionCaseInsensitive;
  NSRegularExpression *regex =
      [NSRegularExpression regularExpressionWithPattern:@"\\w+"
                                                options:options
                                                  error:&error];
  if (error) {
    [self showAlertWithMessage:error.localizedDescription];
    return;
  }
  NSRange matchRange =
      [regex rangeOfFirstMatchInString:trimmedRoom
                               options:0
                                 range:NSMakeRange(0, trimmedRoom.length)];
  if (matchRange.location == NSNotFound ||
      matchRange.length != trimmedRoom.length) {
    [self showAlertWithMessage:@"Invalid room name."];
    return;
  }
    
  // Kick off the video call.
  ARDVideoCallViewController *videoCallViewController =
      [[ARDVideoCallViewController alloc] initForRoom:trimmedRoom
                                                 name:name
                                               create:create
                                             delegate:self];
  videoCallViewController.modalTransitionStyle =
      UIModalTransitionStyleCrossDissolve;
  [self presentViewController:videoCallViewController
                     animated:YES
                   completion:nil];
}

- (void)mainViewDidToggleAudioLoop:(ARDMainView *)mainView {
  if (mainView.isAudioLoopPlaying) {
    [_audioPlayer stop];
  } else {
    [_audioPlayer play];
  }
  mainView.isAudioLoopPlaying = _audioPlayer.playing;
}

#pragma mark - ARDVideoCallViewControllerDelegate

- (void)viewControllerDidFinish:(ARDVideoCallViewController *)viewController {
    if (![viewController isBeingDismissed]) {
        [self dismissViewControllerAnimated:YES completion:^{
            //Nothing to do.
        }];
    }
}

#pragma mark - Private
- (void)showSettings:(id)sender {
  ARDSettingsViewController *settingsController =
      [[ARDSettingsViewController alloc] initWithStyle:UITableViewStyleGrouped
                                         settingsModel:[[ARDSettingsModel alloc] init]];

  UINavigationController *navigationController =
      [[UINavigationController alloc] initWithRootViewController:settingsController];
  [self presentViewControllerAsModal:navigationController];
}

- (NSString*)getLogPath {
    NSArray *paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
    NSString *documentDirectory = [paths objectAtIndex:0];
    return documentDirectory;
}

- (NSString*)httpPost:(NSString *)url
          params:(NSMutableDictionary *)params
           files:(NSMutableDictionary *)files {
  if (url == nil) {
    return @"Invalid URL";
  }

  NSURL *nsUrl = [NSURL URLWithString:url];
  ASIFormDataRequest *request = [ASIFormDataRequest requestWithURL:nsUrl];
  request.timeOutSeconds = 5;
  [request setRequestMethod:@"POST"];
  [request addRequestHeader:@"Charset" value:@"UTF-8"];
    
  if (params != nil) {
    for (NSString *key in params) {
        NSString *value = [params valueForKey:key];
        [request addPostValue:value
                       forKey:key];
    }
  }

  if (files != nil) {
    for (NSString *key in files) {
      [request addFile:[files valueForKey:key]
          withFileName:key
        andContentType:@"application/octet-stream; charset=UTF-8"
                forKey:@"upload_file_minidump"];
    }
  }

  [request startSynchronous];
  NSError *error = [request error];
  NSString *rsp = nil;
  if (!error) {
    rsp = [NSString stringWithFormat:@"%d",[request responseStatusCode]];
    NSString *rr = [request responseString];
    NSLog(@"post rsp %@", rr);
    NSLog(@"Http post rsp %@", rsp);
  } else {
    rsp = [error localizedDescription];
    NSLog(@"Http post error %@", rsp);
  }
  return rsp;
}

- (NSString*)uniqueAppInstanceIdentifier {
  NSUserDefaults* userDefaults = [NSUserDefaults standardUserDefaults];
  static NSString* UUID_KEY = @"MPUUID";
  NSString* app_uuid = [userDefaults stringForKey:UUID_KEY];
  if (app_uuid == nil) {
    CFUUIDRef uuidRef = CFUUIDCreate(kCFAllocatorDefault);
    CFStringRef uuidString = CFUUIDCreateString(kCFAllocatorDefault, uuidRef);
    app_uuid = [NSString stringWithString:(__bridge NSString*)uuidString];
    [userDefaults setObject:app_uuid forKey:UUID_KEY];
    [userDefaults synchronize];
    CFRelease(uuidString);
    CFRelease(uuidRef);
  }
  return app_uuid;
}

- (void)uploadLog:(id)sender {
    //Log path.
    NSString *path = [self getLogPath];
    
    //Zip tmp path.
    NSString *tmpDir = NSTemporaryDirectory();
    
    if (path == nil || tmpDir == nil) {
        return;
    }
    
    //Get date string.
    NSDate *date = [NSDate date];
    NSDateFormatter * dateFormatter = [[NSDateFormatter alloc] init ];
    [dateFormatter setDateFormat:@"yyyyMMdd_HHmmss"];
    NSString *dateStr = [dateFormatter stringFromDate:date];
    
    //Get platform string.
    struct utsname systemInfo;
    uname(&systemInfo);
    NSString *platform = [NSString stringWithCString:systemInfo.machine encoding:NSASCIIStringEncoding];
    platform = [platform stringByReplacingOccurrencesOfString:@","
                                                   withString:@"."];
    
    //Get device name string.
    NSString *userPhoneName = [[UIDevice currentDevice] name];
    userPhoneName = [userPhoneName stringByReplacingOccurrencesOfString:@" "
                                                             withString:@""];
    
    //Get ios version.
    NSString *phoneVersion = [[UIDevice currentDevice] systemVersion];
    
    //Get machine code.
    NSString *id = [self uniqueAppInstanceIdentifier];
    
    //Format zip path.
    NSString *baseName = [NSString stringWithFormat:@"%@-%@-%@-%@-%@.zip", dateStr, userPhoneName, platform, phoneVersion, id];
    NSString *zipPath = [NSString stringWithFormat:@"%@%@", tmpDir, baseName];
    
    //Create zip from path to zipPath.
    BOOL ret = [SSZipArchive createZipFileAtPath:zipPath
                         withContentsOfDirectory:path];
    if (!ret) {
        return;
    }
    
    //Set version info.
    NSMutableDictionary *params = [NSMutableDictionary dictionaryWithCapacity:10];
    [params setObject:@"1.0"
               forKey:@"version"];
    [params setObject:@"557b8a4f"
               forKey:@"git"];
    
    //Set file info.
    NSMutableDictionary *files = [NSMutableDictionary dictionaryWithCapacity:10];
    [files setObject:zipPath
              forKey:baseName];
    
    //Post file.
    NSString *rsp = [self httpPost:@"http://106.120.154.76/android.php"
                            params:params
                             files:files];
    
    NSString *msg = [NSString stringWithFormat:@"Uploading return %@",rsp];
    [self showAlertWithMessage:msg];
    [[NSFileManager defaultManager] removeItemAtPath:zipPath error:nil];
}

- (void)presentViewControllerAsModal:(UIViewController *)viewController {
  [self presentViewController:viewController animated:YES completion:nil];
}

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
