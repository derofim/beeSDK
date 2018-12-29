/*
 *  Copyright 2015 The WebRTC Project Authors. All rights reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#import "ARDVideoCallView.h"
#import <AVFoundation/AVFoundation.h>
#import "UIImage+ARDUtilities.h"
#import "UIView+Utilities.h"
#include <vector>

static CGFloat const kButtonPadding = 16;
static CGFloat const kButtonSize = 48;
static CGFloat const kLocalVideoViewPadding = 8;
static CGFloat const kStatusBarHeight = 20;

#pragma mark - MyVideoRenderer

@implementation MyVideoRenderer
@synthesize rendering = _rendering;

- (instancetype)init {
    if (self = [super init]) {
        _rendering = NO;
    }
    return self;
}
@end


//////////////////////////////////ARDVideoCallView////////////////////////////////////
#pragma mark - ARDVideoCallView

@interface ARDVideoCallView() <BeeVideoRendererDelegate>

@end

@implementation ARDVideoCallView {
  UIButton *_routeChangeButton;
  UIButton *_cameraSwitchButton;
  UIButton *_hangupButton;
  CGSize _mainVideoSize;
  int _maxPartyCount;
  BeeVideoRenderer *_mainView;
  CGFloat _subVideoViewSize;
  NSMutableArray<BeeVideoRenderer*> *_subViews;
  NSMutableArray<BeeVideoRenderer*> *_idleRendererQueue;
}

@synthesize statusLabel = _statusLabel;
@synthesize localVideoView = _localVideoView;
@synthesize remoteVideoViews = _remoteVideoViews;
@synthesize statsView = _statsView;
@synthesize delegate = _delegate;

- (instancetype)initWithFrame:(CGRect)frame {
  if (self = [super initWithFrame:frame]) {
    _maxPartyCount = 4;
      
    _remoteVideoViews = [[NSMutableArray alloc] initWithCapacity:_maxPartyCount];
    _subViews = [[NSMutableArray alloc] initWithCapacity:_maxPartyCount];
    _idleRendererQueue = [[NSMutableArray alloc] initWithCapacity:_maxPartyCount];
    for (int i = 0; i < _maxPartyCount; ++i) {
      BeeVideoRenderer *remoteVideoView = [[MyVideoRenderer alloc] init];
      remoteVideoView.delegate = self;
    
      [_remoteVideoViews addObject:remoteVideoView];
      [self addSubview:remoteVideoView.view];
      
      [_idleRendererQueue addObject:remoteVideoView];
    }
    
    //Set initial main and sub views.
    for (int i = 0; i < _maxPartyCount; ++i) {
      [_subViews setObject:_remoteVideoViews[i] atIndexedSubscript:i];
    }
      
    //Set click listeners.
    __weak __typeof(ARDVideoCallView*) thisWeakView = self;
    
    for (BeeVideoRenderer *renderer in _remoteVideoViews) {
      __weak __typeof(UIView*) weakRemoteVideoView = renderer.view;
      [renderer.view addClickedBlock:^(id obj) {
        __strong __typeof(ARDVideoCallView*) thisView = thisWeakView;
        __strong __typeof(UIView*) videoView = weakRemoteVideoView;
        if (thisView && videoView) {
          [thisView onVideoViewClicked:videoView];
        }
      }];
    }

    _localVideoView = [[MyVideoRenderer alloc] init];
    _localVideoView.delegate = self;
    _mainView = _localVideoView;

    __weak __typeof(UIView*) weakLocalVideoView = _localVideoView.view;
    [_localVideoView.view addClickedBlock:^(id obj) {
      __strong __typeof(ARDVideoCallView*) thisView = thisWeakView;
      __strong __typeof(UIView*) videoView = weakLocalVideoView;
      if (thisView && videoView) {
        [thisView onVideoViewClicked:videoView];
      }
    }];
    
    [self addSubview:_localVideoView.view];

    //Other helper views.
    _statsView = [[ARDStatsView alloc] initWithFrame:CGRectZero];
    _statsView.hidden = YES;
    [self addSubview:_statsView];

    _routeChangeButton = [UIButton buttonWithType:UIButtonTypeCustom];
    _routeChangeButton.backgroundColor = [UIColor whiteColor];
    _routeChangeButton.layer.cornerRadius = kButtonSize / 2;
    _routeChangeButton.layer.masksToBounds = YES;
    UIImage *image = [UIImage imageNamed:@"ic_surround_sound_black_24dp.png"];
    [_routeChangeButton setImage:image forState:UIControlStateNormal];
    [_routeChangeButton addTarget:self
                           action:@selector(onRouteChange:)
                 forControlEvents:UIControlEventTouchUpInside];
    [self addSubview:_routeChangeButton];

    // TODO(tkchin): don't display this if we can't actually do camera switch.
    _cameraSwitchButton = [UIButton buttonWithType:UIButtonTypeCustom];
    _cameraSwitchButton.backgroundColor = [UIColor whiteColor];
    _cameraSwitchButton.layer.cornerRadius = kButtonSize / 2;
    _cameraSwitchButton.layer.masksToBounds = YES;
    image = [UIImage imageNamed:@"ic_switch_video_black_24dp.png"];
    [_cameraSwitchButton setImage:image forState:UIControlStateNormal];
    [_cameraSwitchButton addTarget:self
                      action:@selector(onCameraSwitch:)
            forControlEvents:UIControlEventTouchUpInside];
    [self addSubview:_cameraSwitchButton];

    _hangupButton = [UIButton buttonWithType:UIButtonTypeCustom];
    _hangupButton.backgroundColor = [UIColor redColor];
    _hangupButton.layer.cornerRadius = kButtonSize / 2;
    _hangupButton.layer.masksToBounds = YES;
    image = [UIImage imageForName:@"ic_call_end_black_24dp.png"
                            color:[UIColor whiteColor]];
    [_hangupButton setImage:image forState:UIControlStateNormal];
    [_hangupButton addTarget:self
                      action:@selector(onHangup:)
            forControlEvents:UIControlEventTouchUpInside];
    [self addSubview:_hangupButton];

    _statusLabel = [[UILabel alloc] initWithFrame:CGRectZero];
    _statusLabel.font = [UIFont fontWithName:@"Roboto" size:16];
    _statusLabel.textColor = [UIColor whiteColor];
    [self addSubview:_statusLabel];

    UITapGestureRecognizer *tapRecognizer =
        [[UITapGestureRecognizer alloc]
            initWithTarget:self
                    action:@selector(didTripleTap:)];
    tapRecognizer.numberOfTapsRequired = 3;
    [self addGestureRecognizer:tapRecognizer];
  }
  return self;
}

- (void) onVideoViewClicked:(UIView*)selectedView {
    if (![self isVideoViewRenderering:selectedView]) {
        return;
    }
    
    for (int i = 0; i < _maxPartyCount; ++i) {
        if (selectedView != _subViews[i].view) {
            continue;
        }
        
        BeeVideoRenderer *tmp = _subViews[i];
        _subViews[i] = _mainView;
        _mainView = tmp;
        [self setNeedsLayout];
        break;
    }
}

- (void) onRendererStarted:(BeeVideoRenderer*)renderer {
    MyVideoRenderer *myRenderer = (MyVideoRenderer*)renderer;
    myRenderer.rendering = YES;
    
    MyVideoRenderer *mainRenderer = (MyVideoRenderer*)_mainView;
    if (_mainView == _localVideoView || !mainRenderer.rendering) {
        [self swapToMainView:renderer];
    }
}

- (void) onRendererStopped:(BeeVideoRenderer*)renderer {
    MyVideoRenderer *myRenderer = (MyVideoRenderer*)renderer;
    myRenderer.rendering = NO;
    
    BOOL remoteToMain = NO;
    if (myRenderer.view == _mainView.view) {
        for (int i = 0; i < _maxPartyCount; ++i) {
            MyVideoRenderer *subView = (MyVideoRenderer*)_subViews[i];
            if (subView.rendering) {
                [self swapToMainView:_subViews[i]];
                remoteToMain = YES;
                break;
            }
        }
    }
    
    if (!remoteToMain) {
        [self swapToMainView:_localVideoView];
    }
}

- (void) swapToMainView:(BeeVideoRenderer*)renderer {
    if (renderer.view == _mainView.view) {
        return;
    }
    
    for (int i = 0; i < _maxPartyCount; ++i) {
        if (renderer.view != _subViews[i].view) {
            continue;
        }
        
        BeeVideoRenderer *tmp = _subViews[i];
        _subViews[i] = _mainView;
        _mainView = tmp;
        [self setNeedsLayout];
        break;
    }
}

- (BOOL) isVideoViewRenderering:(UIView *)videoView {
    if (videoView == _localVideoView.view) {
        return YES;
    }
    
    for (int i = 0; i < _maxPartyCount; ++i) {
        if (videoView == _remoteVideoViews[i].view) {
            MyVideoRenderer *renderer = (MyVideoRenderer*)_remoteVideoViews[i];
            return renderer.rendering;
        }
    }
    
    return NO;
}

- (void) onStats:(int)index
           stats:(NSArray *)stats {
    if (index == -1 || (index > -1 && _remoteVideoViews[index] == _mainView)) {
        self.statsView.stats = stats;
        [self setNeedsLayout];
    }
}

- (BeeVideoRenderer*)applyVideoRenderer {
    if (_idleRendererQueue != nil && [_idleRendererQueue count] > 0) {
        BeeVideoRenderer *videoRenderer = [_idleRendererQueue objectAtIndex:0];
        [_idleRendererQueue removeObjectAtIndex:0];
        videoRenderer.view.hidden = NO;
        return videoRenderer;
    } else {
        return nil;
    }
}

- (void)recycleVideoRenderer:(BeeVideoRenderer*)videoRenderer {
    if (_idleRendererQueue != nil) {
        videoRenderer.view.hidden = YES;
        [_idleRendererQueue insertObject:videoRenderer atIndex:0];
    }
}

- (void)layoutSubviews {
  CGRect bounds = self.bounds;
  if (_mainVideoSize.width > 0 && _mainVideoSize.height > 0) {
    // Aspect fill remote video into bounds.
    CGRect mainVideoFrame =
        AVMakeRectWithAspectRatioInsideRect(_mainVideoSize, bounds);
    CGFloat scale = 1;
    if (mainVideoFrame.size.width > mainVideoFrame.size.height) {
      // Scale by height.
      scale = bounds.size.height / mainVideoFrame.size.height;
    } else {
      // Scale by width.
      scale = bounds.size.width / mainVideoFrame.size.width;
    }
    mainVideoFrame.size.height *= scale;
    mainVideoFrame.size.width *= scale;
    _mainView.view.frame = mainVideoFrame;
    _mainView.view.center =
        CGPointMake(CGRectGetMidX(bounds), CGRectGetMidY(bounds));
  } else {
    _mainView.view.frame = bounds;
  }
    
  _subVideoViewSize = CGRectGetMaxY(bounds) / 4;

  // Aspect fit sub video views into a square box.
  CGRect subVideoFrame0 = CGRectMake(0, 0, _subVideoViewSize, _subVideoViewSize);
  subVideoFrame0.origin.x = CGRectGetMaxX(bounds) - subVideoFrame0.size.width - kLocalVideoViewPadding;
  subVideoFrame0.origin.y = CGRectGetMaxY(bounds) - subVideoFrame0.size.height;
  _subViews[0].view.frame = subVideoFrame0;
    
  CGRect subVideoFrame1 = CGRectMake(0, 0, _subVideoViewSize, _subVideoViewSize);
  subVideoFrame1.origin.x = CGRectGetMaxX(bounds) - subVideoFrame1.size.width - kLocalVideoViewPadding;
  subVideoFrame1.origin.y = CGRectGetMaxY(bounds) - 2 * subVideoFrame1.size.height;
  _subViews[1].view.frame = subVideoFrame1;
    
  CGRect subVideoFrame2 = CGRectMake(0, 0, _subVideoViewSize, _subVideoViewSize);
  subVideoFrame2.origin.x = CGRectGetMaxX(bounds) - subVideoFrame2.size.width - kLocalVideoViewPadding;
  subVideoFrame2.origin.y = CGRectGetMaxY(bounds) - 3 * subVideoFrame2.size.height;
  _subViews[2].view.frame = subVideoFrame2;
    
  CGRect subVideoFrame3 = CGRectMake(0, 0, _subVideoViewSize, _subVideoViewSize);
  subVideoFrame3.origin.x = CGRectGetMaxX(bounds) - subVideoFrame3.size.width - kLocalVideoViewPadding;
  subVideoFrame3.origin.y = 0;
  _subViews[3].view.frame = subVideoFrame3;
    
  [self sendSubviewToBack:_mainView.view];
  for (int i = 0; i < _maxPartyCount; ++i) {
    [self bringSubviewToFront:_subViews[i].view];
  }

  // Place stats at the top.
  CGSize statsSize = [_statsView sizeThatFits:bounds.size];
  _statsView.frame = CGRectMake(CGRectGetMinX(bounds),
                                CGRectGetMinY(bounds) + kStatusBarHeight,
                                statsSize.width, statsSize.height);

  // Place hangup button in the bottom left.
  _hangupButton.frame =
      CGRectMake(CGRectGetMinX(bounds) + kButtonPadding,
                 CGRectGetMaxY(bounds) - kButtonPadding -
                     kButtonSize,
                 kButtonSize,
                 kButtonSize);

  // Place button to the right of hangup button.
  CGRect cameraSwitchFrame = _hangupButton.frame;
  cameraSwitchFrame.origin.x =
      CGRectGetMaxX(cameraSwitchFrame) + kButtonPadding;
  _cameraSwitchButton.frame = cameraSwitchFrame;

  // Place route button to the right of camera button.
  CGRect routeChangeFrame = _cameraSwitchButton.frame;
  routeChangeFrame.origin.x =
      CGRectGetMaxX(routeChangeFrame) + kButtonPadding;
  _routeChangeButton.frame = routeChangeFrame;

  [_statusLabel sizeToFit];
  _statusLabel.center =
      CGPointMake(CGRectGetMidX(bounds), CGRectGetMidY(bounds));
}

#pragma mark - BeeVideoRendererDelegate

- (void)didChangeVideoSize:(BeeVideoRenderer*)renderer size:(CGSize)size {
 if (renderer.view == _mainView.view) {
    _mainVideoSize = size;
  }
  [self setNeedsLayout];
}

#pragma mark - Private

- (void)onCameraSwitch:(id)sender {
  [_delegate videoCallViewDidSwitchCamera:self];
}

- (void)onRouteChange:(id)sender {
  [_delegate videoCallViewDidChangeRoute:self];
}

- (void)onHangup:(id)sender {
  [_delegate videoCallViewDidHangup:self];
}

- (void)didTripleTap:(UITapGestureRecognizer *)recognizer {
  [_delegate videoCallViewDidEnableStats:self];
}

@end
