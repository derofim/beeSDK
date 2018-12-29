package com.sohu.tv.audiochat;

import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.os.Build;
import android.os.Bundle;
import android.os.Debug;
import android.os.Environment;
import android.preference.PreferenceManager;
import android.util.Log;
import android.view.ContextMenu;
import android.view.KeyEvent;
import android.view.Menu;
import android.view.MenuItem;
import android.view.MotionEvent;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.inputmethod.EditorInfo;
import android.view.inputmethod.InputMethodManager;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.EditText;
import android.widget.ImageButton;
import android.widget.ListView;
import android.widget.TextView;
import android.widget.Toast;

import org.json.JSONArray;
import org.json.JSONException;
import org.webrtc.Logging;

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

/**
 * Handles the initial setup where the user selects which room to join.
 */
public class ConnectActivity extends Activity {
  private static final String TAG = "ConnectActivity";
  private static final int CONNECTION_REQUEST = 1;
  private static final int REMOVE_FAVORITE_INDEX = 0;
  private static final int CHECK_DMP_TIMER_PERIOD = 5000;
  private static final int NATIVE_MEMORY_MONITOR_PERIOD = 1000;
  private static final boolean isCreate = true;

  private Toast logToast;
  private final ScheduledExecutorService executor = Executors.newSingleThreadScheduledExecutor();
  private ImageButton addFavoriteButton;
  private EditText roomEditText;
  private EditText nickNameEditText;
  private ListView roomListView;

  private SharedPreferences sharedPref;
  private String keyprefAudioBitrateType;
  private String keyprefAudioBitrateValue;
  private String keyprefAudioCodec;
  private String keyprefNoAudioProcessingPipeline;
  private String keyprefOpenSLES;
  private String keyprefDisableBuiltInAec;
  private String keyprefDisableBuiltInAgc;
  private String keyprefDisableBuiltInNs;
  private String keyprefEnableLevelControl;
  private String keyprefDisplayHud;
  private String keyprefRoom;
  private String keyprefNickName;
  private String keyprefRoomList;

  private ArrayList<String> roomList;
  private ArrayAdapter<String> adapter;

  private String logPath;
  private String keyprefLogcat;
  private Timer logTimer = new Timer();
  private Timer nativeMemoryMonitorTimer = new Timer();

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
      String version = sharedPrefGetString(R.string.pref_version_key, null, R.string.pref_version_default);
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
    keyprefAudioBitrateType = getString(R.string.pref_startaudiobitrate_key);
    keyprefAudioBitrateValue = getString(R.string.pref_startaudiobitratevalue_key);
    keyprefAudioCodec = getString(R.string.pref_audiocodec_key);
    keyprefNoAudioProcessingPipeline = getString(R.string.pref_noaudioprocessing_key);
    keyprefOpenSLES = getString(R.string.pref_opensles_key);
    keyprefDisableBuiltInAec = getString(R.string.pref_disable_built_in_aec_key);
    keyprefDisableBuiltInAgc = getString(R.string.pref_disable_built_in_agc_key);
    keyprefDisableBuiltInNs = getString(R.string.pref_disable_built_in_ns_key);
    keyprefEnableLevelControl = getString(R.string.pref_enable_level_control_key);
    keyprefDisplayHud = getString(R.string.pref_displayhud_key);
    keyprefRoomList = getString(R.string.pref_room_list_key);
    keyprefRoom = getString(R.string.pref_room_key);
    keyprefNickName = getString(R.string.pref_nick_name_key);
    keyprefLogcat = getString(R.string.pref_logcat_key);

    setContentView(R.layout.activity_connect);

    roomEditText = findViewById(R.id.room_edittext);
    roomEditText.requestFocus();

    nickNameEditText = findViewById(R.id.nick_name_edittext);

    roomListView = findViewById(R.id.room_listview);
    roomListView.setEmptyView(findViewById(android.R.id.empty));
    roomListView.setOnItemClickListener(roomListClickListener);
    registerForContextMenu(roomListView);

    Button createButton = findViewById(R.id.create_button);
    createButton.setOnClickListener(createListener);

    Button joinButton = findViewById(R.id.join_button);
    joinButton.setOnClickListener(joinListener);

    addFavoriteButton = findViewById(R.id.add_favorite_button);
    addFavoriteButton.setOnClickListener(addFavoriteListener);

    logPath = getLogPath(getApplicationContext());
    final boolean usingLogCat = sharedPref.getBoolean(keyprefLogcat, true);
    if (usingLogCat) {
      LogcatFileManager.getInstance().start(logPath);
    }

    ImageButton uploadButton = findViewById(R.id.upload_button);
    uploadButton.setOnClickListener(new OnClickListener() {
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

    ImageButton clearButton = findViewById(R.id.clear_button);
    clearButton.setOnClickListener(new OnClickListener() {
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

    ImageButton settingButton = findViewById(R.id.setting_button);
    final ConnectActivity thisClassObj = this;
    settingButton.setOnClickListener(new OnClickListener() {
      @Override
      public void onClick(View view) {
        Intent intent = new Intent(thisClassObj, SettingsActivity.class);
        startActivity(intent);
      }
    });

    startCheckDmpLogTimer();
    //startNativeMemoryMonitorTimer();
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
    }  else {
      return super.onOptionsItemSelected(item);
    }
  }

  @Override
  public void onPause() {
    super.onPause();
    String room = roomEditText.getText().toString();
    String nickName = nickNameEditText.getText().toString();
    String roomListJson = new JSONArray(roomList).toString();
    SharedPreferences.Editor editor = sharedPref.edit();
    editor.putString(keyprefRoom, room);
    editor.putString(keyprefNickName, nickName);
    editor.putString(keyprefRoomList, roomListJson);
    editor.commit();
  }

  @Override
  public void onResume() {
    super.onResume();
    String room = sharedPref.getString(keyprefRoom, "");
    String nickName = sharedPref.getString(keyprefNickName, "");
    roomEditText.setText(room);
    nickNameEditText.setText(nickName);
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

  }

  private String sharedPrefGetString(
      int attributeId, String intentName, int defaultId) {
    String defaultValue = getString(defaultId);
    String attributeName = getString(attributeId);
    return sharedPref.getString(attributeName, defaultValue);
  }

  /**
   * Get a value from the shared preference or from the intent, if it does not
   * exist the default is used.
   */
  private boolean sharedPrefGetBoolean(
      int attributeId, String intentName, int defaultId) {
    boolean defaultValue = Boolean.valueOf(getString(defaultId));
    String attributeName = getString(attributeId);
    return sharedPref.getBoolean(attributeName, defaultValue);
  }

  /**
   * Get a value from the shared preference or from the intent, if it does not
   * exist the default is used.
   */
  private int sharedPrefGetInteger(
      int attributeId, String intentName, int defaultId) {
    String defaultString = getString(defaultId);
    int defaultValue = Integer.parseInt(defaultString);
    String attributeName = getString(attributeId);
    String value = sharedPref.getString(attributeName, defaultString);
    try {
        return Integer.parseInt(value);
    } catch (NumberFormatException e) {
        Log.e(TAG, "Wrong setting for: " + attributeName + ":" + value);
        return defaultValue;
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

  private void connectToRoom(String roomId, String nickName, boolean create) {
    String audioCodec = sharedPrefGetString(R.string.pref_audiocodec_key,
        AudioChatRoomActivity.EXTRA_AUDIOCODEC, R.string.pref_audiocodec_default);

    // Check Disable Audio Processing flag.
    boolean noAudioProcessing = sharedPrefGetBoolean(R.string.pref_noaudioprocessing_key,
            AudioChatRoomActivity.EXTRA_NOAUDIOPROCESSING_ENABLED, R.string.pref_noaudioprocessing_default);

    // Check OpenSL ES enabled flag.
    boolean useOpenSLES = sharedPrefGetBoolean(R.string.pref_opensles_key,
            AudioChatRoomActivity.EXTRA_OPENSLES_ENABLED, R.string.pref_opensles_default);

    // Check Disable built-in AEC flag.
    boolean disableBuiltInAEC = sharedPrefGetBoolean(R.string.pref_disable_built_in_aec_key,
        AudioChatRoomActivity.EXTRA_DISABLE_BUILT_IN_AEC, R.string.pref_disable_built_in_aec_default);

    // Check Disable built-in AGC flag.
    boolean disableBuiltInAGC = sharedPrefGetBoolean(R.string.pref_disable_built_in_agc_key,
        AudioChatRoomActivity.EXTRA_DISABLE_BUILT_IN_AGC, R.string.pref_disable_built_in_agc_default);

    // Check Disable built-in NS flag.
    boolean disableBuiltInNS = sharedPrefGetBoolean(R.string.pref_disable_built_in_ns_key,
        AudioChatRoomActivity.EXTRA_DISABLE_BUILT_IN_NS, R.string.pref_disable_built_in_ns_default);

    // Check Enable level control.
    boolean enableLevelControl = sharedPrefGetBoolean(R.string.pref_enable_level_control_key,
        AudioChatRoomActivity.EXTRA_ENABLE_LEVEL_CONTROL, R.string.pref_enable_level_control_key);

    boolean showVolume = sharedPrefGetBoolean(R.string.pref_show_volume_key,
        AudioChatRoomActivity.EXTRA_SHOW_VOLUME, R.string.pref_show_volume_default);

    int audioStartBitrate = 0;

    String bitrateTypeDefault = getString(R.string.pref_startaudiobitrate_default);
    String bitrateType = sharedPref.getString(keyprefAudioBitrateType, bitrateTypeDefault);
    if (!bitrateType.equals(bitrateTypeDefault)) {
        String bitrateValue = sharedPref.getString(keyprefAudioBitrateValue, getString(R.string.pref_startaudiobitratevalue_default));
        audioStartBitrate = Integer.parseInt(bitrateValue);
    }

    // Check statistics display option.
    boolean displayHud = sharedPrefGetBoolean(R.string.pref_displayhud_key,
        AudioChatRoomActivity.EXTRA_DISPLAY_HUD, R.string.pref_displayhud_default);

    boolean enableStatusd = sharedPrefGetBoolean(R.string.pref_enable_statusd_key,
            AudioChatRoomActivity.EXTRA_ENABLE_STATUSD, R.string.pref_enable_statusd_default);

    String deviceId = sharedPrefGetString(R.string.pref_device_id_key,
            AudioChatRoomActivity.EXTRA_DEVICE_ID, R.string.pref_device_id_default);

    // Start AppRTCMobile activity.
    Log.d(TAG, "Connecting to room " + roomId);
      Intent intent = new Intent(this, AudioChatRoomActivity.class);
      intent.putExtra(AudioChatRoomActivity.EXTRA_ROOMID, roomId);
      intent.putExtra(AudioChatRoomActivity.EXTRA_NICKNAME, nickName);
      intent.putExtra(AudioChatRoomActivity.EXTRA_LOOPBACK, false);
      intent.putExtra(AudioChatRoomActivity.EXTRA_NOAUDIOPROCESSING_ENABLED, noAudioProcessing);
      intent.putExtra(AudioChatRoomActivity.EXTRA_OPENSLES_ENABLED, useOpenSLES);
      intent.putExtra(AudioChatRoomActivity.EXTRA_DISABLE_BUILT_IN_AEC, disableBuiltInAEC);
      intent.putExtra(AudioChatRoomActivity.EXTRA_DISABLE_BUILT_IN_AGC, disableBuiltInAGC);
      intent.putExtra(AudioChatRoomActivity.EXTRA_DISABLE_BUILT_IN_NS, disableBuiltInNS);
      intent.putExtra(AudioChatRoomActivity.EXTRA_ENABLE_LEVEL_CONTROL, enableLevelControl);
      intent.putExtra(AudioChatRoomActivity.EXTRA_AUDIO_BITRATE, audioStartBitrate);
      intent.putExtra(AudioChatRoomActivity.EXTRA_AUDIOCODEC, audioCodec);
      intent.putExtra(AudioChatRoomActivity.EXTRA_DISPLAY_HUD, displayHud);
      intent.putExtra(AudioChatRoomActivity.EXTRA_SHOW_VOLUME, showVolume);

      intent.putExtra(AudioChatRoomActivity.EXTRA_LOG_PATH, logPath);
      intent.putExtra(AudioChatRoomActivity.EXTRA_ENABLE_STATUSD, enableStatusd);
      intent.putExtra(AudioChatRoomActivity.EXTRA_DEVICE_ID, deviceId);
      intent.putExtra(AudioChatRoomActivity.EXTRA_CREATE_ROOM, create);

      startActivityForResult(intent, CONNECTION_REQUEST);
  }

  private boolean validateUrl(String url) {
    return true;
  }

  private final AdapterView.OnItemClickListener roomListClickListener =
      new AdapterView.OnItemClickListener() {
        @Override
        public void onItemClick(AdapterView<?> adapterView, View view, int i, long l) {
          String roomId = ((TextView) view).getText().toString();
          String nickName = nickNameEditText.getText().toString();
          connectToRoom(roomId, nickName,false);
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
      String roomId = roomEditText.getText().toString();
      String nickName = nickNameEditText.getText().toString();
      connectToRoom(roomId, nickName, true);
    }
  };

  private final OnClickListener joinListener = new OnClickListener() {
    @Override
    public void onClick(View view) {
      String roomId = roomEditText.getText().toString();
      String nickName = nickNameEditText.getText().toString();
      connectToRoom(roomId, nickName, false);
    }
  };

    public static Boolean hideInputMethod(Context context, View v) {
        InputMethodManager imm = (InputMethodManager) context
                .getSystemService(Context.INPUT_METHOD_SERVICE);
        if (imm != null) {
            return imm.hideSoftInputFromWindow(v.getWindowToken(), 0);
        }
        return false;
    }

    public static boolean isShouldHideInput(View v, MotionEvent event) {
        if (v != null && (v instanceof EditText)) {
            int[] leftTop = { 0, 0 };
            v.getLocationInWindow(leftTop);
            int left = leftTop[0], top = leftTop[1], bottom = top + v.getHeight(), right = left
                    + v.getWidth();
            if (event.getX() > left && event.getX() < right
                    && event.getY() > top && event.getY() < bottom) {
                return false;
            } else {
                return true;
            }
        }
        return false;
    }

    @Override
    public boolean dispatchTouchEvent(MotionEvent ev) {
        if (ev.getAction() == MotionEvent.ACTION_DOWN) {
            View v = getCurrentFocus();
            if (isShouldHideInput(v, ev)) {
                if(hideInputMethod(this, v)) {
                    v.clearFocus();
                    return true;
                }
            }
        }
        return super.dispatchTouchEvent(ev);
    }
}

