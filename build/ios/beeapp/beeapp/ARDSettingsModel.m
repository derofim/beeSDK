/*
 *  Copyright 2016 The WebRTC Project Authors. All rights reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#import "ARDSettingsModel+Private.h"
#import "ARDSettingsStore.h"
#import <AVFoundation/AVFoundation.h>

NS_ASSUME_NONNULL_BEGIN

@interface ARDSettingsModel () {
  ARDSettingsStore *_settingsStore;
}
@end

@implementation ARDSettingsModel

- (NSArray<NSString *> *)availableVideoResolutions {
  NSMutableSet<NSArray<NSNumber *> *> *resolutions =
      [[NSMutableSet<NSArray<NSNumber *> *> alloc] init];
  for (AVCaptureDevice *device in [AVCaptureDevice devicesWithMediaType:AVMediaTypeVideo]) {
    for (AVCaptureDeviceFormat *format in device.formats) {
      CMVideoDimensions resolution =
          CMVideoFormatDescriptionGetDimensions(format.formatDescription);
      NSArray<NSNumber *> *resolutionObject = @[ @(resolution.width), @(resolution.height) ];
      [resolutions addObject:resolutionObject];
    }
  }

  NSArray<NSArray<NSNumber *> *> *sortedResolutions =
      [[resolutions allObjects] sortedArrayUsingComparator:^NSComparisonResult(
                                    NSArray<NSNumber *> *obj1, NSArray<NSNumber *> *obj2) {
        return obj1.firstObject > obj2.firstObject;
      }];

  NSMutableArray<NSString *> *resolutionStrings = [[NSMutableArray<NSString *> alloc] init];
  for (NSArray<NSNumber *> *resolution in sortedResolutions) {
    NSString *resolutionString =
        [NSString stringWithFormat:@"%@x%@", resolution.firstObject, resolution.lastObject];
    [resolutionStrings addObject:resolutionString];
  }

  return [resolutionStrings copy];
}

- (NSString *)currentVideoResolutionSettingFromStore {
  [self registerStoreDefaults];
  return [[self settingsStore] videoResolution];
}

- (BOOL)storeVideoResolutionSetting:(NSString *)resolution {
  if (![[self availableVideoResolutions] containsObject:resolution]) {
    return NO;
  }
  [[self settingsStore] setVideoResolution:resolution];
  return YES;
}

- (nullable NSNumber *)currentMaxBitrateSettingFromStore {
  [self registerStoreDefaults];
  return [[self settingsStore] maxBitrate];
}

- (void)storeMaxBitrateSetting:(nullable NSNumber *)bitrate {
  [[self settingsStore] setMaxBitrate:bitrate];
}

- (nullable NSString *)currentJanusSettingFromStore {
  [self registerStoreDefaults];
  return [[self settingsStore] janus];
}

- (void)storeJanusSetting:(nullable NSString *)url {
    [[self settingsStore] setJanus:url];
}

- (nullable NSString *)currentTestStreamNameSettingFromStore {
    [self registerStoreDefaults];
    return [[self settingsStore] testStreamName];
}

- (void)storeTestStreamNameSetting:(nullable NSString *)streamName {
    [self registerStoreDefaults];
    return [[self settingsStore] setTestStreamName:streamName];
}

- (nullable NSString *)currentUidSettingFromStore {
  [self registerStoreDefaults];
  return [[self settingsStore] uid];
}

- (void)storeUidSetting:(nullable NSString *)uid {
    [[self settingsStore] setUid:uid];
}

- (nullable NSNumber *)currentFpsSettingFromStore {
    [self registerStoreDefaults];
    return [[self settingsStore] fps];
}

- (void)storeFpsSetting:(nullable NSNumber *)fps {
    [[self settingsStore] setFps:fps];
}

- (BOOL)currentLoopbackSettingFromStore {
    return [[self settingsStore] loopback];
}

- (void)storeLoopbackSetting:(BOOL)loopback {
    [[self settingsStore] setLoopback:loopback];
}

- (BOOL)currentAudioOnlySettingFromStore {
  return [[self settingsStore] audioOnly];
}

- (void)storeAudioOnlySetting:(BOOL)audioOnly {
  [[self settingsStore] setAudioOnly:audioOnly];
}

- (BOOL)currentCreateAecDumpSettingFromStore {
  return [[self settingsStore] createAecDump];
}

- (void)storeCreateAecDumpSetting:(BOOL)createAecDump {
  [[self settingsStore] setCreateAecDump:createAecDump];
}

- (BOOL)currentUseLevelControllerSettingFromStore {
  return [[self settingsStore] useLevelController];
}

- (void)storeUseLevelControllerSetting:(BOOL)useLevelController {
  [[self settingsStore] setUseLevelController:useLevelController];
}

- (BOOL)currentUseManualAudioConfigSettingFromStore {
  return [[self settingsStore] useManualAudioConfig];
}

- (void)storeUseManualAudioConfigSetting:(BOOL)useManualAudioConfig {
  [[self settingsStore] setUseManualAudioConfig:useManualAudioConfig];
}

#pragma mark - Testable

- (ARDSettingsStore *)settingsStore {
  if (!_settingsStore) {
    _settingsStore = [[ARDSettingsStore alloc] init];
    [self registerStoreDefaults];
  }
  return _settingsStore;
}

- (int)currentVideoResolutionWidthFromStore {
  NSString *resolution = [self currentVideoResolutionSettingFromStore];

  return [self videoResolutionComponentAtIndex:0 inString:resolution];
}

- (int)currentVideoResolutionHeightFromStore {
  NSString *resolution = [self currentVideoResolutionSettingFromStore];
  return [self videoResolutionComponentAtIndex:1 inString:resolution];
}

#pragma mark -

- (NSString *)defaultVideoResolutionSetting {
  NSArray<NSString *> *resolutionStrings = [self availableVideoResolutions];
  for (int i = 0; i < resolutionStrings.count; i++) {
    NSString *str = resolutionStrings[i];
    if ([str isEqualToString:@"640x480"]) {
      return str;
    }
  }
  return resolutionStrings.firstObject;
}

- (int)videoResolutionComponentAtIndex:(int)index inString:(NSString *)resolution {
  if (index != 0 && index != 1) {
    return 0;
  }
  NSArray<NSString *> *components = [resolution componentsSeparatedByString:@"x"];
  if (components.count != 2) {
    return 0;
  }
  return components[index].intValue;
}

- (void)registerStoreDefaults {
  NSString *defaultVideoResolutionSetting = [self defaultVideoResolutionSetting];
  [ARDSettingsStore setDefaultsForVideoResolution:[self defaultVideoResolutionSetting]
                                          bitrate:nil
                                        audioOnly:NO
                                    createAecDump:NO
                               useLevelController:NO
                             useManualAudioConfig:YES];
  NSString *uid = [[self settingsStore] uid];
  if (uid == nil) {
    [[self settingsStore] setUid:[self uniqueAppInstanceIdentifier]];
  }
    
  NSNumber *fps = [[self settingsStore] fps];
  if (fps == nil) {
    [[self settingsStore] setFps:[NSNumber numberWithInt:30]];
  }
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

@end
NS_ASSUME_NONNULL_END
