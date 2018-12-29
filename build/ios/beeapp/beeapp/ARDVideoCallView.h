/*
 *  Copyright 2015 The WebRTC Project Authors. All rights reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#import <UIKit/UIKit.h>
#import "ARDStatsView.h"
#import "bee/ios/bee_video_renderer.h"

#pragma mark - MyVideoRenderer

@interface MyVideoRenderer : BeeVideoRenderer
@property (nonatomic, assign) BOOL rendering;
@end

#pragma mark - ARDVideoCallViewDelegate

@class ARDVideoCallView;
@protocol ARDVideoCallViewDelegate <NSObject>

// Called when the camera switch button is pressed.
- (void)videoCallViewDidSwitchCamera:(ARDVideoCallView *)view;

// Called when the route change button is pressed.
- (void)videoCallViewDidChangeRoute:(ARDVideoCallView *)view;

// Called when the hangup button is pressed.
- (void)videoCallViewDidHangup:(ARDVideoCallView *)view;

// Called when stats are enabled by triple tapping.
- (void)videoCallViewDidEnableStats:(ARDVideoCallView *)view;

@end

#pragma mark - ARDVideoCallView

// Video call view that shows local and remote video, provides a label to
// display status, and also a hangup button.
@interface ARDVideoCallView : UIView

@property(nonatomic, readonly) UILabel *statusLabel;
@property(nonatomic, readonly) BeeVideoRenderer *localVideoView;
@property(nonatomic, readonly) NSMutableArray<BeeVideoRenderer*> *remoteVideoViews;
@property(nonatomic, readonly) ARDStatsView *statsView;
@property(nonatomic, weak) id<ARDVideoCallViewDelegate> delegate;

- (void) onVideoViewClicked:(UIView*)selectedView;
- (void) onRendererStarted:(BeeVideoRenderer*)renderer;
- (void) onRendererStopped:(BeeVideoRenderer*)renderer;
- (BOOL) isVideoViewRenderering : (UIView *)videoView;
- (void) onStats:(int)index stats:(NSArray *)stats;
- (BeeVideoRenderer*) applyVideoRenderer;
- (void) recycleVideoRenderer:(BeeVideoRenderer*)videoRenderer;

@end
