package com.sohu.tv.audiochat;

import android.Manifest;
import android.app.FragmentTransaction;
import android.content.Context;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.os.Build;
import android.os.Bundle;
import android.provider.Settings;
import android.support.design.widget.NavigationView;
import android.support.v4.app.ActivityCompat;
import android.support.v4.content.ContextCompat;
import android.support.v4.widget.DrawerLayout;
import android.support.v4.widget.ViewDragHelper;
import android.support.v7.app.ActionBarDrawerToggle;
import android.support.v7.app.AppCompatActivity;
import android.support.v7.widget.Toolbar;
import android.util.DisplayMetrics;
import android.util.Log;
import android.view.KeyEvent;
import android.view.MenuItem;
import android.view.View;
import android.view.Window;
import android.widget.Button;
import android.widget.ImageButton;
import android.widget.ListView;
import android.widget.RadioButton;
import android.widget.RadioGroup;
import android.widget.TextView;
import android.widget.Toast;

import com.sohu.tv.bee.AppRTCAudioManager;
import com.sohu.tv.bee.AppRTCAudioManager.AudioDevice;
import com.sohu.tv.bee.BeeAudioParam;
import com.sohu.tv.bee.BeeSystemDefine;
import com.sohu.tv.bee.BeeSystemDefine.BeeErrorCode;
import com.sohu.tv.bee.BeeSystemDefine.BeePartyType;
import com.sohu.tv.bee.BeeSystemDefine.MediaType;
import com.sohu.tv.bee.BeeSystemDefine.BoardType;
import com.sohu.tv.bee.BeeSystemParam;
import com.sohu.tv.bee.CpuMonitor;
import com.sohu.tv.bee.VideoRoom;
import com.sohu.tv.bee.VideoRoomSink;

import org.webrtc.StatsReport;
import org.webrtc.Logging;

import java.lang.reflect.Field;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Set;
import java.util.Timer;
import java.util.TimerTask;

public class AudioChatRoomActivity extends AppCompatActivity implements VideoRoomSink {
    public static final String EXTRA_ROOMID = "com.sohu.tv.beedmo.ROOMID";
    public static final String EXTRA_NICKNAME = "com.sohu.tv.beedmo.NICKNAME";
    public static final String EXTRA_LOOPBACK = "com.sohu.tv.beedmo.LOOPBACK";
    public static final String EXTRA_AUDIO_BITRATE = "com.sohu.tv.beedmo.AUDIO_BITRATE";
    public static final String EXTRA_AUDIOCODEC = "com.sohu.tv.beedmo.AUDIOCODEC";
    public static final String EXTRA_NOAUDIOPROCESSING_ENABLED = "com.sohu.tv.beedmo.NOAUDIOPROCESSING";
    public static final String EXTRA_OPENSLES_ENABLED = "com.sohu.tv.beedmo.OPENSLES";
    public static final String EXTRA_DISABLE_BUILT_IN_AEC = "com.sohu.tv.beedmo.DISABLE_BUILT_IN_AEC";
    public static final String EXTRA_DISABLE_BUILT_IN_AGC = "com.sohu.tv.beedmo.DISABLE_BUILT_IN_AGC";
    public static final String EXTRA_DISABLE_BUILT_IN_NS = "com.sohu.tv.beedmo.DISABLE_BUILT_IN_NS";
    public static final String EXTRA_ENABLE_LEVEL_CONTROL = "com.sohu.tv.beedmo.ENABLE_LEVEL_CONTROL";
    public static final String EXTRA_SHOW_VOLUME = "com.sohu.tv.beedmo.SHOW_VOLUME";
    public static final String EXTRA_DISPLAY_HUD = "com.sohu.tv.beedmo.DISPLAY_HUD";
    public static final String EXTRA_LOG_PATH = "com.sohu.tv.beedemo.LOG_PATH";
    public static final String EXTRA_ENABLE_STATUSD = "com.sohu.tv.beedemo.ENABLE_STATUSD";
    public static final String EXTRA_DEVICE_ID = "com.sohu.tv.beedemo.DEVICE_ID";
    public static final String EXTRA_CREATE_ROOM = "com.sohu.tv.beedemo.CREATE_ROOM";

    private static final String TAG = "AudioChatRoom";
    private static final int CAPTURE_PERMISSION_REQUEST_CODE = 1;

    // List of mandatory application permissions.
    private static final String[] MANDATORY_PERMISSIONS = {
            Manifest.permission.RECORD_AUDIO,           //Push audio
            Manifest.permission.WRITE_EXTERNAL_STORAGE, //Write log
            Manifest.permission.READ_EXTERNAL_STORAGE,  //Upload log
            Manifest.permission.MODIFY_AUDIO_SETTINGS,
            Manifest.permission.INTERNET
    };

    private static final int kPermutation[] = {
        0, 1, 2, 3, 4, 4, 5, 5, 5, 5, 6,
        6, 6, 6, 6, 7, 7, 7, 7, 8, 8, 8,
        9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9
    };

    // Peer connection statistics callback period in ms.
    private static final int STAT_CALLBACK_PERIOD = 200;

    //UI elements
    private Toast logToast;
    private HudFragment hudFragment;
    private CpuMonitor cpuMonitor;
    private UserAdapter userAdapter;
    private DrawerLayout drawerLayout;
    private NavigationView navigation;
    private ActionBarDrawerToggle drawerToggle;
    private View openedDrawerView;
    private RadioButton localSelButton;
    private RadioButton remoteSelButton;
    private RadioGroup statsDirectionSelGroup;
    boolean statsLocalDirection = true;
    boolean showVolume = false;
    boolean showStatistic = false;

    //Bee video room.
    private VideoRoom beeVideoRoom;
    private boolean beeInitialized = false;
    private int connectedCount = 0;
    private boolean statsEnabled = false;
    private int monitorStatsIndex = -2;
    private int mediaType = MediaType.eMediaType_Audio.getValue();
    private int boardType = BoardType.eBoardType_None.ordinal();
    private BeeSystemParam bee_param_ = new BeeSystemParam();
    private RoomParameters roomParameters;
    User localUser = null;
    private HashMap<String, User> userTable = new HashMap<>();
    private boolean createRoom = false;
    String token = "5eb73bb4918354cee213903c3940c0e6183f289d";
    String nickName = null;
    String streamName = null;
    int _abs_max = 0;
    int currentStatsCount = 0;

    private class RoomParameters {
        private final String roomId;
        private final String uid;
        private final String uName;
        private final boolean loopback;

        private RoomParameters(String roomId, String uid, String uName, boolean loopback) {
            this.roomId = roomId;
            this.uid = uid;
            this.uName = uName;
            this.loopback = loopback;
        }
    }

    enum UserState {
        eUserState_Idle,
        eUserState_Connecting,
        eUserState_Connected,
        eUserState_DisConnecting
    }

    private class User {
        private String uid;
        private String uName;
        private String streamName;
        private int mediaType = MediaType.eMediaType_Audio.getValue();
        private BeeSystemDefine.BeePartyType partyType;
        private BeeSystemDefine.BeeSvcType svcType;
        private boolean creator = false;
        private BoardType boardType = BoardType.eBoardType_None;
        private UserState userState = UserState.eUserState_Idle;
        private UserModel model = null;
        private Timer volumeTimer = new Timer();

        private void connect(int mediaType) {
            if (beeVideoRoom != null) {
                beeVideoRoom.connect(uid, streamName, mediaType, null);
                userState = UserState.eUserState_Connecting;
            }
        }

        private void disconnect() {
            if (beeVideoRoom != null) {
                beeVideoRoom.disconnect(uid, streamName, 0);
                userState = UserState.eUserState_DisConnecting;
            }
        }

        private void onConnected() {
            userState = UserState.eUserState_Connected;
            connectedCount++;
            if (showVolume) {
                volumeTimer.schedule(new TimerTask() {
                    @Override
                    public void run() {
                        if (beeVideoRoom != null) {
                            if (partyType == BeePartyType.eBeePartyType_Local) {
                                beeVideoRoom.getAudioInputLevel(streamName);
                            } else {
                                beeVideoRoom.getAudioOutputLevel(uid, streamName);
                            }
                        }
                    }
                }, 0, 200);
            }
        }

        private void onDisConnected() {
            userState = UserState.eUserState_Idle;
            connectedCount--;
        }
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_audio_chat_room);

        //Dynamically request permissions first if version >= 23.
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) {
            requestPermissions();
        }

        // Check for mandatory permissions.
        for (String permission : MANDATORY_PERMISSIONS) {
            if (checkCallingOrSelfPermission(permission) != PackageManager.PERMISSION_GRANTED) {
                logAndToast("Permission " + permission + " is not granted");
                setResult(RESULT_CANCELED);
                finish();
                return;
            }
        }

        List<UserModel> userList = new ArrayList<>();
        userAdapter = new UserAdapter(AudioChatRoomActivity.this, R.layout.user_list_item, userList);
        ListView listView = findViewById(R.id.list_view);
        listView.setAdapter(userAdapter);

        loadDrawer();

        final Intent intent = getIntent();
        String roomId = intent.getStringExtra(EXTRA_ROOMID);
        Log.d(TAG, "Room ID: " + roomId);
        if (roomId == null || roomId.length() == 0) {
            logAndToast(getString(R.string.missing_url));
            Log.e(TAG, "Incorrect room ID in intent!");
            setResult(RESULT_CANCELED);
            finish();
            return;
        }

        showVolume = intent.getBooleanExtra(EXTRA_SHOW_VOLUME, false);
        showStatistic = intent.getBooleanExtra(EXTRA_DISPLAY_HUD, false);

        String title = "语音聊天室:" + roomId;
        android.support.v7.app.ActionBar actionBar = getSupportActionBar();
        if(actionBar != null){
            actionBar.setHomeButtonEnabled(true);
            actionBar.setDisplayHomeAsUpEnabled(true);
            actionBar.setTitle(title);
        }

        hudFragment = new HudFragment();
        cpuMonitor = new CpuMonitor(this);
        hudFragment.setCpuMonitor(cpuMonitor);
        hudFragment.setArguments(intent.getExtras());

        String androidId = Settings.System.getString(getContentResolver(), Settings.System.ANDROID_ID);
        nickName = intent.getStringExtra(EXTRA_NICKNAME);
        if (nickName == null) {
            nickName = androidId;
        }
        roomParameters = new RoomParameters(roomId, androidId, nickName, false);

        Context context = getApplicationContext();
        String logPath = intent.getStringExtra(EXTRA_LOG_PATH);
        boolean enableStatusd = intent.getBooleanExtra(EXTRA_ENABLE_STATUSD, true);
        String deviceId = intent.getStringExtra(EXTRA_DEVICE_ID);
        createRoom = intent.getBooleanExtra(EXTRA_CREATE_ROOM, true);
        initBeeParam(logPath, enableStatusd, deviceId, bee_param_);

        beeVideoRoom = new VideoRoom();
        if (!beeVideoRoom.init(bee_param_, context, false, false, null, 10000, this)) {
            logAndToast("Bee initialize fail.");
            return;
        }

        BeeAudioParam audioParam = new BeeAudioParam();
        audioParam.audioStartBitrate = intent.getIntExtra(EXTRA_AUDIO_BITRATE, 0);
        audioParam.audioCodec = intent.getStringExtra(EXTRA_AUDIOCODEC);
        audioParam.noAudioProcessing = intent.getBooleanExtra(EXTRA_NOAUDIOPROCESSING_ENABLED, false);
        audioParam.useOpenSLES = intent.getBooleanExtra(EXTRA_OPENSLES_ENABLED, false);
        audioParam.disableBuiltInAEC = intent.getBooleanExtra(EXTRA_DISABLE_BUILT_IN_AEC, false);
        audioParam.disableBuiltInAGC = intent.getBooleanExtra(EXTRA_DISABLE_BUILT_IN_AGC, false);
        audioParam.disableBuiltInNS = intent.getBooleanExtra(EXTRA_DISABLE_BUILT_IN_NS, false);
        audioParam.enableLevelControl = intent.getBooleanExtra(EXTRA_ENABLE_LEVEL_CONTROL, false);
        beeVideoRoom.setAudioParam(audioParam);
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
        }
    }

    // Log |msg| and Toast about it.
    private void logAndToast(String msg) {
        Log.d(TAG, msg);
        if (logToast != null) {
            logToast.cancel();
        }
        logToast = Toast.makeText(this, msg, Toast.LENGTH_SHORT);
        logToast.show();
    }

    private void initBeeParam(String logPath, boolean enableStatusd, String deviceId, BeeSystemParam param) {
        param.platform_type = BeeSystemDefine.BeePlatformType.kPlatformType_PC.ordinal();
        param.net_type = BeeSystemDefine.BeeNetType.kNetType_WireLine.ordinal();
        param.app_name = "AudioChat";
        param.app_version = "1.0";
        param.system_info = "Android4.4";
        param.machine_code = deviceId;
        param.log_path = logPath;
        param.log_level = BeeSystemDefine.BeeLogLevel.kLogLevel_Debug.ordinal();
        param.log_max_line = 0;
        param.log_volume_count = 0;
        param.log_volume_size = 0;
        param.session_count = 16;
        param.enable_statusd = enableStatusd;
    }

    public static String createRandom(boolean numberFlag, int length){
        String retStr = "";
        String strTable = numberFlag ? "1234567890" : "1234567890abcdefghijkmnpqrstuvwxyz";
        int len = strTable.length();
        boolean bDone = true;
        do {
            retStr = "";
            int count = 0;
            for (int i = 0; i < length; i++) {
                double dblR = Math.random() * len;
                int intR = (int) Math.floor(dblR);
                char c = strTable.charAt(intR);
                if (('0' <= c) && (c <= '9')) {
                    count++;
                }
                retStr += strTable.charAt(intR);
            }
            if (count >= 2) {
                bDone = false;
            }
        } while (bDone);

        return retStr;
    }

    private void startCall() {
        if (!beeInitialized) {
            logAndToast("Not initialized");
            return;
        }

        if (beeVideoRoom == null) {
            logAndToast("Video room not created.");
            return;
        }

        logAndToast("Connecting room " + roomParameters.roomId);

        streamName = createRandom(false, 20);
        if (createRoom) {
            boardType = BoardType.eBoardType_Teacher.ordinal();
            beeVideoRoom.create(
                    roomParameters.roomId,
                    roomParameters.uid,
                    roomParameters.uName,
                    token,
                    streamName,
                    mediaType,
                    BeeSystemDefine.BoardType.eBoardType_Teacher,
                    false,
                    "",
                    null,
                    0,
                    0,
                    0,
                    null);
        } else {
            boardType = BoardType.eBoardType_Student.ordinal();
            beeVideoRoom.join(
                    roomParameters.roomId,
                    roomParameters.uid,
                    roomParameters.uName,
                    token,
                    streamName,
                    mediaType,
                    BeeSystemDefine.BoardType.eBoardType_Student,
                    false,
                    "",
                    null,
                    0,
                    0,
                    0,
                    null);
        }
    }

    private void disconnect() {
        if (beeVideoRoom != null) {
            beeVideoRoom.leave();
            beeVideoRoom = null;
        }

        if (iceConnected()) {
            setResult(RESULT_OK);
        } else {
            setResult(RESULT_CANCELED);
        }

        finish();
    }

    private Boolean iceConnected() {
        return connectedCount > 0;
    }

    public void onInitialize(final BeeErrorCode error) {
        runOnUiThread(new Runnable() {
            @Override
            public void run() {
                if (error == BeeErrorCode.kBeeErrorCode_Success) {
                    beeInitialized = true;
                    startCall();
                } else {
                    logAndToast("Bee initialize fail:" + error + ".");
                }
            }
        });
    }

    public void onUnInitialize(final BeeErrorCode error) {
        runOnUiThread(new Runnable() {
            @Override
            public void run() {
                logAndToast("onUnInitialize " + error + ".");
            }
        });
    }

    public void onJoin(final String streamName, final BeeErrorCode error, final String msg) {
        runOnUiThread(new Runnable() {
            @Override
            public void run() {
                Logging.d(TAG, "onJoin streamName:" + streamName + " error:" + error + ".");
                if (error == BeeErrorCode.kBeeErrorCode_Success) {
                    if (!statsEnabled) {
                        if (showStatistic) {
                            beeVideoRoom.enableStatsEvents(null, null, BeePartyType.eBeePartyType_Local, STAT_CALLBACK_PERIOD);
                        }
                        statsEnabled = true;
                    }

                    localUser = new User();
                    localUser.uName = nickName;
                    localUser.streamName = streamName;
                    localUser.mediaType = MediaType.eMediaType_Audio.getValue();
                    localUser.partyType = BeePartyType.eBeePartyType_Local;
                    localUser.creator = (boardType == BoardType.eBoardType_Teacher.ordinal());
                    localUser.onConnected();

                    addMemberToListView(localUser);

                    logAndToast("Connected to room " + roomParameters.roomId);
                } else {
                    logAndToast("Join failed " + error + " msg:" + msg + ".");
                }
            }
        });
    }

    public void onLeave(final BeeErrorCode error) {
        runOnUiThread(new Runnable() {
            @Override
            public void run() {
                Logging.d(TAG, "onLeave " + " error:" + error + ".");
                delMemberFromListView(localUser);
            }
        });
    }

    public void onMembers(final VideoRoom.MemberInfo[] remoteMembers) {
        runOnUiThread(new Runnable() {
            @Override
            public void run() {
                Logging.d(TAG, "Report " + remoteMembers.length + " members.");
                for (VideoRoom.MemberInfo member : remoteMembers) {
                    if (member.svcType != BeeSystemDefine.BeeSvcType.eBeeSvcType_Media) {
                        continue;
                    }
                    createUser(member);
                }
            }
        });
    }

    public void onConnect(final String uid, final String streamName, final BeeErrorCode error, final String msg) {
        runOnUiThread(new Runnable() {
            @Override
            public void run() {
                Logging.d(TAG, "User " + streamName + " connect result " + error + " msg:" + msg + ".");
                User user = userTable.get(streamName);
                if (user != null) {
                    if (error == BeeErrorCode.kBeeErrorCode_Success) {
                        user.onConnected();
                        addMemberToListView(user);
                    } else {
                        userTable.remove(streamName);
                    }
                }
            }
        });
    }

    public void onDisConnect(final String uid, final String streamName, final BeeErrorCode error, final String msg) {
        runOnUiThread(new Runnable() {
            @Override
            public void run() {
                if (streamName == null) {
                    logAndToast("Disconnected, reason:" + error + " msg:" + msg + ".");
                    disconnect();
                } else {
                    Logging.d(TAG, "User " + streamName + " disconnect result " + error + ".");
                    User user = userTable.get(streamName);
                    if (user != null) {
                        user.onDisConnected();
                        userTable.remove(streamName);
                        delMemberFromListView(user);
                    }
                }
            }
        });
    }

    public void onStats(final String uid, final String streamName, final BeePartyType partyType, final StatsReport[] reports) {
        runOnUiThread(new Runnable() {
            @Override
            public void run() {
                if (partyType == BeePartyType.eBeePartyType_Local) {
                    if (statsLocalDirection) {
                        showStats(reports);
                    }
                } else {
                    if (!statsLocalDirection) {
                        showStats(reports);
                    }
                }
            }
        });
    }

    public void onSlowLink(final String uid, final String streamName, final BeePartyType partyType, final String info) {
        runOnUiThread(new Runnable() {
            @Override
            public void run() {
                logAndToast("Slow link in user:" + streamName + " type:" + partyType + " info:" + info);
            }
        });
    }

    public void onMessage(final String from, final String message, final BeeSystemDefine.BeeSvcType svcType) {
        runOnUiThread(new Runnable() {
            @Override
            public void run() {
                logAndToast("onMessage from:" + from + " message:" + message + ".");
            }
        });
    }

    public void onAudioManagerDevicesChanged(final AppRTCAudioManager.AudioDevice device, final Set<AppRTCAudioManager.AudioDevice> availableDevices) {
        runOnUiThread(new Runnable() {
            @Override
            public void run() {
                Logging.d(TAG, "onAudioManagerDevicesChanged: " + availableDevices + ", " + "selected: " + device);
            }
        });
    }

    public void onAudioInputLevel(final String streamName, final int level) {
        runOnUiThread(new Runnable() {
            @Override
            public void run() {
                User user = localUser;
                if (user == null) {
                    return;
                }

                updateMemberVolume(user, level);
            }
        });
    }

    public void onAudioOutputLevel(final String uid, final String streamName, final int level) {
        runOnUiThread(new Runnable() {
            @Override
            public void run() {
                User user = userTable.get(streamName);
                if (user == null) {
                    return;
                }

                updateMemberVolume(user, level);
            }
        });
    }

    private void createUser(VideoRoom.MemberInfo memberInfo) {
        if (memberInfo == null) {
            return;
        }

        if (null != userTable.get(memberInfo.streamName)) {
            return;
        }

        User user = new User();
        user.uid = memberInfo.uid;
        user.streamName = memberInfo.streamName;
        user.uName = memberInfo.uName;
        user.partyType = memberInfo.partyType;
        user.svcType = memberInfo.svcType;
        user.mediaType = memberInfo.mediaType;
        user.boardType = memberInfo.boardType;
        user.creator = (memberInfo.boardType == BeeSystemDefine.BoardType.eBoardType_Teacher);
        user.userState = UserState.eUserState_Idle;

        userTable.put(user.streamName, user);
        user.connect(mediaType);
    }

    // Activity interfaces
    @Override
    public void onPause() {
        super.onPause();
        if (cpuMonitor != null) {
            cpuMonitor.pause();
        }
    }

    @Override
    public void onResume() {
        super.onResume();
        if (cpuMonitor != null) {
            cpuMonitor.resume();
        }
    }

    protected void destroyMyself() {
        Thread.setDefaultUncaughtExceptionHandler(null);
        disconnect();
        if (logToast != null) {
            logToast.cancel();
        }
    }

    @Override
    protected void onDestroy() {
        destroyMyself();
        super.onDestroy();
    }

    @Override
    public void onActivityResult(int requestCode, int resultCode, Intent data) {
        if (requestCode != CAPTURE_PERMISSION_REQUEST_CODE)
            return;
        startCall();
    }

    void addMemberToListView(final User user) {
        UserModel userModel = new UserModel();
        userModel.nickName = user.uName;
        userModel.volume = 0;
        userModel.creator = user.creator;
        user.model = userModel;
        userModel.local = (user.partyType == BeePartyType.eBeePartyType_Local);
        userAdapter.add(userModel);
    }


    void delMemberFromListView(final User user) {
        runOnUiThread(new Runnable() {
            @Override
            public void run() {
                userAdapter.remove(user.model);
            }
        });
    }

    void updateMemberVolume(final User user, final int volume) {
        runOnUiThread(new Runnable() {
            @Override
            public void run() {
                if (user.model != null) {
                    user.model.volume = volume;
                    userAdapter.notifyDataSetChanged();
                }
            }
        });
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        int p = item.getItemId();
        switch (p) {
            case android.R.id.home:
                destroyMyself(); // back button
                return true;
        }
        return super.onOptionsItemSelected(item);
    }

    private void loadDrawer() {
        drawerLayout = findViewById(R.id.drawer_layout);
        navigation = findViewById(R.id.drawer_navigation);
        drawerToggle = new ActionBarDrawerToggle(AudioChatRoomActivity.this,drawerLayout,null,R.string.openString,R.string.openString){
            @Override
            public void onDrawerOpened(View drawerView) {
                super.onDrawerOpened(drawerView);
                openedDrawerView = drawerView;
                localSelButton = drawerView.findViewById(R.id.stats_direction_local);
                remoteSelButton = drawerView.findViewById(R.id.stats_direction_remote);
                statsDirectionSelGroup = drawerView.findViewById(R.id.stats_direction);
                statsDirectionSelGroup.setOnCheckedChangeListener(new RadioGroup.OnCheckedChangeListener() {
                    @Override
                    public void onCheckedChanged(RadioGroup group, int checkedId) {
                        switch (checkedId) {
                            case R.id.stats_direction_local:
                                statsLocalDirection = true;
                                break;
                            case R.id.stats_direction_remote:
                                statsLocalDirection = false;
                                break;
                        }
                    }
                });
                return;
            }
            @Override
            public void onDrawerClosed(View drawerView) {
                super.onDrawerClosed(drawerView);
                openedDrawerView = null;
            }
        };
        navigation.setNavigationItemSelectedListener(new NavigationView.OnNavigationItemSelectedListener() {
            @Override
            public boolean onNavigationItemSelected(MenuItem menuItem) {
                menuItem.setChecked(true);
                drawerLayout.closeDrawers();
                return false;
            }
        });
        drawerToggle.syncState();
        drawerLayout.setDrawerListener(drawerToggle);

        setDrawerLeftEdgeSize(0.5f);
    }

    public void setDrawerLeftEdgeSize(float displayWidthPercentage) {
        if (drawerLayout == null) {
            return;
        }

        try {
            Field leftDraggerField = drawerLayout.getClass().getDeclaredField("mLeftDragger");
            leftDraggerField.setAccessible(true);
            ViewDragHelper leftDragger = (ViewDragHelper) leftDraggerField.get(drawerLayout);
            Field edgeSizeField = leftDragger.getClass().getDeclaredField("mEdgeSize");
            edgeSizeField.setAccessible(true);
            int edgeSize = edgeSizeField.getInt(leftDragger);
            DisplayMetrics dm = new DisplayMetrics();
            getWindowManager().getDefaultDisplay().getMetrics(dm);
            edgeSizeField.setInt(leftDragger, Math.max(edgeSize, (int) (dm.widthPixels * displayWidthPercentage)));
        } catch (Exception e) {
        }
    }

    private void showStats(final StatsReport[] reports) {
        if (openedDrawerView == null) {
            return;
        }

        int totalCount = 1000 / STAT_CALLBACK_PERIOD;
        currentStatsCount++;
        if (currentStatsCount < totalCount) {
            return;
        }
        currentStatsCount = 0;

        TextView encoderStatView = openedDrawerView.findViewById(R.id.encoder_stat_call);
        TextView hudViewBwe = openedDrawerView.findViewById(R.id.hud_stat_bwe);
        TextView hudViewConnection = openedDrawerView.findViewById(R.id.hud_stat_connection);
        TextView hudViewVideoSend = openedDrawerView.findViewById(R.id.hud_stat_video_send);
        TextView hudViewVideoRecv = openedDrawerView.findViewById(R.id.hud_stat_video_recv);

        hudFragment.setViews(encoderStatView, hudViewBwe, hudViewConnection, hudViewVideoSend, hudViewVideoRecv);
        hudFragment.updateEncoderStatistics(reports);
    }

    @Override
    public boolean onKeyDown(int keyCode, KeyEvent event) {
        switch (keyCode) {
            case KeyEvent.KEYCODE_VOLUME_DOWN: {
                if (beeVideoRoom != null && beeVideoRoom.getCurrentPlayoutVolume() <= 0) {
                    beeVideoRoom.setPlayoutMute(true);
                }
            }
            break;
            case KeyEvent.KEYCODE_VOLUME_UP: {
                if (beeVideoRoom != null) {
                    beeVideoRoom.setPlayoutMute(false);
                }
            }
            break;
        }
        return super.onKeyDown(keyCode, event);
    }

}
