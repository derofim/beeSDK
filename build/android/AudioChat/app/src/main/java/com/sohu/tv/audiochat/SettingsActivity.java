/*
 *  Copyright 2014 The WebRTC Project Authors. All rights reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

package com.sohu.tv.audiochat;

import android.app.Activity;
import android.content.SharedPreferences;
import android.content.SharedPreferences.OnSharedPreferenceChangeListener;
import android.os.Build;
import android.os.Bundle;
import android.preference.ListPreference;
import android.preference.Preference;

import org.webrtc.voiceengine.WebRtcAudioUtils;

/**
 * Settings activity for AppRTC.
 */
public class SettingsActivity extends Activity implements OnSharedPreferenceChangeListener {
  private SettingsFragment settingsFragment;



  private String keyprefStartAudioBitrateType;
  private String keyprefStartAudioBitrateValue;
  private String keyPrefAudioCodec;
  private String keyprefNoAudioProcessing;
  private String keyprefOpenSLES;
  private String keyprefDisableBuiltInAEC;
  private String keyprefDisableBuiltInAGC;
  private String keyprefDisableBuiltInNS;
  private String keyprefEnableLevelControl;
  private String keyprefSpeakerphone;
  private String keyPrefShowVolume;

  private String keyPrefDisplayHud;
  private String keyprefLogcat;
  private String keyprefStatusd;
  private String keyprefDeviceId;
  private String keyprefVersionId;


  @Override
  protected void onCreate(Bundle savedInstanceState) {
    super.onCreate(savedInstanceState);
    keyprefStartAudioBitrateType = getString(R.string.pref_startaudiobitrate_key);
    keyprefStartAudioBitrateValue = getString(R.string.pref_startaudiobitratevalue_key);
    keyPrefAudioCodec = getString(R.string.pref_audiocodec_key);
    keyprefNoAudioProcessing = getString(R.string.pref_noaudioprocessing_key);
    keyprefOpenSLES = getString(R.string.pref_opensles_key);
    keyprefDisableBuiltInAEC = getString(R.string.pref_disable_built_in_aec_key);
    keyprefDisableBuiltInAGC = getString(R.string.pref_disable_built_in_agc_key);
    keyprefDisableBuiltInNS = getString(R.string.pref_disable_built_in_ns_key);
    keyPrefShowVolume = getString(R.string.pref_show_volume_key);
    keyprefEnableLevelControl = getString(R.string.pref_enable_level_control_key);
    keyprefSpeakerphone = getString(R.string.pref_speakerphone_key);
    keyPrefDisplayHud = getString(R.string.pref_displayhud_key);
    keyprefLogcat = getString(R.string.pref_logcat_key);
    keyprefStatusd = getString(R.string.pref_enable_statusd_key);
    keyprefDeviceId = getString(R.string.pref_device_id_key);
    keyprefVersionId = getString(R.string.pref_version_key);

    // Display the fragment as the main content.
    settingsFragment = new SettingsFragment();
    getFragmentManager()
        .beginTransaction()
        .replace(android.R.id.content, settingsFragment)
        .commit();
  }

  @Override
  protected void onResume() {
    super.onResume();
    // Set summary to be the user-description for the selected value
    SharedPreferences sharedPreferences =
        settingsFragment.getPreferenceScreen().getSharedPreferences();
    sharedPreferences.registerOnSharedPreferenceChangeListener(this);

    updateSummary(sharedPreferences, keyprefStartAudioBitrateType);
    updateSummaryBitrate(sharedPreferences, keyprefStartAudioBitrateValue);
    setAudioBitrateEnable(sharedPreferences);
    updateSummary(sharedPreferences, keyPrefAudioCodec);
    updateSummaryB(sharedPreferences, keyprefNoAudioProcessing);
    updateSummaryB(sharedPreferences, keyprefOpenSLES);
    updateSummaryB(sharedPreferences, keyprefDisableBuiltInAEC);
    updateSummaryB(sharedPreferences, keyprefDisableBuiltInAGC);
    updateSummaryB(sharedPreferences, keyprefDisableBuiltInNS);
    updateSummaryB(sharedPreferences, keyPrefShowVolume);
    updateSummaryB(sharedPreferences, keyprefEnableLevelControl);
    updateSummaryList(sharedPreferences, keyprefSpeakerphone);
    updateSummaryB(sharedPreferences, keyPrefDisplayHud);
    updateSummaryB(sharedPreferences, keyprefLogcat);
    updateSummaryB(sharedPreferences, keyprefStatusd);
    setDeviceId(sharedPreferences);
    updateSummary(sharedPreferences, keyprefDeviceId);
    updateSummary(sharedPreferences, keyprefVersionId);

    // Disable forcing WebRTC based AEC so it won't affect our value.
    // Otherwise, if it was enabled, isAcousticEchoCancelerSupported would always return false.
    WebRtcAudioUtils.setWebRtcBasedAcousticEchoCanceler(false);
    if (!WebRtcAudioUtils.isAcousticEchoCancelerSupported()) {
      Preference disableBuiltInAECPreference =
          settingsFragment.findPreference(keyprefDisableBuiltInAEC);

      disableBuiltInAECPreference.setSummary(getString(R.string.pref_built_in_aec_not_available));
      disableBuiltInAECPreference.setEnabled(false);
    }

    WebRtcAudioUtils.setWebRtcBasedAutomaticGainControl(false);
    if (!WebRtcAudioUtils.isAutomaticGainControlSupported()) {
      Preference disableBuiltInAGCPreference =
          settingsFragment.findPreference(keyprefDisableBuiltInAGC);

      disableBuiltInAGCPreference.setSummary(getString(R.string.pref_built_in_agc_not_available));
      disableBuiltInAGCPreference.setEnabled(false);
    }

    WebRtcAudioUtils.setWebRtcBasedNoiseSuppressor(false);
    if (!WebRtcAudioUtils.isNoiseSuppressorSupported()) {
      Preference disableBuiltInNSPreference =
          settingsFragment.findPreference(keyprefDisableBuiltInNS);

      disableBuiltInNSPreference.setSummary(getString(R.string.pref_built_in_ns_not_available));
      disableBuiltInNSPreference.setEnabled(false);
    }
  }

  @Override
  protected void onPause() {
    super.onPause();
    SharedPreferences sharedPreferences =
        settingsFragment.getPreferenceScreen().getSharedPreferences();
    sharedPreferences.unregisterOnSharedPreferenceChangeListener(this);
  }

  @Override
  public void onSharedPreferenceChanged(SharedPreferences sharedPreferences, String key) {
    // clang-format off
    if (key.equals(keyprefStartAudioBitrateType)
        || key.equals(keyPrefAudioCodec)
        || key.equals(keyprefDeviceId)
        || key.equals(keyprefVersionId)) {
      updateSummary(sharedPreferences, key);
    } else if (key.equals(keyprefLogcat)
        || key.equals(keyprefStatusd)
        || key.equals(keyprefNoAudioProcessing)
        || key.equals(keyprefOpenSLES)
        || key.equals(keyprefDisableBuiltInAEC)
        || key.equals(keyprefDisableBuiltInAGC)
        || key.equals(keyprefDisableBuiltInNS)
        || key.equals(keyPrefShowVolume)
        || key.equals(keyprefEnableLevelControl)
        || key.equals(keyPrefDisplayHud)) {
      updateSummaryB(sharedPreferences, key);
    } else if (key.equals(keyprefSpeakerphone)) {
      updateSummaryList(sharedPreferences, key);
    }
    // clang-format on
    if (key.equals(keyprefStartAudioBitrateType)) {
      setAudioBitrateEnable(sharedPreferences);
    }
  }

  private void updateSummary(SharedPreferences sharedPreferences, String key) {
    Preference updatedPref = settingsFragment.findPreference(key);
    // Set summary to be the user-description for the selected value
    updatedPref.setSummary(sharedPreferences.getString(key, ""));
  }

  private void updateSummaryBitrate(SharedPreferences sharedPreferences, String key) {
    Preference updatedPref = settingsFragment.findPreference(key);
    updatedPref.setSummary(sharedPreferences.getString(key, "") + " kbps");
  }

  private void updateSummaryB(SharedPreferences sharedPreferences, String key) {
    Preference updatedPref = settingsFragment.findPreference(key);
    updatedPref.setSummary(sharedPreferences.getBoolean(key, true)
            ? getString(R.string.pref_value_true)
            : getString(R.string.pref_value_false));
  }

  private void updateSummaryList(SharedPreferences sharedPreferences, String key) {
    ListPreference updatedPref = (ListPreference) settingsFragment.findPreference(key);
    updatedPref.setSummary(updatedPref.getEntry());
  }

  private void setAudioBitrateEnable(SharedPreferences sharedPreferences) {
    Preference bitratePreferenceValue =
        settingsFragment.findPreference(keyprefStartAudioBitrateValue);
    String bitrateTypeDefault = getString(R.string.pref_startaudiobitrate_default);
    String bitrateType =
        sharedPreferences.getString(keyprefStartAudioBitrateType, bitrateTypeDefault);
    if (bitrateType.equals(bitrateTypeDefault)) {
      bitratePreferenceValue.setEnabled(false);
    } else {
      bitratePreferenceValue.setEnabled(true);
    }
  }

  private void setDeviceId(SharedPreferences sharedPreferences) {
    String deviceId = sharedPreferences.getString(keyprefDeviceId, "");
    if (deviceId.equals("")) {
      SharedPreferences.Editor editor = sharedPreferences.edit();
      editor.putString(keyprefDeviceId, getPseudoId());
      editor.commit();
    }
  }

  private String getPseudoId(){
    String id = "35" +//we make this look like a valid IMEI
            Build.BOARD.length()%10 +
            Build.BRAND.length()%10 +
            Build.CPU_ABI.length()%10 +
            Build.DEVICE.length()%10 +
            Build.DISPLAY.length()%10 +
            Build.HOST.length()%10 +
            Build.ID.length()%10 +
            Build.MANUFACTURER.length()%10 +
            Build.MODEL.length()%10 +
            Build.PRODUCT.length()%10 +
            Build.TAGS.length()%10 +
            Build.TYPE.length()%10 +
            Build.USER.length()%10; //13 digits
    return id;
  }
}
