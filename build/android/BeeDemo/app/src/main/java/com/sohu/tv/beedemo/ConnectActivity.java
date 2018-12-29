/*
 *  Copyright 2014 The WebRTC Project Authors. All rights reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

package com.sohu.tv.beedemo;

import android.Manifest;
import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.pm.PackageManager;
import android.net.Uri;
import android.os.Build;
import android.os.Bundle;
import android.os.Debug;
import android.os.Environment;
import android.preference.PreferenceManager;
import android.support.v4.app.ActivityCompat;
import android.support.v4.content.ContextCompat;
import android.util.Log;
import android.view.ContextMenu;
import android.view.KeyEvent;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.inputmethod.EditorInfo;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.EditText;
import android.widget.ImageButton;
import android.widget.ListView;
import android.widget.TextView;
import android.widget.Toast;

import com.sohu.tv.bee.BeeAsyncHandler;
import com.sohu.tv.bee.BeeSDK;
import com.sohu.tv.bee.BeeSDKSink;
import com.sohu.tv.bee.BeeSystemDefine;
import com.sohu.tv.bee.BeeSystemParam;

import java.io.BufferedReader;
import java.io.DataOutputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.net.HttpURLConnection;
import java.net.URL;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Date;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Random;
import java.util.Timer;
import java.util.TimerTask;
import java.util.concurrent.Executors;
import java.util.concurrent.ScheduledExecutorService;

import org.json.JSONArray;
import org.json.JSONException;
import org.webrtc.Logging;

/**
 * Handles the initial setup where the user selects which room to join.
 */
public class ConnectActivity extends Activity implements BeeSDKSink {
  private static final String TAG = "ConnectActivity";
  private static final int CONNECTION_REQUEST = 1;
  private static final int REMOVE_FAVORITE_INDEX = 0;
  private static boolean commandLineRun = false;
  private static final int CHECK_DMP_TIMER_PERIOD = 5000;
  private static final int NATIVE_MEMORY_MONITOR_PERIOD = 1000;
  private static final boolean isCreate = true;

  private Toast logToast;
  private final ScheduledExecutorService executor = Executors.newSingleThreadScheduledExecutor();
  private Button createButton;
  private Button joinButton;
  private ImageButton addFavoriteButton;
  private ImageButton uploadButton;
  private ImageButton clearButton;
  private ImageButton settingButton;
  private EditText roomEditText;
  private ListView roomListView;
  private SharedPreferences sharedPref;
  private String keyprefVideoCallEnabled;
  private String keyprefScreencapture;
  private String keyprefCamera2;
  private String keyprefResolution;
  private String keyprefFps;
  private String keyprefCaptureQualitySlider;
  private String keyprefVideoBitrateType;
  private String keyprefVideoBitrateValue;
  private String keyprefVideoCodec;
  private String keyprefAudioBitrateType;
  private String keyprefAudioBitrateValue;
  private String keyprefAudioCodec;
  private String keyprefHwCodecAcceleration;
  private String keyprefCaptureToTexture;
  private String keyprefFlexfec;
  private String keyprefNoAudioProcessingPipeline;
  private String keyprefAecDump;
  private String keyprefOpenSLES;
  private String keyprefDisableBuiltInAec;
  private String keyprefDisableBuiltInAgc;
  private String keyprefDisableBuiltInNs;
  private String keyprefEnableLevelControl;
  private String keyprefDisplayHud;
  private String keyprefTracing;
  private String keyprefRoomServerUrl;
  private String keyprefRoom;
  private String keyprefRoomList;
  private ArrayList<String> roomList;
  private ArrayAdapter<String> adapter;
  private String keyprefEnableDataChannel;
  private String keyprefOrdered;
  private String keyprefMaxRetransmitTimeMs;
  private String keyprefMaxRetransmits;
  private String keyprefDataProtocol;
  private String keyprefNegotiated;
  private String keyprefDataId;
  private String logPath;
  private String keyprefLogcat;
  private Timer logTimer = new Timer();
  private Timer nativeMemoryMonitorTimer = new Timer();
  private boolean beeInitialized = false;
  private BeeSystemParam bee_param_ = new BeeSystemParam();

  public int httpPost(String actionUrl, Map<String, String> params, Map<String, File> files) throws IOException {
    String BOUNDARY = java.util.UUID.randomUUID().toString();
    String PREFIX = "--", LINEND = "\r\n";
    String MULTIPART_FROM_DATA = "multipart/form-data";
    String CHARSET = "UTF-8";
    URL uri = new URL(actionUrl);
    HttpURLConnection conn = (HttpURLConnection) uri.openConnection();
    conn.setReadTimeout(5 * 1000);
    conn.setDoInput(true);// 允许输入
    conn.setDoOutput(true);// 允许输出
    conn.setUseCaches(false);
    conn.setRequestMethod("POST"); // Post方式
    conn.setRequestProperty("connection", "keep-alive");
    conn.setRequestProperty("Charsert", "UTF-8");
    conn.setRequestProperty("Content-Type", MULTIPART_FROM_DATA + ";boundary=" + BOUNDARY);

    // 首先组拼文本类型的参数
    StringBuilder sb = new StringBuilder();
    for (Map.Entry<String, String> entry : params.entrySet()) {
      sb.append(PREFIX);
      sb.append(BOUNDARY);
      sb.append(LINEND);
      sb.append("Content-Disposition: form-data; name=\"" + entry.getKey() + "\"" + LINEND);
      sb.append("Content-Type: text/plain; charset=" + CHARSET + LINEND);
      sb.append("Content-Transfer-Encoding: 8bit" + LINEND);
      sb.append(LINEND);
      sb.append(entry.getValue());
      sb.append(LINEND);
    }

    DataOutputStream outStream = new DataOutputStream(conn.getOutputStream());
    outStream.write(sb.toString().getBytes());

    // 发送文件数据
    if (files != null) {
      for (Map.Entry<String, File> file : files.entrySet()) {
        StringBuilder sb1 = new StringBuilder();
        sb1.append(PREFIX);
        sb1.append(BOUNDARY);
        sb1.append(LINEND);
        sb1.append("Content-Disposition: form-data; name=\"upload_file_minidump\"; filename=\"" + file.getKey() + "\"" + LINEND);
        sb1.append("Content-Type: application/octet-stream; charset=" + CHARSET + LINEND);
        sb1.append(LINEND);
        outStream.write(sb1.toString().getBytes());
        InputStream is = new FileInputStream(file.getValue());
        byte[] buffer = new byte[1024];
        int len = 0;
        while ((len = is.read(buffer)) != -1) {
          outStream.write(buffer, 0, len);
        }

        is.close();
        outStream.write(LINEND.getBytes());
      }
    }

    // 请求结束标志
    byte[] end_data = (PREFIX + BOUNDARY + PREFIX + LINEND).getBytes();
    outStream.write(end_data);
    outStream.flush();

    // 得到响应码
    int res = conn.getResponseCode();
    InputStream in = conn.getInputStream();
    InputStreamReader isReader = new InputStreamReader(in);
    BufferedReader bufReader = new BufferedReader(isReader);
    String line = null;
    String data = "OK";

    while ((line = bufReader.readLine()) != null) {
      data += line;
    }

    Log.d(TAG, "rsp " + data);
    if (res == 200) {
      int ch;
      StringBuilder sb2 = new StringBuilder();
      while ((ch = in.read()) != -1) {
        sb2.append((char) ch);
      }
    }
    outStream.close();
    conn.disconnect();
    return res;
  }

  private void logAndToast(String msg) {
    Log.d(TAG, msg);
    if (logToast != null) {
      logToast.cancel();
    }
    logToast = Toast.makeText(this, msg, Toast.LENGTH_SHORT);
    logToast.show();
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

  private void reportLog() {
    Map<String, String> params = new HashMap<>();
    Map<String, File> files = new HashMap<>();
    File zipFile = null;
    try {
      String version = sharedPrefGetString(R.string.pref_version_key, null, R.string.pref_version_default, false);
      String[] vec = version.split(" ");
      if (vec.length == 2) {
        params.put("version", vec[0]);
        params.put("git", vec[1]);
      }
      long time = System.currentTimeMillis();
      SimpleDateFormat format = new SimpleDateFormat("yyyyMMdd_HHmmss");
      Date date = new Date(time);
      String zipFilName = format.format(date) + "-" + Build.BRAND + "-" + Build.MODEL + "-" + getPseudoId() +".zip";
      zipFilName = zipFilName.replaceAll(" ", "_");
      String zipFilePath = "/sdcard/" + zipFilName;
      ZipUtil.zipFolder(logPath, zipFilePath);
      zipFile = new File(zipFilePath);
      files.put(zipFilName, zipFile);
      final int rsp = httpPost("http://106.120.154.76/android.php", params, files);

      runOnUiThread(new Runnable() {
        @Override
        public void run() {
          if (rsp == 200) {
            logAndToast("上传日志成功");
          } else {
            logAndToast("上传日志错误 " + rsp);
          }
        }
      });
    } catch (final Exception e) {
      runOnUiThread(new Runnable() {
        @Override
        public void run() {
          logAndToast("Upload file fail." + e);
        }
      });
    } finally {
      if (zipFile != null) {
        zipFile.delete();
      }
    }
  }

  private void checkAndReportDumpLog() {
    do {
      if (logPath == null || executor == null) {
        break;
      }

      File dir = new File(logPath);
      if (!dir.exists() || !dir.isDirectory()) {
        break;
      }

      final List<File> dumpFiles = new ArrayList<>();
      boolean foundDump = false;
      for (File file : dir.listFiles()) {
        if (file.getName().endsWith(".dmp")) {
          foundDump = true;
          dumpFiles.add(file);
        }
      }

      if (!foundDump) {
        break;
      }

      executor.execute(new Runnable() {
        @Override
        public void run() {
          reportLog();
          //Delete dump files.
          for (File file : dumpFiles) {
            file.delete();
          }
        }
      });
    } while (false);
  }

  private void startCheckDmpLogTimer() {
    try {
      logTimer.schedule(new TimerTask() {
        @Override
        public void run() {
          checkAndReportDumpLog();
        }
      }, 0, CHECK_DMP_TIMER_PERIOD);
    } catch (Exception e) {
      Logging.e(TAG, "Can not schedule dmp log timer", e);
    }
  }

  private void startNativeMemoryMonitorTimer() {
    try {
      logTimer.schedule(new TimerTask() {
        @Override
        public void run() {
          Logging.d(TAG,"@@@ NativeHeapSizeTotal     : " + (Debug.getNativeHeapSize() >> 10) );
          Logging.d(TAG,"@@@ NativeAllocatedHeapSize : " + (Debug.getNativeHeapAllocatedSize() >> 10));
          Logging.d(TAG,"@@@ NativeAllocatedFree     : " + (Debug.getNativeHeapFreeSize() >> 10));
          Logging.d(TAG,"@@@-------------------------------------------------");
        }
      }, 0, NATIVE_MEMORY_MONITOR_PERIOD);
    } catch (Exception e) {
      Logging.e(TAG, "Can not schedule native memory monitor timer", e);
    }
  }

  @Override
  public void onCreate(Bundle savedInstanceState) {
    super.onCreate(savedInstanceState);

    // Get setting keys.
    PreferenceManager.setDefaultValues(this, R.xml.preferences, false);
    sharedPref = PreferenceManager.getDefaultSharedPreferences(this);
    keyprefVideoCallEnabled = getString(R.string.pref_videocall_key);
    keyprefScreencapture = getString(R.string.pref_screencapture_key);
    keyprefCamera2 = getString(R.string.pref_camera2_key);
    keyprefResolution = getString(R.string.pref_resolution_key);
    keyprefFps = getString(R.string.pref_fps_key);
    keyprefCaptureQualitySlider = getString(R.string.pref_capturequalityslider_key);
    keyprefVideoBitrateType = getString(R.string.pref_maxvideobitrate_key);
    keyprefVideoBitrateValue = getString(R.string.pref_maxvideobitratevalue_key);
    keyprefVideoCodec = getString(R.string.pref_videocodec_key);
    keyprefHwCodecAcceleration = getString(R.string.pref_hwcodec_key);
    keyprefCaptureToTexture = getString(R.string.pref_capturetotexture_key);
    keyprefFlexfec = getString(R.string.pref_flexfec_key);
    keyprefAudioBitrateType = getString(R.string.pref_startaudiobitrate_key);
    keyprefAudioBitrateValue = getString(R.string.pref_startaudiobitratevalue_key);
    keyprefAudioCodec = getString(R.string.pref_audiocodec_key);
    keyprefNoAudioProcessingPipeline = getString(R.string.pref_noaudioprocessing_key);
    keyprefAecDump = getString(R.string.pref_aecdump_key);
    keyprefOpenSLES = getString(R.string.pref_opensles_key);
    keyprefDisableBuiltInAec = getString(R.string.pref_disable_built_in_aec_key);
    keyprefDisableBuiltInAgc = getString(R.string.pref_disable_built_in_agc_key);
    keyprefDisableBuiltInNs = getString(R.string.pref_disable_built_in_ns_key);
    keyprefEnableLevelControl = getString(R.string.pref_enable_level_control_key);
    keyprefDisplayHud = getString(R.string.pref_displayhud_key);
    keyprefTracing = getString(R.string.pref_tracing_key);
    keyprefRoomServerUrl = getString(R.string.pref_room_server_url_key);
    keyprefRoom = getString(R.string.pref_room_key);
    keyprefRoomList = getString(R.string.pref_room_list_key);
    keyprefEnableDataChannel = getString(R.string.pref_enable_datachannel_key);
    keyprefOrdered = getString(R.string.pref_ordered_key);
    keyprefMaxRetransmitTimeMs = getString(R.string.pref_max_retransmit_time_ms_key);
    keyprefMaxRetransmits = getString(R.string.pref_max_retransmits_key);
    keyprefDataProtocol = getString(R.string.pref_data_protocol_key);
    keyprefNegotiated = getString(R.string.pref_negotiated_key);
    keyprefDataId = getString(R.string.pref_data_id_key);
    keyprefLogcat = getString(R.string.pref_logcat_key);

    setContentView(R.layout.activity_connect);

    roomEditText = (EditText) findViewById(R.id.room_edittext);
    roomEditText.setOnEditorActionListener(new TextView.OnEditorActionListener() {
      @Override
      public boolean onEditorAction(TextView textView, int i, KeyEvent keyEvent) {
        if (i == EditorInfo.IME_ACTION_DONE) {
          addFavoriteButton.performClick();
          return true;
        }
        return false;
      }
    });
    roomEditText.requestFocus();

    roomListView = (ListView) findViewById(R.id.room_listview);
    roomListView.setEmptyView(findViewById(android.R.id.empty));
    roomListView.setOnItemClickListener(roomListClickListener);
    registerForContextMenu(roomListView);
    createButton = findViewById(R.id.create_button);
    createButton.setOnClickListener(createListener);
    joinButton = findViewById(R.id.join_button);
    joinButton.setOnClickListener(joinListener);
    addFavoriteButton = (ImageButton) findViewById(R.id.add_favorite_button);
    addFavoriteButton.setOnClickListener(addFavoriteListener);
    // If an implicit VIEW intent is launching the app, go directly to that URL.
    final Intent intent = getIntent();
    if ("android.intent.action.VIEW".equals(intent.getAction()) && !commandLineRun) {
      boolean loopback = intent.getBooleanExtra(CallActivity.EXTRA_LOOPBACK, false);
      int runTimeMs = intent.getIntExtra(CallActivity.EXTRA_RUNTIME, 0);
      boolean useValuesFromIntent =
          intent.getBooleanExtra(CallActivity.EXTRA_USE_VALUES_FROM_INTENT, false);
      String room = sharedPref.getString(keyprefRoom, "");
      connectToRoom(room, isCreate, true, loopback, useValuesFromIntent, runTimeMs);
    }

    logPath = getLogPath(getApplicationContext());
    final boolean usingLogCat = sharedPref.getBoolean(keyprefLogcat, true);
    if (usingLogCat) {
      LogcatFileManager.getInstance().start(logPath);
    }
    uploadButton = findViewById(R.id.upload_button);
    uploadButton.setOnClickListener(new View.OnClickListener() {
      @Override
      public void onClick(View view) {
        executor.execute(new Runnable() {
          @Override
          public void run() {
            reportLog();
          }
        });
      }
    });

    clearButton = findViewById(R.id.clear_button);
    clearButton.setOnClickListener(new View.OnClickListener() {
      @Override
      public void onClick(View view) {
        LogcatFileManager.getInstance().stop();
        File dir = new File(logPath);
        if (dir.exists() && dir.isDirectory()) {
          for (File file : dir.listFiles()) {
            String fileName = file.getName();
            if (!fileName.startsWith("bee")) {
              file.delete();
            }
          }
        }
        if (usingLogCat) {
          LogcatFileManager.getInstance().start(logPath);
        }
        runOnUiThread(new Runnable() {
          @Override
          public void run() {
            logAndToast("日志已清除");
          }
        });
      }
    });

    settingButton = findViewById(R.id.setting_button);
    final ConnectActivity thisClassObj = this;
    settingButton.setOnClickListener(new View.OnClickListener() {
      @Override
      public void onClick(View view) {
        Intent intent = new Intent(thisClassObj, SettingsActivity.class);
        startActivity(intent);
      }
    });

    startCheckDmpLogTimer();
    //startNativeMemoryMonitorTimer();

    boolean enableStatusd = sharedPrefGetBoolean(R.string.pref_enable_statusd_key,
            CallActivity.EXTRA_ENABLE_STATUSD, R.string.pref_enable_statusd_default, false);
    String deviceId = sharedPrefGetString(R.string.pref_device_id_key,
            CallActivity.EXTRA_DEVICE_ID, R.string.pref_device_id_default, false);

    // Check HW encoder flag.
    boolean hwEncoder = sharedPrefGetBoolean(R.string.pref_hwencoder_key,
            CallActivity.EXTRA_HWENCODER_ENABLED, R.string.pref_hwencoder_default, false);

    // Check HW decoder flag.
    boolean hwDecoder = sharedPrefGetBoolean(R.string.pref_hwdecoder_key,
            CallActivity.EXTRA_HWDECODER_ENABLED, R.string.pref_hwdecoder_default, false);

    initBeeParam(logPath, enableStatusd, deviceId, hwEncoder, hwDecoder, bee_param_);
    //请求权限
    //Dynamically request permissions first if version >= 23.
    if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) {
      requestPermissions();
    } else {
      BeeSDK.sharedInstance().init(getApplicationContext(), bee_param_, 10000, this, new BeeAsyncHandler(){
        @Override
        public void InitHandler(BeeSystemDefine.BeeErrorCode code) {
          if (code != BeeSystemDefine.BeeErrorCode.kBeeErrorCode_Success) {
            logAndToast("Bee initialize fail.");
          }
        }
      });
    }
  }

  @Override
  public boolean onCreateOptionsMenu(Menu menu) {
    getMenuInflater().inflate(R.menu.connect_menu, menu);
    return true;
  }

  @Override
  public void onCreateContextMenu(ContextMenu menu, View v, ContextMenu.ContextMenuInfo menuInfo) {
    if (v.getId() == R.id.room_listview) {
      AdapterView.AdapterContextMenuInfo info = (AdapterView.AdapterContextMenuInfo) menuInfo;
      menu.setHeaderTitle(roomList.get(info.position));
      String[] menuItems = getResources().getStringArray(R.array.roomListContextMenu);
      for (int i = 0; i < menuItems.length; i++) {
        menu.add(Menu.NONE, i, i, menuItems[i]);
      }
    } else {
      super.onCreateContextMenu(menu, v, menuInfo);
    }
  }

  @Override
  public boolean onContextItemSelected(MenuItem item) {
    if (item.getItemId() == REMOVE_FAVORITE_INDEX) {
      AdapterView.AdapterContextMenuInfo info =
          (AdapterView.AdapterContextMenuInfo) item.getMenuInfo();
      roomList.remove(info.position);
      adapter.notifyDataSetChanged();
      return true;
    }

    return super.onContextItemSelected(item);
  }

  @Override
  public boolean onOptionsItemSelected(MenuItem item) {
    // Handle presses on the action bar items.
    if (item.getItemId() == R.id.action_settings) {
      Intent intent = new Intent(this, SettingsActivity.class);
      startActivity(intent);
      return true;
    } else if (item.getItemId() == R.id.action_loopback) {
      connectToRoom(null, false, false, true, false, 0);
      return true;
    } else {
      return super.onOptionsItemSelected(item);
    }
  }

  @Override
  public void onPause() {
    super.onPause();
    String room = roomEditText.getText().toString();
    String roomListJson = new JSONArray(roomList).toString();
    SharedPreferences.Editor editor = sharedPref.edit();
    editor.putString(keyprefRoom, room);
    editor.putString(keyprefRoomList, roomListJson);
    editor.commit();
  }

  @Override
  public void onResume() {
    super.onResume();
    String room = sharedPref.getString(keyprefRoom, "");
    roomEditText.setText(room);
    roomList = new ArrayList<String>();
    String roomListJson = sharedPref.getString(keyprefRoomList, null);
    if (roomListJson != null) {
      try {
        JSONArray jsonArray = new JSONArray(roomListJson);
        for (int i = 0; i < jsonArray.length(); i++) {
          roomList.add(jsonArray.get(i).toString());
        }
      } catch (JSONException e) {
        Log.e(TAG, "Failed to load room list: " + e.toString());
      }
    }
    adapter = new ArrayAdapter<String>(this, android.R.layout.simple_list_item_1, roomList);
    roomListView.setAdapter(adapter);
    if (adapter.getCount() > 0) {
      roomListView.requestFocus();
      roomListView.setItemChecked(0, true);
    }
  }

  @Override
  protected void onActivityResult(int requestCode, int resultCode, Intent data) {
    if (requestCode == CONNECTION_REQUEST && commandLineRun) {
      Log.d(TAG, "Return: " + resultCode);
      setResult(resultCode);
      commandLineRun = false;
      finish();
    }
  }

  @Override
  protected void onDestroy() {
    BeeSDK.sharedInstance().unit(null);
    super.onDestroy();
  }

  private void initBeeParam(String logPath, boolean enableStatusd, String deviceId, boolean hwEncode, boolean hwDecode, BeeSystemParam param) {
    param.platform_type     = BeeSystemDefine.BeePlatformType.kPlatformType_PC.ordinal();
    param.app_name          = "VideoRoomTest";
    param.app_version       = "1.0";
    param.system_info       = "Android4.4";
    param.machine_code      = deviceId;
    param.log_path          = logPath;
    param.log_level         = BeeSystemDefine.BeeLogLevel.kLogLevel_Debug.ordinal();
    param.log_max_line      = 0;
    param.log_volume_count  = 5;
    param.log_volume_size   = 1 * 1024;
    param.session_count     = 1;
    param.enable_statusd    = enableStatusd;
    param.enable_video_encoder_hw = hwEncode;
    param.enable_video_decoder_hw = hwDecode;
  }

  /**
   * Get a value from the shared preference or from the intent, if it does not
   * exist the default is used.
   */
  private String sharedPrefGetString(
      int attributeId, String intentName, int defaultId, boolean useFromIntent) {
    String defaultValue = getString(defaultId);
    if (useFromIntent) {
      String value = getIntent().getStringExtra(intentName);
      if (value != null) {
        return value;
      }
      return defaultValue;
    } else {
      String attributeName = getString(attributeId);
      return sharedPref.getString(attributeName, defaultValue);
    }
  }

  /**
   * Get a value from the shared preference or from the intent, if it does not
   * exist the default is used.
   */
  private boolean sharedPrefGetBoolean(
      int attributeId, String intentName, int defaultId, boolean useFromIntent) {
    boolean defaultValue = Boolean.valueOf(getString(defaultId));
    if (useFromIntent) {
      return getIntent().getBooleanExtra(intentName, defaultValue);
    } else {
      String attributeName = getString(attributeId);
      return sharedPref.getBoolean(attributeName, defaultValue);
    }
  }

  /**
   * Get a value from the shared preference or from the intent, if it does not
   * exist the default is used.
   */
  private int sharedPrefGetInteger(
      int attributeId, String intentName, int defaultId, boolean useFromIntent) {
    String defaultString = getString(defaultId);
    int defaultValue = Integer.parseInt(defaultString);
    if (useFromIntent) {
      return getIntent().getIntExtra(intentName, defaultValue);
    } else {
      String attributeName = getString(attributeId);
      String value = sharedPref.getString(attributeName, defaultString);
      try {
        return Integer.parseInt(value);
      } catch (NumberFormatException e) {
        Log.e(TAG, "Wrong setting for: " + attributeName + ":" + value);
        return defaultValue;
      }
    }
  }

  private String getLogPath(Context context) {
    File fileDir = null;
    if (Environment.MEDIA_MOUNTED.equals(Environment.getExternalStorageState()) || !Environment.isExternalStorageRemovable()) {
      fileDir = context.getExternalFilesDir(null); //In /storage/emulated/0/Android/data/com.waka.workspace.logtofile/files/Logs/
    }
    if (fileDir == null) {
      fileDir = context.getFilesDir(); //In /data/data
    }
    if (fileDir != null) {
      return fileDir.getPath();
    } else {
      Logging.e(TAG, "Cannot get log path");
      return "";
    }
  }

  private void connectToRoom(String roomId, boolean create, boolean commandLineRun, boolean loopback,
      boolean useValuesFromIntent, int runTimeMs) {
    this.commandLineRun = commandLineRun;

    // roomId is random for loopback.
    if (loopback) {
      roomId = Integer.toString((new Random()).nextInt(100000000));
    }

    String roomUrl = sharedPref.getString(
        keyprefRoomServerUrl, getString(R.string.pref_room_server_url_default));

    // Video call enabled flag.
    boolean videoCallEnabled = sharedPrefGetBoolean(R.string.pref_videocall_key,
        CallActivity.EXTRA_VIDEO_CALL, R.string.pref_videocall_default, useValuesFromIntent);

    // Use screencapture option.
    boolean useScreencapture = sharedPrefGetBoolean(R.string.pref_screencapture_key,
        CallActivity.EXTRA_SCREENCAPTURE, R.string.pref_screencapture_default, useValuesFromIntent);

    // Use Camera2 option.
    boolean useCamera2 = sharedPrefGetBoolean(R.string.pref_camera2_key, CallActivity.EXTRA_CAMERA2,
        R.string.pref_camera2_default, useValuesFromIntent);

    // Get default codecs.
    String videoCodec = sharedPrefGetString(R.string.pref_videocodec_key,
        CallActivity.EXTRA_VIDEOCODEC, R.string.pref_videocodec_default, useValuesFromIntent);
    String audioCodec = sharedPrefGetString(R.string.pref_audiocodec_key,
        CallActivity.EXTRA_AUDIOCODEC, R.string.pref_audiocodec_default, useValuesFromIntent);

    // Check HW codec flag.
    boolean hwCodec = sharedPrefGetBoolean(R.string.pref_hwcodec_key,
        CallActivity.EXTRA_HWCODEC_ENABLED, R.string.pref_hwcodec_default, useValuesFromIntent);

    // Check Capture to texture.
    boolean captureToTexture = sharedPrefGetBoolean(R.string.pref_capturetotexture_key,
        CallActivity.EXTRA_CAPTURETOTEXTURE_ENABLED, R.string.pref_capturetotexture_default,
        useValuesFromIntent);

    // Check FlexFEC.
    boolean flexfecEnabled = sharedPrefGetBoolean(R.string.pref_flexfec_key,
        CallActivity.EXTRA_FLEXFEC_ENABLED, R.string.pref_flexfec_default, useValuesFromIntent);

    // Check Disable Audio Processing flag.
    boolean noAudioProcessing = sharedPrefGetBoolean(R.string.pref_noaudioprocessing_key,
        CallActivity.EXTRA_NOAUDIOPROCESSING_ENABLED, R.string.pref_noaudioprocessing_default,
        useValuesFromIntent);

    // Check Disable Audio Processing flag.
    boolean aecDump = sharedPrefGetBoolean(R.string.pref_aecdump_key,
        CallActivity.EXTRA_AECDUMP_ENABLED, R.string.pref_aecdump_default, useValuesFromIntent);

    // Check OpenSL ES enabled flag.
    boolean useOpenSLES = sharedPrefGetBoolean(R.string.pref_opensles_key,
        CallActivity.EXTRA_OPENSLES_ENABLED, R.string.pref_opensles_default, useValuesFromIntent);

    // Check Disable built-in AEC flag.
    boolean disableBuiltInAEC = sharedPrefGetBoolean(R.string.pref_disable_built_in_aec_key,
        CallActivity.EXTRA_DISABLE_BUILT_IN_AEC, R.string.pref_disable_built_in_aec_default,
        useValuesFromIntent);

    // Check Disable built-in AGC flag.
    boolean disableBuiltInAGC = sharedPrefGetBoolean(R.string.pref_disable_built_in_agc_key,
        CallActivity.EXTRA_DISABLE_BUILT_IN_AGC, R.string.pref_disable_built_in_agc_default,
        useValuesFromIntent);

    // Check Disable built-in NS flag.
    boolean disableBuiltInNS = sharedPrefGetBoolean(R.string.pref_disable_built_in_ns_key,
        CallActivity.EXTRA_DISABLE_BUILT_IN_NS, R.string.pref_disable_built_in_ns_default,
        useValuesFromIntent);

    // Check Enable level control.
    boolean enableLevelControl = sharedPrefGetBoolean(R.string.pref_enable_level_control_key,
        CallActivity.EXTRA_ENABLE_LEVEL_CONTROL, R.string.pref_enable_level_control_key,
        useValuesFromIntent);

    // Get video resolution from settings.
    int videoWidth = 0;
    int videoHeight = 0;
    if (useValuesFromIntent) {
      videoWidth = getIntent().getIntExtra(CallActivity.EXTRA_VIDEO_WIDTH, 0);
      videoHeight = getIntent().getIntExtra(CallActivity.EXTRA_VIDEO_HEIGHT, 0);
    }
    if (videoWidth == 0 && videoHeight == 0) {
      String resolution =
          sharedPref.getString(keyprefResolution, getString(R.string.pref_resolution_default));
      String[] dimensions = resolution.split("[ x]+");
      if (dimensions.length == 2) {
        try {
          videoWidth = Integer.parseInt(dimensions[0]);
          videoHeight = Integer.parseInt(dimensions[1]);
        } catch (NumberFormatException e) {
          videoWidth = 0;
          videoHeight = 0;
          Log.e(TAG, "Wrong video resolution setting: " + resolution);
        }
      }
    }

    // Get camera fps from settings.
    int cameraFps = 0;
    if (useValuesFromIntent) {
      cameraFps = getIntent().getIntExtra(CallActivity.EXTRA_VIDEO_FPS, 0);
    }
    if (cameraFps == 0) {
      String fps = sharedPref.getString(keyprefFps, getString(R.string.pref_fps_default));
      String[] fpsValues = fps.split("[ x]+");
      if (fpsValues.length == 2) {
        try {
          cameraFps = Integer.parseInt(fpsValues[0]);
        } catch (NumberFormatException e) {
          cameraFps = 0;
          Log.e(TAG, "Wrong camera fps setting: " + fps);
        }
      }
    }

    // Check capture quality slider flag.
    boolean captureQualitySlider = sharedPrefGetBoolean(R.string.pref_capturequalityslider_key,
        CallActivity.EXTRA_VIDEO_CAPTUREQUALITYSLIDER_ENABLED,
        R.string.pref_capturequalityslider_default, useValuesFromIntent);

    // Get video and audio start bitrate.
    int videoStartBitrate = 0;
    if (useValuesFromIntent) {
      videoStartBitrate = getIntent().getIntExtra(CallActivity.EXTRA_VIDEO_BITRATE, 0);
    }
    if (videoStartBitrate == 0) {
      String bitrateTypeDefault = getString(R.string.pref_maxvideobitrate_default);
      String bitrateType = sharedPref.getString(keyprefVideoBitrateType, bitrateTypeDefault);
      if (!bitrateType.equals(bitrateTypeDefault)) {
        String bitrateValue = sharedPref.getString(
            keyprefVideoBitrateValue, getString(R.string.pref_maxvideobitratevalue_default));
        videoStartBitrate = Integer.parseInt(bitrateValue);
      }
    }

    int audioStartBitrate = 0;
    if (useValuesFromIntent) {
      audioStartBitrate = getIntent().getIntExtra(CallActivity.EXTRA_AUDIO_BITRATE, 0);
    }
    if (audioStartBitrate == 0) {
      String bitrateTypeDefault = getString(R.string.pref_startaudiobitrate_default);
      String bitrateType = sharedPref.getString(keyprefAudioBitrateType, bitrateTypeDefault);
      if (!bitrateType.equals(bitrateTypeDefault)) {
        String bitrateValue = sharedPref.getString(
            keyprefAudioBitrateValue, getString(R.string.pref_startaudiobitratevalue_default));
        audioStartBitrate = Integer.parseInt(bitrateValue);
      }
    }

    // Check statistics display option.
    boolean displayHud = sharedPrefGetBoolean(R.string.pref_displayhud_key,
        CallActivity.EXTRA_DISPLAY_HUD, R.string.pref_displayhud_default, useValuesFromIntent);

    boolean tracing = sharedPrefGetBoolean(R.string.pref_tracing_key, CallActivity.EXTRA_TRACING,
        R.string.pref_tracing_default, useValuesFromIntent);

    // Get datachannel options
    boolean dataChannelEnabled = sharedPrefGetBoolean(R.string.pref_enable_datachannel_key,
        CallActivity.EXTRA_DATA_CHANNEL_ENABLED, R.string.pref_enable_datachannel_default,
        useValuesFromIntent);
    boolean ordered = sharedPrefGetBoolean(R.string.pref_ordered_key, CallActivity.EXTRA_ORDERED,
        R.string.pref_ordered_default, useValuesFromIntent);
    boolean negotiated = sharedPrefGetBoolean(R.string.pref_negotiated_key,
        CallActivity.EXTRA_NEGOTIATED, R.string.pref_negotiated_default, useValuesFromIntent);
    int maxRetrMs = sharedPrefGetInteger(R.string.pref_max_retransmit_time_ms_key,
        CallActivity.EXTRA_MAX_RETRANSMITS_MS, R.string.pref_max_retransmit_time_ms_default,
        useValuesFromIntent);
    int maxRetr =
        sharedPrefGetInteger(R.string.pref_max_retransmits_key, CallActivity.EXTRA_MAX_RETRANSMITS,
            R.string.pref_max_retransmits_default, useValuesFromIntent);
    int id = sharedPrefGetInteger(R.string.pref_data_id_key, CallActivity.EXTRA_ID,
        R.string.pref_data_id_default, useValuesFromIntent);
    String protocol = sharedPrefGetString(R.string.pref_data_protocol_key,
        CallActivity.EXTRA_PROTOCOL, R.string.pref_data_protocol_default, useValuesFromIntent);

    // Start AppRTCMobile activity.
    Log.d(TAG, "Connecting to room " + roomId);
    if (validateUrl(roomUrl)) {
      Uri uri = Uri.parse(roomUrl);
      Intent intent = new Intent(this, CallActivity.class);
      intent.setData(uri);
      intent.putExtra(CallActivity.EXTRA_ROOMID, roomId);
      intent.putExtra(CallActivity.EXTRA_LOOPBACK, loopback);
      intent.putExtra(CallActivity.EXTRA_VIDEO_CALL, videoCallEnabled);
      intent.putExtra(CallActivity.EXTRA_SCREENCAPTURE, useScreencapture);
      intent.putExtra(CallActivity.EXTRA_CAMERA2, useCamera2);
      intent.putExtra(CallActivity.EXTRA_VIDEO_WIDTH, videoWidth);
      intent.putExtra(CallActivity.EXTRA_VIDEO_HEIGHT, videoHeight);
      intent.putExtra(CallActivity.EXTRA_VIDEO_FPS, cameraFps);
      intent.putExtra(CallActivity.EXTRA_VIDEO_CAPTUREQUALITYSLIDER_ENABLED, captureQualitySlider);
      intent.putExtra(CallActivity.EXTRA_VIDEO_BITRATE, videoStartBitrate);
      intent.putExtra(CallActivity.EXTRA_VIDEOCODEC, videoCodec);
      intent.putExtra(CallActivity.EXTRA_HWCODEC_ENABLED, hwCodec);
      intent.putExtra(CallActivity.EXTRA_CAPTURETOTEXTURE_ENABLED, captureToTexture);
      intent.putExtra(CallActivity.EXTRA_FLEXFEC_ENABLED, flexfecEnabled);
      intent.putExtra(CallActivity.EXTRA_NOAUDIOPROCESSING_ENABLED, noAudioProcessing);
      intent.putExtra(CallActivity.EXTRA_AECDUMP_ENABLED, aecDump);
      intent.putExtra(CallActivity.EXTRA_OPENSLES_ENABLED, useOpenSLES);
      intent.putExtra(CallActivity.EXTRA_DISABLE_BUILT_IN_AEC, disableBuiltInAEC);
      intent.putExtra(CallActivity.EXTRA_DISABLE_BUILT_IN_AGC, disableBuiltInAGC);
      intent.putExtra(CallActivity.EXTRA_DISABLE_BUILT_IN_NS, disableBuiltInNS);
      intent.putExtra(CallActivity.EXTRA_ENABLE_LEVEL_CONTROL, enableLevelControl);
      intent.putExtra(CallActivity.EXTRA_AUDIO_BITRATE, audioStartBitrate);
      intent.putExtra(CallActivity.EXTRA_AUDIOCODEC, audioCodec);
      intent.putExtra(CallActivity.EXTRA_DISPLAY_HUD, displayHud);
      intent.putExtra(CallActivity.EXTRA_TRACING, tracing);
      intent.putExtra(CallActivity.EXTRA_CMDLINE, commandLineRun);
      intent.putExtra(CallActivity.EXTRA_RUNTIME, runTimeMs);

      intent.putExtra(CallActivity.EXTRA_DATA_CHANNEL_ENABLED, dataChannelEnabled);

      if (dataChannelEnabled) {
        intent.putExtra(CallActivity.EXTRA_ORDERED, ordered);
        intent.putExtra(CallActivity.EXTRA_MAX_RETRANSMITS_MS, maxRetrMs);
        intent.putExtra(CallActivity.EXTRA_MAX_RETRANSMITS, maxRetr);
        intent.putExtra(CallActivity.EXTRA_PROTOCOL, protocol);
        intent.putExtra(CallActivity.EXTRA_NEGOTIATED, negotiated);
        intent.putExtra(CallActivity.EXTRA_ID, id);
      }

      if (useValuesFromIntent) {
        if (getIntent().hasExtra(CallActivity.EXTRA_VIDEO_FILE_AS_CAMERA)) {
          String videoFileAsCamera =
              getIntent().getStringExtra(CallActivity.EXTRA_VIDEO_FILE_AS_CAMERA);
          intent.putExtra(CallActivity.EXTRA_VIDEO_FILE_AS_CAMERA, videoFileAsCamera);
        }

        if (getIntent().hasExtra(CallActivity.EXTRA_SAVE_REMOTE_VIDEO_TO_FILE)) {
          String saveRemoteVideoToFile =
              getIntent().getStringExtra(CallActivity.EXTRA_SAVE_REMOTE_VIDEO_TO_FILE);
          intent.putExtra(CallActivity.EXTRA_SAVE_REMOTE_VIDEO_TO_FILE, saveRemoteVideoToFile);
        }

        if (getIntent().hasExtra(CallActivity.EXTRA_SAVE_REMOTE_VIDEO_TO_FILE_WIDTH)) {
          int videoOutWidth =
              getIntent().getIntExtra(CallActivity.EXTRA_SAVE_REMOTE_VIDEO_TO_FILE_WIDTH, 0);
          intent.putExtra(CallActivity.EXTRA_SAVE_REMOTE_VIDEO_TO_FILE_WIDTH, videoOutWidth);
        }

        if (getIntent().hasExtra(CallActivity.EXTRA_SAVE_REMOTE_VIDEO_TO_FILE_HEIGHT)) {
          int videoOutHeight =
              getIntent().getIntExtra(CallActivity.EXTRA_SAVE_REMOTE_VIDEO_TO_FILE_HEIGHT, 0);
          intent.putExtra(CallActivity.EXTRA_SAVE_REMOTE_VIDEO_TO_FILE_HEIGHT, videoOutHeight);
        }
      }

      intent.putExtra(CallActivity.EXTRA_CREATE_ROOM, create);
      startActivityForResult(intent, CONNECTION_REQUEST);
    }
  }

  private boolean validateUrl(String url) {
    return true;
  }

  private final AdapterView.OnItemClickListener roomListClickListener =
      new AdapterView.OnItemClickListener() {
        @Override
        public void onItemClick(AdapterView<?> adapterView, View view, int i, long l) {
          String roomId = ((TextView) view).getText().toString();
          connectToRoom(roomId, false, false, false, false, 0);
        }
      };

  private final OnClickListener addFavoriteListener = new OnClickListener() {
    @Override
    public void onClick(View view) {
      String newRoom = roomEditText.getText().toString();
      if (newRoom.length() > 0 && !roomList.contains(newRoom)) {
        adapter.add(newRoom);
        adapter.notifyDataSetChanged();
      }
    }
  };

  private final OnClickListener createListener = new OnClickListener() {
    @Override
    public void onClick(View view) {
      connectToRoom(roomEditText.getText().toString(), true, false, false, false, 0);
    }
  };

  private final OnClickListener joinListener = new OnClickListener() {
    @Override
    public void onClick(View view) {
      connectToRoom(roomEditText.getText().toString(), false, false, false, false, 0);
    }
  };

  @Override
  public void onLog(String log) {

  }

  @Override
  public void onNotify(BeeSystemDefine.BeeErrorCode ec1, int ec2) {

  }

  private void requestPermissions() {
    String needGrantPermissions[] = {
            Manifest.permission.CAMERA,                 //Push video
            Manifest.permission.RECORD_AUDIO,           //Push audio
            Manifest.permission.WRITE_EXTERNAL_STORAGE, //Write log
            Manifest.permission.READ_EXTERNAL_STORAGE   //Upload log
    };

    List<String> notGrantedPermissionsList = new ArrayList<>();
    for (String permission : needGrantPermissions) {
      if (ContextCompat.checkSelfPermission(this, permission) != PackageManager.PERMISSION_GRANTED) {
        if (ActivityCompat.shouldShowRequestPermissionRationale(this, permission)) {
          logAndToast("Need permission " + permission);
        }
        notGrantedPermissionsList.add(permission);
      }
    }

    if (!notGrantedPermissionsList.isEmpty()) {
      String[] notGrantedPermissionsArray = notGrantedPermissionsList.toArray(new String[notGrantedPermissionsList.size()]);
      ActivityCompat.requestPermissions(this, notGrantedPermissionsArray, 10000);
    } else {
      BeeSDK.sharedInstance().init(getApplicationContext(), bee_param_, 10000, this, new BeeAsyncHandler(){
        @Override
        public void InitHandler(BeeSystemDefine.BeeErrorCode code) {
          if (code != BeeSystemDefine.BeeErrorCode.kBeeErrorCode_Success) {
            logAndToast("Bee initialize fail.");
          }
        }
      });
    }
  }

  @Override
  public void onRequestPermissionsResult(int requestCode, String[] permissions, int[] grantResults) {
    super.onRequestPermissionsResult(requestCode, permissions, grantResults);
    if (requestCode == 10000) {
      for (int i = 0; i < permissions.length; i++) {
        if (grantResults[i] == PackageManager.PERMISSION_GRANTED) {
          //logAndToast("Permission " + permissions[i] + "granted.");
        } else {
          //logAndToast( "Permission " + permissions[i] + "not granted.");
          setResult(RESULT_CANCELED);
          finish();
          return;
        }
      }

      BeeSDK.sharedInstance().init(getApplicationContext(), bee_param_, 10000, this, new BeeAsyncHandler(){
        @Override
        public void InitHandler(BeeSystemDefine.BeeErrorCode code) {
          if (code != BeeSystemDefine.BeeErrorCode.kBeeErrorCode_Success) {
            logAndToast("Bee initialize fail.");
          }
        }
      });
    }
  }

}
