/*
 *  Copyright 2016 The WebRTC Project Authors. All rights reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#import "ARDSettingsViewController.h"
#import "ARDSettingsModel.h"

NS_ASSUME_NONNULL_BEGIN

typedef NS_ENUM(int, ARDSettingsSections) {
  ARDSettingsSectionAudioSettings = 0,
  ARDSettingsSectionVideoResolution,
  ARDSettingsSectionBitRate,
  ARDSettingsSectionJanus,
  ARDSettingsSectionTestStreamName,
  ARDSettingsSectionUid,
  ARDSettingsSectionFps,
  ARDSettingsSectionLoopback
};

typedef NS_ENUM(int, ARDAudioSettingsOptions) {
  ARDAudioSettingsAudioOnly = 0,
  ARDAudioSettingsCreateAecDump,
  ARDAudioSettingsUseLevelController,
  ARDAudioSettingsUseManualAudioConfig,
};

typedef NS_ENUM(int, ARDTextFiledTag) {
  ARDTextFiledTagMaxBitrate = 0,
  ARDTextFiledTagJanus,
  ARDTextFiledTagTestStreamName,
  ARDTextFiledTagUid,
  ARDTextFiledTagFps
};

@interface ARDSettingsViewController () <UITextFieldDelegate> {
  ARDSettingsModel *_settingsModel;
}

@end

@implementation ARDSettingsViewController

- (instancetype)initWithStyle:(UITableViewStyle)style
                settingsModel:(ARDSettingsModel *)settingsModel {
  self = [super initWithStyle:style];
  if (self) {
    _settingsModel = settingsModel;
  }
  return self;
}

#pragma mark - View lifecycle

- (void)viewDidLoad {
  [super viewDidLoad];
  self.title = @"Settings";
  [self addDoneBarButton];
}

- (void)viewWillAppear:(BOOL)animated {
  [super viewWillAppear:animated];
}

#pragma mark - Data source

- (NSArray<NSString *> *)videoResolutionArray {
  return [_settingsModel availableVideoResolutions];
}

/*
- (NSArray<RTCVideoCodecInfo *> *)videoCodecArray {
  return [_settingsModel availableVideoCodecs];
}
*/

#pragma mark -

- (void)addDoneBarButton {
  UIBarButtonItem *barItem =
      [[UIBarButtonItem alloc] initWithBarButtonSystemItem:UIBarButtonSystemItemDone
                                                    target:self
                                                    action:@selector(dismissModally:)];
  self.navigationItem.leftBarButtonItem = barItem;
}

#pragma mark - Dismissal of view controller

- (void)dismissModally:(id)sender {
  [self dismissViewControllerAnimated:YES completion:nil];
}

#pragma mark - Table view data source

- (NSInteger)numberOfSectionsInTableView:(UITableView *)tableView {
  return 9;
}

- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section {
  switch (section) {
    case ARDSettingsSectionAudioSettings:
      return 3;
    case ARDSettingsSectionVideoResolution:
      return self.videoResolutionArray.count;
    default:
      return 1;
  }
}

#pragma mark - Table view delegate helpers

- (void)removeAllAccessories:(UITableView *)tableView
                   inSection:(int)section
{
  for (int i = 0; i < [tableView numberOfRowsInSection:section]; i++) {
    NSIndexPath *rowPath = [NSIndexPath indexPathForRow:i inSection:section];
    UITableViewCell *cell = [tableView cellForRowAtIndexPath:rowPath];
    cell.accessoryType = UITableViewCellAccessoryNone;
  }
}

- (void)tableView:(UITableView *)tableView
updateListSelectionAtIndexPath:(NSIndexPath *)indexPath
        inSection:(int)section {
  [self removeAllAccessories:tableView inSection:section];
  UITableViewCell *cell = [tableView cellForRowAtIndexPath:indexPath];
  cell.accessoryType = UITableViewCellAccessoryCheckmark;
  [tableView deselectRowAtIndexPath:indexPath animated:YES];
}

#pragma mark - Table view delegate

- (nullable NSString *)tableView:(UITableView *)tableView
         titleForHeaderInSection:(NSInteger)section {
  switch (section) {
    case ARDSettingsSectionAudioSettings:
      return @"Audio";
    case ARDSettingsSectionVideoResolution:
      return @"Video resolution";
    case ARDSettingsSectionBitRate:
      return @"Maximum bitrate";
    case ARDSettingsSectionJanus:
      return @"Janus";
    case ARDSettingsSectionTestStreamName:
      return @"Stream Name";
    case ARDSettingsSectionUid:
      return @"Uid";
    case ARDSettingsSectionFps:
      return @"Fps";
    case ARDSettingsSectionLoopback:
      return @"Loopback";
    default:
      return @"";
  }
}

- (UITableViewCell *)tableView:(UITableView *)tableView
         cellForRowAtIndexPath:(NSIndexPath *)indexPath {
  switch (indexPath.section) {
    case ARDSettingsSectionAudioSettings:
      return [self audioSettingsTableViewCellForTableView:tableView atIndexPath:indexPath];

    case ARDSettingsSectionVideoResolution:
      return [self videoResolutionTableViewCellForTableView:tableView atIndexPath:indexPath];

    case ARDSettingsSectionBitRate:
      return [self bitrateTableViewCellForTableView:tableView atIndexPath:indexPath];

    case ARDSettingsSectionJanus:
      return [self janusTableViewCellForTableView:tableView atIndexPath:indexPath];
    
    case ARDSettingsSectionTestStreamName:
      return [self testStreamNameTableViewCellForTableView:tableView atIndexPath:indexPath];

    case ARDSettingsSectionUid:
      return [self uidTableViewCellForTableView:tableView atIndexPath:indexPath];
    
    case ARDSettingsSectionFps:
      return [self fpsTableViewCellForTableView:tableView atIndexPath:indexPath];
    
    case ARDSettingsSectionLoopback:
      return [self loopbackSettingsTableViewCellForTableView:tableView atIndexPath:indexPath];

    default:
      return [[UITableViewCell alloc] initWithStyle:UITableViewCellStyleDefault
                                    reuseIdentifier:@"identifier"];
  }
}

- (void)tableView:(UITableView *)tableView didSelectRowAtIndexPath:(NSIndexPath *)indexPath {
  switch (indexPath.section) {
    case ARDSettingsSectionVideoResolution:
      [self tableView:tableView disSelectVideoResolutionAtIndex:indexPath];
      break;
  }
}

#pragma mark - Table view delegate(Video Resolution)

- (UITableViewCell *)videoResolutionTableViewCellForTableView:(UITableView *)tableView
                                                  atIndexPath:(NSIndexPath *)indexPath {
  NSString *dequeueIdentifier = @"ARDSettingsVideoResolutionViewCellIdentifier";
  UITableViewCell *cell = [tableView dequeueReusableCellWithIdentifier:dequeueIdentifier];
  if (!cell) {
    cell = [[UITableViewCell alloc] initWithStyle:UITableViewCellStyleDefault
                                  reuseIdentifier:dequeueIdentifier];
  }
  NSString *resolution = self.videoResolutionArray[indexPath.row];
  cell.textLabel.text = resolution;
  if ([resolution isEqualToString:[_settingsModel currentVideoResolutionSettingFromStore]]) {
    cell.accessoryType = UITableViewCellAccessoryCheckmark;
  } else {
    cell.accessoryType = UITableViewCellAccessoryNone;
  }

  return cell;
}

- (void)tableView:(UITableView *)tableView
    disSelectVideoResolutionAtIndex:(NSIndexPath *)indexPath {
  [self tableView:tableView
      updateListSelectionAtIndexPath:indexPath
                           inSection:ARDSettingsSectionVideoResolution];

  NSString *videoResolution = self.videoResolutionArray[indexPath.row];
  [_settingsModel storeVideoResolutionSetting:videoResolution];
}

#pragma mark - Table view delegate(Bitrate)

- (UITableViewCell *)bitrateTableViewCellForTableView:(UITableView *)tableView
                                          atIndexPath:(NSIndexPath *)indexPath {
  NSString *dequeueIdentifier = @"ARDSettingsBitrateCellIdentifier";
  UITableViewCell *cell = [tableView dequeueReusableCellWithIdentifier:dequeueIdentifier];
  if (!cell) {
    cell = [[UITableViewCell alloc] initWithStyle:UITableViewCellStyleDefault
                                  reuseIdentifier:dequeueIdentifier];

    UITextField *textField = [[UITextField alloc]
        initWithFrame:CGRectMake(10, 0, cell.bounds.size.width - 20, cell.bounds.size.height)];
    NSString *currentMaxBitrate = [_settingsModel currentMaxBitrateSettingFromStore].stringValue;
    textField.text = currentMaxBitrate;
    textField.placeholder = @"Enter max bit rate (kbps)";
    textField.keyboardType = UIKeyboardTypeNumberPad;
    textField.delegate = self;
    textField.tag = ARDTextFiledTagMaxBitrate;

    // Numerical keyboards have no return button, we need to add one manually.
    UIToolbar *numberToolbar =
        [[UIToolbar alloc] initWithFrame:CGRectMake(0, 0, self.view.bounds.size.width, 50)];
    numberToolbar.items = @[
      [[UIBarButtonItem alloc] initWithBarButtonSystemItem:UIBarButtonSystemItemFlexibleSpace
                                                    target:nil
                                                    action:nil],
      [[UIBarButtonItem alloc] initWithTitle:@"Apply"
                                       style:UIBarButtonItemStyleDone
                                      target:self
                                      action:@selector(numberTextFieldDidEndEditing:)]
    ];
    [numberToolbar sizeToFit];

    textField.inputAccessoryView = numberToolbar;
    [cell addSubview:textField];
  }
  return cell;
}

- (void)numberTextFieldDidEndEditing:(id)sender {
  [self.view endEditing:YES];
}

- (void)textFieldDidEndEditing:(UITextField *)textField {
  switch(textField.tag) {
    case ARDTextFiledTagMaxBitrate: {
      NSNumber *bitrateNumber = nil;
      if (textField.text.length != 0) {
        bitrateNumber = [NSNumber numberWithInteger:textField.text.intValue];
      }
      [_settingsModel storeMaxBitrateSetting:bitrateNumber];
      break;
    }
    case ARDTextFiledTagJanus: {
      NSString *url = nil;
      if (textField.text.length != 0) {
        url = textField.text;
      }
      [_settingsModel storeJanusSetting:url];
      break;
    }
    case ARDTextFiledTagTestStreamName: {
      NSString *streamName = nil;
      if (textField.text.length != 0) {
        streamName = textField.text;
      }
      [_settingsModel storeTestStreamNameSetting:streamName];
      break;
    }
    case ARDTextFiledTagUid: {
      NSString *uid = nil;
      if (textField.text.length != 0) {
        uid = textField.text;
      }
      [_settingsModel storeUidSetting:uid];
      break;
    }
    case ARDTextFiledTagFps: {
      NSNumber *fps = nil;
      if (textField.text.length != 0) {
        fps = [NSNumber numberWithInteger:textField.text.intValue];
      }
      [_settingsModel storeFpsSetting:fps];
      break;
    }
  }
}

#pragma mark - Table view delegate(Janus)

- (UITableViewCell *)janusTableViewCellForTableView:(UITableView *)tableView
                                          atIndexPath:(NSIndexPath *)indexPath {
    NSString *dequeueIdentifier = @"ARDSettingsJanusCellIdentifier";
    UITableViewCell *cell = [tableView dequeueReusableCellWithIdentifier:dequeueIdentifier];
    if (!cell) {
        cell = [[UITableViewCell alloc] initWithStyle:UITableViewCellStyleDefault
                                      reuseIdentifier:dequeueIdentifier];
        
        UITextField *textField = [[UITextField alloc]
                                  initWithFrame:CGRectMake(10, 0, cell.bounds.size.width - 20, cell.bounds.size.height)];
        NSString *currentJanus = [_settingsModel currentJanusSettingFromStore];
        textField.text = currentJanus;
        textField.placeholder = @"Enter Janus Address";
        textField.keyboardType = UIKeyboardTypeDefault;
        textField.delegate = self;
        textField.tag = ARDTextFiledTagJanus;
        
        UIToolbar *numberToolbar =
        [[UIToolbar alloc] initWithFrame:CGRectMake(0, 0, self.view.bounds.size.width, 50)];
        numberToolbar.items = @[
                                [[UIBarButtonItem alloc] initWithBarButtonSystemItem:UIBarButtonSystemItemFlexibleSpace
                                                                              target:nil
                                                                              action:nil],
                                [[UIBarButtonItem alloc] initWithTitle:@"Apply"
                                                                 style:UIBarButtonItemStyleDone
                                                                target:self
                                                                action:@selector(numberTextFieldDidEndEditing:)]
                                ];
        [numberToolbar sizeToFit];
        
        textField.inputAccessoryView = numberToolbar;
        [cell addSubview:textField];
    }
    return cell;
}

#pragma mark - Table view delegate(Test Stream)

- (UITableViewCell *)testStreamNameTableViewCellForTableView:(UITableView *)tableView
                                                 atIndexPath:(NSIndexPath *)indexPath {
    NSString *dequeueIdentifier = @"ARDSettingsTestStreamNameCellIdentifier";
    UITableViewCell *cell = [tableView dequeueReusableCellWithIdentifier:dequeueIdentifier];
    if (!cell) {
        cell = [[UITableViewCell alloc] initWithStyle:UITableViewCellStyleDefault
                                      reuseIdentifier:dequeueIdentifier];
        
        UITextField *textField = [[UITextField alloc]
                                  initWithFrame:CGRectMake(10, 0, cell.bounds.size.width - 20, cell.bounds.size.height)];
        NSString *currentTestStreamName = [_settingsModel currentTestStreamNameSettingFromStore];
        textField.text = currentTestStreamName;
        textField.placeholder = @"Enter Test Stream Name.";
        textField.keyboardType = UIKeyboardTypeDefault;
        textField.delegate = self;
        textField.tag = ARDTextFiledTagTestStreamName;
        
        UIToolbar *numberToolbar =
        [[UIToolbar alloc] initWithFrame:CGRectMake(0, 0, self.view.bounds.size.width, 50)];
        numberToolbar.items = @[
                                [[UIBarButtonItem alloc] initWithBarButtonSystemItem:UIBarButtonSystemItemFlexibleSpace
                                                                              target:nil
                                                                              action:nil],
                                [[UIBarButtonItem alloc] initWithTitle:@"Apply"
                                                                 style:UIBarButtonItemStyleDone
                                                                target:self
                                                                action:@selector(numberTextFieldDidEndEditing:)]
                                ];
        [numberToolbar sizeToFit];
        
        textField.inputAccessoryView = numberToolbar;
        [cell addSubview:textField];
    }
    return cell;
}

#pragma mark - Table view delegate(Uid)

- (UITableViewCell *)uidTableViewCellForTableView:(UITableView *)tableView
                                      atIndexPath:(NSIndexPath *)indexPath {
    NSString *dequeueIdentifier = @"ARDSettingsUidCellIdentifier";
    UITableViewCell *cell = [tableView dequeueReusableCellWithIdentifier:dequeueIdentifier];
    if (!cell) {
        cell = [[UITableViewCell alloc] initWithStyle:UITableViewCellStyleDefault
                                      reuseIdentifier:dequeueIdentifier];
        
        UITextField *textField = [[UITextField alloc]
                                  initWithFrame:CGRectMake(10, 0, cell.bounds.size.width - 20, cell.bounds.size.height)];
        NSString *currentUid = [_settingsModel currentUidSettingFromStore];
        textField.text = currentUid;
        textField.placeholder = @"Enter uid";
        textField.keyboardType = UIKeyboardTypeDefault;
        textField.delegate = self;
        textField.tag = ARDTextFiledTagUid;
        
        UIToolbar *numberToolbar =
        [[UIToolbar alloc] initWithFrame:CGRectMake(0, 0, self.view.bounds.size.width, 50)];
        numberToolbar.items = @[
                                [[UIBarButtonItem alloc] initWithBarButtonSystemItem:UIBarButtonSystemItemFlexibleSpace
                                                                              target:nil
                                                                              action:nil],
                                [[UIBarButtonItem alloc] initWithTitle:@"Apply"
                                                                 style:UIBarButtonItemStyleDone
                                                                target:self
                                                                action:@selector(numberTextFieldDidEndEditing:)]
                                ];
        [numberToolbar sizeToFit];
        
        textField.inputAccessoryView = numberToolbar;
        [cell addSubview:textField];
    }
    return cell;
}

#pragma mark - Table view delegate(Fps)

- (UITableViewCell *)fpsTableViewCellForTableView:(UITableView *)tableView
                                      atIndexPath:(NSIndexPath *)indexPath {
    NSString *dequeueIdentifier = @"ARDSettingsFpsCellIdentifier";
    UITableViewCell *cell = [tableView dequeueReusableCellWithIdentifier:dequeueIdentifier];
    if (!cell) {
        cell = [[UITableViewCell alloc] initWithStyle:UITableViewCellStyleDefault
                                      reuseIdentifier:dequeueIdentifier];
        
        UITextField *textField = [[UITextField alloc]
                                  initWithFrame:CGRectMake(10, 0, cell.bounds.size.width - 20, cell.bounds.size.height)];
        NSString *currentFps = [_settingsModel currentFpsSettingFromStore].stringValue;
        textField.text = currentFps;
        textField.placeholder = @"Enter fps";
        textField.keyboardType = UIKeyboardTypeNumberPad;
        textField.delegate = self;
        textField.tag = ARDTextFiledTagFps;
        
        UIToolbar *numberToolbar =
        [[UIToolbar alloc] initWithFrame:CGRectMake(0, 0, self.view.bounds.size.width, 50)];
        numberToolbar.items = @[
                                [[UIBarButtonItem alloc] initWithBarButtonSystemItem:UIBarButtonSystemItemFlexibleSpace
                                                                              target:nil
                                                                              action:nil],
                                [[UIBarButtonItem alloc] initWithTitle:@"Apply"
                                                                 style:UIBarButtonItemStyleDone
                                                                target:self
                                                                action:@selector(numberTextFieldDidEndEditing:)]
                                ];
        [numberToolbar sizeToFit];
        
        textField.inputAccessoryView = numberToolbar;
        [cell addSubview:textField];
    }
    return cell;
}

#pragma mark - Table view delegate(Loopback settings)

- (UITableViewCell *)loopbackSettingsTableViewCellForTableView:(UITableView *)tableView
                                                   atIndexPath:(NSIndexPath *)indexPath {
    NSString *dequeueIdentifier = @"ARDSettingsLoopbackSettingsCellIdentifier";
    UITableViewCell *cell = [tableView dequeueReusableCellWithIdentifier:dequeueIdentifier];
    if (!cell) {
        cell = [[UITableViewCell alloc] initWithStyle:UITableViewCellStyleDefault
                                      reuseIdentifier:dequeueIdentifier];
        cell.selectionStyle = UITableViewCellSelectionStyleNone;
        UISwitch *switchView = [[UISwitch alloc] initWithFrame:CGRectZero];
        switchView.tag = indexPath.row;
        [switchView addTarget:self
                       action:@selector(loopbackSettingSwitchChanged:)
             forControlEvents:UIControlEventValueChanged];
        cell.accessoryView = switchView;
    }
    
    cell.textLabel.text = @"Enable";
    UISwitch *switchView = (UISwitch *)cell.accessoryView;
    switchView.on = [_settingsModel currentLoopbackSettingFromStore];
    
    return cell;
}

- (void)loopbackSettingSwitchChanged:(UISwitch *)sender {
    [_settingsModel storeLoopbackSetting:sender.isOn];
}

#pragma mark - Table view delegate(Audio settings)

- (UITableViewCell *)audioSettingsTableViewCellForTableView:(UITableView *)tableView
                                                atIndexPath:(NSIndexPath *)indexPath {
  NSString *dequeueIdentifier = @"ARDSettingsAudioSettingsCellIdentifier";
  UITableViewCell *cell = [tableView dequeueReusableCellWithIdentifier:dequeueIdentifier];
  if (!cell) {
    cell = [[UITableViewCell alloc] initWithStyle:UITableViewCellStyleDefault
                                  reuseIdentifier:dequeueIdentifier];
    cell.selectionStyle = UITableViewCellSelectionStyleNone;
    UISwitch *switchView = [[UISwitch alloc] initWithFrame:CGRectZero];
    switchView.tag = indexPath.row;
    [switchView addTarget:self
                   action:@selector(audioSettingSwitchChanged:)
         forControlEvents:UIControlEventValueChanged];
    cell.accessoryView = switchView;
  }

  cell.textLabel.text = [self labelForAudioSettingAtIndexPathRow:indexPath.row];
  UISwitch *switchView = (UISwitch *)cell.accessoryView;
  switchView.on = [self valueForAudioSettingAtIndexPathRow:indexPath.row];

  return cell;
}

- (NSString *)labelForAudioSettingAtIndexPathRow:(NSInteger)setting {
  switch (setting) {
    case ARDAudioSettingsAudioOnly:
      return @"Audio only";
    case ARDAudioSettingsCreateAecDump:
      return @"Create AecDump";
    case ARDAudioSettingsUseLevelController:
      return @"Use level controller";
    case ARDAudioSettingsUseManualAudioConfig:
      return @"Use manual audio config";
    default:
      return @"";
  }
}

- (BOOL)valueForAudioSettingAtIndexPathRow:(NSInteger)setting {
  switch (setting) {
    case ARDAudioSettingsAudioOnly:
      return [_settingsModel currentAudioOnlySettingFromStore];
    case ARDAudioSettingsCreateAecDump:
      return [_settingsModel currentCreateAecDumpSettingFromStore];
    case ARDAudioSettingsUseLevelController:
      return [_settingsModel currentUseLevelControllerSettingFromStore];
    case ARDAudioSettingsUseManualAudioConfig:
      return [_settingsModel currentUseManualAudioConfigSettingFromStore];
    default:
      return NO;
  }
}

- (void)audioSettingSwitchChanged:(UISwitch *)sender {
  switch (sender.tag) {
    case ARDAudioSettingsAudioOnly: {
      [_settingsModel storeAudioOnlySetting:sender.isOn];
      break;
    }
    case ARDAudioSettingsCreateAecDump: {
      [_settingsModel storeCreateAecDumpSetting:sender.isOn];
      break;
    }
    case ARDAudioSettingsUseLevelController: {
      [_settingsModel storeUseLevelControllerSetting:sender.isOn];
      break;
    }
    case ARDAudioSettingsUseManualAudioConfig: {
      [_settingsModel storeUseManualAudioConfigSetting:sender.isOn];
      break;
    }
    default:
      break;
  }
}

@end
NS_ASSUME_NONNULL_END
