/*
 *  Copyright 2015 The WebRTC Project Authors. All rights reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

package com.sohu.tv.beedemo;

import android.app.Activity;
import android.app.Fragment;
import android.os.Bundle;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Button;
import android.widget.ImageButton;
import android.widget.RelativeLayout;
import android.widget.SeekBar;
import android.widget.TextView;

import org.webrtc.RendererCommon.ScalingType;

/**
 * Fragment for call control.
 */
public class CallFragment extends Fragment {
  private View controlView;
  private TextView contactView;
  private TextView noNetView;
  private RelativeLayout reconnect_layout;
  private Button reConnect_btn;
  private ImageButton disconnectButton;
  private ImageButton cameraSwitchButton;
  private ImageButton videoScalingButton;
  private ImageButton toggleMuteButton;
  private TextView captureFormatText;
  private SeekBar captureFormatSlider;
  private OnCallEvents callEvents;
  private ScalingType scalingType;
  private boolean videoCallEnabled = true;

  @Override
  public View onCreateView(
      LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
    controlView = inflater.inflate(R.layout.fragment_call, container, false);

    // Create UI controls.
    noNetView = (TextView) controlView.findViewById(R.id.no_net_view);
    reconnect_layout = (RelativeLayout) controlView.findViewById(R.id.reconnect_layout);
    reConnect_btn = (Button) controlView.findViewById(R.id.reConnect_btn);
    contactView = (TextView) controlView.findViewById(R.id.contact_name_call);
    disconnectButton = (ImageButton) controlView.findViewById(R.id.button_call_disconnect);
    cameraSwitchButton = (ImageButton) controlView.findViewById(R.id.button_call_switch_camera);
    videoScalingButton = (ImageButton) controlView.findViewById(R.id.button_call_scaling_mode);
    toggleMuteButton = (ImageButton) controlView.findViewById(R.id.button_call_toggle_mic);
    captureFormatText = (TextView) controlView.findViewById(R.id.capture_format_text_call);
    captureFormatSlider = (SeekBar) controlView.findViewById(R.id.capture_format_slider_call);

    // Add buttons click events.
    reConnect_btn.setOnClickListener(new View.OnClickListener() {
      @Override
      public void onClick(View v) {
        setNotNetLayout(false);
        setReconnectLayout(false);
        callEvents.onCallReconnect();
      }
    });

    disconnectButton.setOnClickListener(new View.OnClickListener() {
      @Override
      public void onClick(View view) {
        callEvents.onCallHangUp();
      }
    });

    cameraSwitchButton.setOnClickListener(new View.OnClickListener() {
      @Override
      public void onClick(View view) {
        callEvents.onCameraSwitch();
      }
    });

    videoScalingButton.setOnClickListener(new View.OnClickListener() {
      @Override
      public void onClick(View view) {
        if (scalingType == ScalingType.SCALE_ASPECT_FILL) {
          videoScalingButton.setBackgroundResource(R.drawable.ic_action_full_screen);
          scalingType = ScalingType.SCALE_ASPECT_FIT;
        } else {
          videoScalingButton.setBackgroundResource(R.drawable.ic_action_return_from_full_screen);
          scalingType = ScalingType.SCALE_ASPECT_FILL;
        }
        callEvents.onVideoScalingSwitch(scalingType);
      }
    });
    scalingType = ScalingType.SCALE_ASPECT_FILL;

    toggleMuteButton.setOnClickListener(new View.OnClickListener() {
      @Override
      public void onClick(View view) {
        boolean enabled = callEvents.onToggleMic();
        toggleMuteButton.setAlpha(enabled ? 1.0f : 0.3f);
      }
    });

    return controlView;
  }

  @Override
  public void onStart() {
    super.onStart();

    boolean captureSliderEnabled = false;
    Bundle args = getArguments();
    if (args != null) {
      String contactName = args.getString(CallActivity.EXTRA_ROOMID);
      contactView.setText(contactName);
      videoCallEnabled = args.getBoolean(CallActivity.EXTRA_VIDEO_CALL, true);
      captureSliderEnabled = videoCallEnabled
          && args.getBoolean(CallActivity.EXTRA_VIDEO_CAPTUREQUALITYSLIDER_ENABLED, false);
    }
    if (!videoCallEnabled) {
      cameraSwitchButton.setVisibility(View.INVISIBLE);
    }
    if (captureSliderEnabled) {
      captureFormatSlider.setOnSeekBarChangeListener(
          new CaptureQualityController(captureFormatText, callEvents));
    } else {
      captureFormatText.setVisibility(View.GONE);
      captureFormatSlider.setVisibility(View.GONE);
    }
  }

  // TODO(sakal): Replace with onAttach(Context) once we only support API level 23+.
  @SuppressWarnings("deprecation")
  @Override
  public void onAttach(Activity activity) {
    super.onAttach(activity);
    callEvents = (OnCallEvents) activity;
  }

  public void setReconnectLayout(boolean isShow) {
    if (isShow) {
      reconnect_layout.setVisibility(View.VISIBLE);
    } else {
      reconnect_layout.setVisibility(View.GONE);
    }
  }

  public void setNotNetLayout(boolean isShow) {
    if (isShow) {
      noNetView.setVisibility(View.VISIBLE);
    } else {
      noNetView.setVisibility(View.GONE);
    }
  }

}
