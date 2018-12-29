/*
 *  Copyright 2017 The WebRTC Project Authors. All rights reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#import "ARDCaptureController.h"

#import "WebRTC/RTCLogging.h"

@implementation ARDCaptureController {
  RTCCameraVideoCapturer *_capturer;
  NSInteger _fps;
  int _width;
  int _height;
  BOOL _usingFrontCamera;
}

- (instancetype)initWithCapturer:(RTCCameraVideoCapturer *)capturer
                             fps:(NSInteger)fps
                           width:(int)width
                          height:(int)height {
  if ([super init]) {
    _capturer = capturer;
    _fps = fps;
    _width = width;
    _height = height;
    _usingFrontCamera = YES;
  }

  return self;
}

- (void)startCapture {
  AVCaptureDevicePosition position =
      _usingFrontCamera ? AVCaptureDevicePositionFront : AVCaptureDevicePositionBack;
  AVCaptureDevice *device = [self findDeviceForPosition:position];
  AVCaptureDeviceFormat *format = [self selectFormatForDevice:device];

  if (format == nil) {
    RTCLogError(@"No valid formats for device %@", device);
    NSAssert(NO, @"");
    return;
  }

  [_capturer startCaptureWithDevice:device format:format fps:_fps];
}

- (void)stopCapture {
  [_capturer stopCapture];
}

- (void)switchCamera {
  _usingFrontCamera = !_usingFrontCamera;
  [self startCapture];
}

- (BOOL)isFrontCamera {
  return _usingFrontCamera;
}

#pragma mark - Private

- (AVCaptureDevice *)findDeviceForPosition:(AVCaptureDevicePosition)position {
  NSArray<AVCaptureDevice *> *captureDevices = [RTCCameraVideoCapturer captureDevices];
  for (AVCaptureDevice *device in captureDevices) {
    if (device.position == position) {
      return device;
    }
  }
  return captureDevices[0];
}

- (AVCaptureDeviceFormat *)selectFormatForDevice:(AVCaptureDevice *)device {
  NSArray<AVCaptureDeviceFormat *> *formats =
      [RTCCameraVideoCapturer supportedFormatsForDevice:device];
  AVCaptureDeviceFormat *selectedFormat = nil;
  int currentDiff = INT_MAX;

  for (AVCaptureDeviceFormat *format in formats) {
    CMVideoDimensions dimension = CMVideoFormatDescriptionGetDimensions(format.formatDescription);
    FourCharCode pixelFormat = CMFormatDescriptionGetMediaSubType(format.formatDescription);
    int diff = abs(_width - dimension.width) + abs(_height - dimension.height);
      NSLog(@"Checking %d*%d, target %d*%d, diff %d", dimension.width, dimension.height, _width, _height, diff);
    if (diff < currentDiff) {
      selectedFormat = format;
      currentDiff = diff;
    } else if (diff == currentDiff && pixelFormat == [_capturer preferredOutputPixelFormat]) {
      selectedFormat = format;
    }
  }

  CMVideoDimensions dimension = CMVideoFormatDescriptionGetDimensions(selectedFormat.formatDescription);
  FourCharCode pixelFormat = CMFormatDescriptionGetMediaSubType(selectedFormat.formatDescription);
  NSLog(@"Pick %d*%d", dimension.width, dimension.height);
  return selectedFormat;
}

- (NSInteger)selectFpsForFormat:(AVCaptureDeviceFormat *)format {
  Float64 maxFramerate = 0;
  for (AVFrameRateRange *fpsRange in format.videoSupportedFrameRateRanges) {
    maxFramerate = fmax(maxFramerate, fpsRange.maxFrameRate);
  }
  return maxFramerate;
}

@end
