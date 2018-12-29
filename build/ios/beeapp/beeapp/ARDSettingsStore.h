/*
 *  Copyright 2016 The WebRTC Project Authors. All rights reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

/**
 * Light-weight persistent store for user settings.
 *
 * It will persist between application launches and application updates.
 */
@interface ARDSettingsStore : NSObject

/**
 * Set fallback values in case the setting has not been written by the user.
 * @param dictionary of values to store
 */
+ (void)setDefaultsForVideoResolution:(NSString *)videoResolution                           
                              bitrate:(nullable NSNumber *)bitrate
                            audioOnly:(BOOL)audioOnly
                        createAecDump:(BOOL)createAecDump
                   useLevelController:(BOOL)useLevelController
                 useManualAudioConfig:(BOOL)useManualAudioConfig;

@property(nonatomic) NSString *videoResolution;
@property(nonatomic) NSData *videoCodec;

/**
 * Returns current max bitrate number stored in the store.
 */
- (nullable NSNumber *)maxBitrate;

/**
 * Stores the provided value as maximum bitrate setting.
 * @param value the number to be stored
 */
- (void)setMaxBitrate:(nullable NSNumber *)value;

/**
 * Return current test janus url.
 */
- (nullable NSString *)janus;

/**
 * Stores the provided url as janus setting.
 * @param url the test janus url to be stored
 */
- (void)setJanus:(nullable NSString *)url;

/**
 * Return current test stream name.
 */
- (nullable NSString *)testStreamName;

/**
 * Stores the provided stream name as test stream name setting.
 * @param streamName the test stream name to be stored
 */
- (void)setTestStreamName:(nullable NSString *)streamName;

/**
 * Return current uid.
 */
- (nullable NSString *)uid;

/**
 * Stores the provided uid
 * @param uid Unique id of device.
 */
- (void)setUid:(nullable NSString *)uid;

/**
 * Return current fps.
 */
- (nullable NSNumber *)fps;

/**
 * Stores the provided fps
 * @param fps Frames per second.
 */
- (void)setFps:(nullable NSNumber *)fps;

@property(nonatomic) BOOL loopback;
@property(nonatomic) BOOL audioOnly;
@property(nonatomic) BOOL createAecDump;
@property(nonatomic) BOOL useLevelController;
@property(nonatomic) BOOL useManualAudioConfig;

@end
NS_ASSUME_NONNULL_END
