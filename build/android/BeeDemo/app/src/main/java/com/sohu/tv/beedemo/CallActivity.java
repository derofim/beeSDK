package com.sohu.tv.beedemo;

import android.Manifest;
import android.app.Activity;
import android.app.AlertDialog;
import android.app.FragmentTransaction;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.graphics.Rect;
import android.media.projection.MediaProjectionManager;
import android.os.Build;
import android.os.Bundle;
import android.os.Environment;
import android.os.Handler;
import android.provider.Settings;
import android.support.v4.app.ActivityCompat;
import android.support.v4.content.ContextCompat;
import android.util.DisplayMetrics;
import android.util.Log;
import android.view.View;
import android.view.ViewGroup;
import android.view.Window;
import android.view.WindowManager;
import android.view.WindowManager.LayoutParams;
import android.widget.Button;
import android.widget.FrameLayout;
import android.widget.RelativeLayout;
import android.widget.Toast;

import java.io.File;
import java.util.ArrayList;
import java.util.Comparator;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.PriorityQueue;
import java.util.Vector;

import com.sohu.tv.bee.BeeAsyncHandler;
import com.sohu.tv.bee.BeeOpenSessionParam;
import com.sohu.tv.bee.BeeSDK;
import com.sohu.tv.bee.BeeSDKSink;
import com.sohu.tv.bee.BeeSurfaceViewRenderer;
import com.sohu.tv.bee.BeeSystemDefine;
import com.sohu.tv.bee.BeeSystemDefine.BeeErrorCode;
import com.sohu.tv.bee.BeeSystemParam;
import com.sohu.tv.bee.BeeVideoRoom;
import com.sohu.tv.bee.BeeVideoRoomDefine;
import com.sohu.tv.bee.BeeVideoRoomSink;
import com.sohu.tv.bee.BeeWhiteBoard;
import com.sohu.tv.bee.BeeWhiteBoardDefine;
import com.sohu.tv.bee.BeeWhiteBoardView;
import com.sohu.tv.bee.CpuMonitor;

import org.webrtc.Camera2Enumerator;
import org.webrtc.EglBase;
import org.webrtc.Logging;
import org.webrtc.NetworkMonitor;
import org.webrtc.NetworkMonitorAutoDetect;
import org.webrtc.RendererCommon.ScalingType;

import static com.sohu.tv.bee.BeeVideoRoom.hasVideo;
import static com.sohu.tv.bee.BeeVideoRoomDefine.kBeeSvcType_VideoRoom;
import static com.sohu.tv.bee.BeeWhiteBoardDefine.kBeeSvcType_WhiteBoard;
import static org.webrtc.NetworkMonitorAutoDetect.ConnectionType.CONNECTION_NONE;
import static org.webrtc.NetworkMonitorAutoDetect.ConnectionType.CONNECTION_WIFI;

public class CallActivity extends Activity implements OnCallEvents, BeeVideoRoomSink, NetworkMonitor.NetworkObserver {
    public static final String EXTRA_ROOMID = "com.sohu.tv.beedmo.ROOMID";
    public static final String EXTRA_LOOPBACK = "com.sohu.tv.beedmo.LOOPBACK";
    public static final String EXTRA_VIDEO_CALL = "com.sohu.tv.beedmo.VIDEO_CALL";
    public static final String EXTRA_SCREENCAPTURE = "com.sohu.tv.beedmo.SCREENCAPTURE";
    public static final String EXTRA_CAMERA2 = "com.sohu.tv.beedmo.CAMERA2";
    public static final String EXTRA_VIDEO_WIDTH = "com.sohu.tv.beedmo.VIDEO_WIDTH";
    public static final String EXTRA_VIDEO_HEIGHT = "com.sohu.tv.beedmo.VIDEO_HEIGHT";
    public static final String EXTRA_VIDEO_FPS = "com.sohu.tv.beedmo.VIDEO_FPS";
    public static final String EXTRA_VIDEO_CAPTUREQUALITYSLIDER_ENABLED = "com.sohu.tv.beedmo.VIDEO_CAPTUREQUALITYSLIDER";
    public static final String EXTRA_VIDEO_BITRATE = "com.sohu.tv.beedmo.VIDEO_BITRATE";
    public static final String EXTRA_VIDEOCODEC = "com.sohu.tv.beedmo.VIDEOCODEC";
    public static final String EXTRA_HWCODEC_ENABLED = "com.sohu.tv.beedmo.HWCODEC";
    public static final String EXTRA_HWENCODER_ENABLED = "com.sohu.tv.beedmo.HWENCODER";
    public static final String EXTRA_HWDECODER_ENABLED = "com.sohu.tv.beedmo.HWDECODER";
    public static final String EXTRA_CAPTURETOTEXTURE_ENABLED = "com.sohu.tv.beedmo.CAPTURETOTEXTURE";
    public static final String EXTRA_FLEXFEC_ENABLED = "com.sohu.tv.beedmo.FLEXFEC";
    public static final String EXTRA_AUDIO_BITRATE = "com.sohu.tv.beedmo.AUDIO_BITRATE";
    public static final String EXTRA_AUDIOCODEC = "com.sohu.tv.beedmo.AUDIOCODEC";
    public static final String EXTRA_NOAUDIOPROCESSING_ENABLED = "com.sohu.tv.beedmo.NOAUDIOPROCESSING";
    public static final String EXTRA_AECDUMP_ENABLED = "com.sohu.tv.beedmo.AECDUMP";
    public static final String EXTRA_OPENSLES_ENABLED = "com.sohu.tv.beedmo.OPENSLES";
    public static final String EXTRA_DISABLE_BUILT_IN_AEC = "com.sohu.tv.beedmo.DISABLE_BUILT_IN_AEC";
    public static final String EXTRA_DISABLE_BUILT_IN_AGC = "com.sohu.tv.beedmo.DISABLE_BUILT_IN_AGC";
    public static final String EXTRA_DISABLE_BUILT_IN_NS = "com.sohu.tv.beedmo.DISABLE_BUILT_IN_NS";
    public static final String EXTRA_ENABLE_LEVEL_CONTROL = "com.sohu.tv.beedmo.ENABLE_LEVEL_CONTROL";
    public static final String EXTRA_DISPLAY_HUD = "com.sohu.tv.beedmo.DISPLAY_HUD";
    public static final String EXTRA_TRACING = "com.sohu.tv.beedmo.TRACING";
    public static final String EXTRA_CMDLINE = "com.sohu.tv.beedmo.CMDLINE";
    public static final String EXTRA_RUNTIME = "com.sohu.tv.beedmo.RUNTIME";
    public static final String EXTRA_VIDEO_FILE_AS_CAMERA = "com.sohu.tv.beedmo.VIDEO_FILE_AS_CAMERA";
    public static final String EXTRA_SAVE_REMOTE_VIDEO_TO_FILE = "com.sohu.tv.beedmo.SAVE_REMOTE_VIDEO_TO_FILE";
    public static final String EXTRA_SAVE_REMOTE_VIDEO_TO_FILE_WIDTH = "com.sohu.tv.beedmo.SAVE_REMOTE_VIDEO_TO_FILE_WIDTH";
    public static final String EXTRA_SAVE_REMOTE_VIDEO_TO_FILE_HEIGHT = "com.sohu.tv.beedmo.SAVE_REMOTE_VIDEO_TO_FILE_HEIGHT";
    public static final String EXTRA_USE_VALUES_FROM_INTENT = "com.sohu.tv.beedmo.USE_VALUES_FROM_INTENT";
    public static final String EXTRA_DATA_CHANNEL_ENABLED = "com.sohu.tv.beedmo.DATA_CHANNEL_ENABLED";
    public static final String EXTRA_ORDERED = "com.sohu.tv.beedmo.ORDERED";
    public static final String EXTRA_MAX_RETRANSMITS_MS = "com.sohu.tv.beedmo.MAX_RETRANSMITS_MS";
    public static final String EXTRA_MAX_RETRANSMITS = "com.sohu.tv.beedmo.MAX_RETRANSMITS";
    public static final String EXTRA_PROTOCOL = "com.sohu.tv.beedmo.PROTOCOL";
    public static final String EXTRA_NEGOTIATED = "com.sohu.tv.beedmo.NEGOTIATED";
    public static final String EXTRA_ID = "com.sohu.tv.beedemo.ID";
    public static final String EXTRA_LOG_PATH = "com.sohu.tv.beedemo.LOG_PATH";
    public static final String EXTRA_ENABLE_STATUSD = "com.sohu.tv.beedemo.ENABLE_STATUSD";
    public static final String EXTRA_DEVICE_ID = "com.sohu.tv.beedemo.DEVICE_ID";
    public static final String EXTRA_CREATE_ROOM = "com.sohu.tv.beedemo.CREATE_ROOM";

    private static final String TAG = "BeeVideoRoomDemo";
    private static final int CAPTURE_PERMISSION_REQUEST_CODE = 1;

    private static final String TOKEN = "5eb73bb4918354cee213903c3940c0e6183f289d";
    // List of mandatory application permissions.
    private static final String[] MANDATORY_PERMISSIONS = {
            Manifest.permission.CAMERA,                 //Push video
            Manifest.permission.RECORD_AUDIO,           //Push audio
            Manifest.permission.WRITE_EXTERNAL_STORAGE, //Write log
            Manifest.permission.READ_EXTERNAL_STORAGE,  //Upload log
            Manifest.permission.MODIFY_AUDIO_SETTINGS,
            Manifest.permission.INTERNET
    };

    // Peer connection statistics callback period in ms.
    private static final int STAT_CALLBACK_PERIOD = 1000;

    // Main video screen position.
    private static final int MAIN_X = 0;
    private static final int MAIN_Y = 0;
    private static final int MAIN_WIDTH = 100;
    private static final int MAIN_HEIGHT = 100;

    // Sub video screens position.
    private Rect[] SUB_RECTS = null;

    // UI elements.
    private BeeSurfaceViewRenderer[] localRenderer = null;
    private final int maxPartyCount = 4;
    private BeeSurfaceViewRenderer[] remoteRenderers = null;
    private PercentFrameLayout mainRendererLayout = null;
    private PercentFrameLayout[] subRendererLayouts = null;
    //mainRenderer used as vector of size 1 for SurfaceViewRenderer is non-cloneable, to distinguish
    //from localRender there must be a container to clone.
    private BeeSurfaceViewRenderer[] mainRenderer = null;
    private BeeSurfaceViewRenderer[] subRenderers = null;
    private Boolean[] subRendererStartedFlag = new Boolean[maxPartyCount];

    private ScalingType scalingType;
    private Toast logToast;
    private boolean commandLineRun;
    private int runTimeMs;
    private boolean activityRunning;

    private boolean isError;
    private boolean callControlFragmentVisible = true;
    private long callStartedTimeMs = 0;
    private boolean micEnabled = true;
    private boolean screencaptureEnabled = false;
    private static Intent mediaProjectionPermissionResultData;
    private static int mediaProjectionPermissionResultCode;

    // Controls.
    private CallFragment callFragment;
    private HudFragment hudFragment;
    private CpuMonitor cpuMonitor;
    Context context = null;

    //Bee White Board
    private BeeWhiteBoard beeWhiteBoard;
    private BeeWhiteBoardView beeWhiteBoardView;

    //Bee video room.
    private BeeVideoRoom beeVideoRoom;
    private int connectedCount = 0;
    private int connectedVideoCount = 0;
    private boolean statsEnabled = false;
    private int monitorStatsIndex = -2;
    private int mediatype = 0;
    private boolean pushVideo = true;
    private boolean pushAudio = true;
    private String extradata;
    private int videoWidth = 640;
    private int videoHeight = 480;
    private int videoFps = 30;
    private int beeTimeout = 10000;
    private RoomParameters roomParameters;
    private PriorityQueue<VideoRendererWrapper> idleRendererQueue;
    private HashMap<String, User> userTable = new HashMap<>();
    private boolean createRoom = false;

    private int handler_ = -1;
    private Vector<BeeOpenSessionParam.BeeSDKCapability> beeCapabilities = new Vector<>();
    private CallActivity this_ = this;
    private NetworkMonitorAutoDetect.ConnectionType currentConnectionType = CONNECTION_NONE;
    private Object allowNotWifiObject = new Object();
    private boolean allowNotWifi = false;

    private class RoomParameters {
        private final String roomName;
        private final String uid;
        private final String nickName;
        private final boolean loopback;
        private RoomParameters(String roomName, String uid, String nickName, boolean loopback) {
            this.roomName = roomName;
            this.uid = uid;
            this.nickName = nickName;
            this.loopback = loopback;
        }
    }

    private class VideoRendererWrapper {
        VideoRendererWrapper(int remoteIndex, int yPos, BeeSurfaceViewRenderer renderer) {
            this.remoteIndex = remoteIndex;
            this.yPos = yPos;
            this.renderer = renderer;
        }
        private int remoteIndex;
        private int yPos;
        private BeeSurfaceViewRenderer renderer;
    }

    enum UserState {
        eUserState_Idle,
        eUserState_Connecting,
        eUserState_Connected,
        eUserState_DisConnecting
    }

    private class User {
        private String uid;
        private String nickName;
        private String streamName;
        private BeeVideoRoomDefine.VideoRoomRole role;
        private BeeVideoRoomDefine.VideoRoomPartyType partyType;
        private int mediaType;
        private UserState userState = UserState.eUserState_Idle;
        private VideoRendererWrapper videoRendererWrapper;

        private void connect(int mediaType) {
            if (beeVideoRoom != null) {
                beeVideoRoom.connect(uid, streamName, mediaType, videoRendererWrapper==null?null:videoRendererWrapper.renderer);
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
        }

        private void onDisConnected() {
            userState = UserState.eUserState_Idle;
            connectedCount--;
        }
    }

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        Thread.setDefaultUncaughtExceptionHandler(new UnhandledExceptionHandler(this));
        context = getApplicationContext();
        callStartedTimeMs = System.currentTimeMillis();
        // Set window styles for fullscreen-window size. Needs to be done before
        // adding content.
        requestWindowFeature(Window.FEATURE_NO_TITLE);
        getWindow().addFlags(
                LayoutParams.FLAG_FULLSCREEN |
                LayoutParams.FLAG_KEEP_SCREEN_ON |
                LayoutParams.FLAG_DISMISS_KEYGUARD |
                LayoutParams.FLAG_SHOW_WHEN_LOCKED |
                LayoutParams.FLAG_TURN_SCREEN_ON);
        getWindow().getDecorView().setSystemUiVisibility(
                View.SYSTEM_UI_FLAG_HIDE_NAVIGATION |
                View.SYSTEM_UI_FLAG_FULLSCREEN |
                View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY);

        setContentView(R.layout.activity_call);

        scalingType = ScalingType.SCALE_ASPECT_FILL;

        // Create UI controls.
        initSubVideoPosition();
        remoteRenderers = new BeeSurfaceViewRenderer[maxPartyCount];
        subRendererLayouts = new PercentFrameLayout[maxPartyCount];

        localRenderer = new BeeSurfaceViewRenderer[1]; //Using vector for cloning.
        localRenderer[0] = findViewById(R.id.local_video_view);
        remoteRenderers[0] = findViewById(R.id.remote_video_view0);
        remoteRenderers[1] = findViewById(R.id.remote_video_view1);
        remoteRenderers[2] = findViewById(R.id.remote_video_view2);
        remoteRenderers[3] = findViewById(R.id.remote_video_view3);

        mainRendererLayout = findViewById(R.id.local_video_layout);
        subRendererLayouts[0] = findViewById(R.id.remote_video_layout0);
        subRendererLayouts[1] = findViewById(R.id.remote_video_layout1);
        subRendererLayouts[2] = findViewById(R.id.remote_video_layout2);
        subRendererLayouts[3] = findViewById(R.id.remote_video_layout3);


        mainRenderer = localRenderer.clone();
        subRenderers = remoteRenderers.clone();

        for (int i = 0;i < maxPartyCount;++i) {
            subRendererStartedFlag[i] = false;
        }

        idleRendererQueue = new PriorityQueue<VideoRendererWrapper>(maxPartyCount, new Comparator<VideoRendererWrapper>() {
            @Override
            public int compare(VideoRendererWrapper o1, VideoRendererWrapper o2) {
                return o2.yPos - o1.yPos;
            }
        });
        idleRendererQueue.add(new VideoRendererWrapper(0, SUB_RECTS[0].top, remoteRenderers[0]));
        idleRendererQueue.add(new VideoRendererWrapper(1, SUB_RECTS[1].top, remoteRenderers[1]));
        idleRendererQueue.add(new VideoRendererWrapper(2, SUB_RECTS[2].top, remoteRenderers[2]));
        idleRendererQueue.add(new VideoRendererWrapper(3, SUB_RECTS[3].top, remoteRenderers[3]));

        callFragment = new CallFragment();
        hudFragment = new HudFragment();

        // Show/hide call control fragment on view click.
        View.OnClickListener listener = new View.OnClickListener() {
          @Override
          public void onClick(View view) {
              if (!activityRunning || mainRenderer == null || subRenderers == null) {
                  return;
              }

              BeeSurfaceViewRenderer selected_renderer = (BeeSurfaceViewRenderer) findViewById(view.getId());
              if (selected_renderer == mainRenderer[0]) {
                  //If main window clicked, toggle call control panel.
                  toggleCallControlFragmentVisibility();
              } else {
                  for (int i = 0;i < maxPartyCount;++i) {
                      //Swap clicked sub window and main window.
                      if (selected_renderer == subRenderers[i] && subRendererStartedFlag[i]) {
                          swapRenderer(i);
                          break;
                      }
                  }
              }
          }
        };

        mainRenderer[0].setOnClickListener(listener);
        mainRenderer[0].setZOrderMediaOverlay(true);

        for (int i = 0;i < maxPartyCount;++i) {
            subRenderers[i].setOnClickListener(listener);
            subRenderers[i].setZOrderMediaOverlay(true);
        }

        updateVideoView(false);

        // Check for mandatory permissions.
        for (String permission : MANDATORY_PERMISSIONS) {
            if (checkCallingOrSelfPermission(permission) != PackageManager.PERMISSION_GRANTED) {
                logAndToast("Permission " + permission + " is not granted");
                setResult(RESULT_CANCELED);
                finish();
                return;
            }
        }

        //Get data from ConnectActivity, mainly room id.
        final Intent intent = getIntent();

        // Get Intent parameters.
        String roomId = intent.getStringExtra(EXTRA_ROOMID);
        Log.d(TAG, "Room ID: " + roomId);
        if (roomId == null || roomId.length() == 0) {
            logAndToast(getString(R.string.missing_url));
            Log.e(TAG, "Incorrect room ID in intent!");
            setResult(RESULT_CANCELED);
            finish();
            return;
        }

        boolean loopback = intent.getBooleanExtra(EXTRA_LOOPBACK, false);
        boolean tracing = intent.getBooleanExtra(EXTRA_TRACING, false);
        pushAudio = true;
        pushVideo = intent.getBooleanExtra(EXTRA_VIDEO_CALL, true);
        mediatype = (pushAudio? BeeVideoRoomDefine.VideoRoomMediaType.eBeeVideoRoomMediaType_Audio.getValue():0) |
                    (pushVideo? BeeVideoRoomDefine.VideoRoomMediaType.eBeeVideoRoomMediaType_Video.getValue():0);

        int width = intent.getIntExtra(EXTRA_VIDEO_WIDTH, 0);
        if (width != 0) {
            videoWidth = width;
        }

        int height = intent.getIntExtra(EXTRA_VIDEO_HEIGHT, 0);
        if (height != 0) {
            videoHeight = height;
        }

        int fps = intent.getIntExtra(EXTRA_VIDEO_FPS, 30);
        if (fps != 0) {
            videoFps = fps;
        }

        screencaptureEnabled = intent.getBooleanExtra(EXTRA_SCREENCAPTURE, false);
        if (screencaptureEnabled && Build.VERSION.SDK_INT < Build.VERSION_CODES.LOLLIPOP) {
            logAndToast("Screen capture not supported on android " + Build.VERSION.SDK_INT);
            screencaptureEnabled = false;
        }

        // If capturing format is not specified for screencapture, use screen resolution.
        if (screencaptureEnabled) {
            try {
                DisplayMetrics displayMetrics = new DisplayMetrics();
                WindowManager windowManager =
                        (WindowManager) getApplication().getSystemService(Context.WINDOW_SERVICE);
                windowManager.getDefaultDisplay().getRealMetrics(displayMetrics);
                videoWidth = displayMetrics.widthPixels;
                videoHeight = displayMetrics.heightPixels;
            } catch (Exception e) {
                Logging.e(TAG, "Get window metric exception ", e);
            }
        }

        //
        String androidId = Settings.System.getString(getContentResolver(), Settings.System.ANDROID_ID);

        roomParameters = new RoomParameters(roomId, androidId, androidId, loopback);
        commandLineRun = intent.getBooleanExtra(EXTRA_CMDLINE, false);
        runTimeMs = intent.getIntExtra(EXTRA_RUNTIME, 0);

        Log.d(TAG, "VIDEO_FILE: '" + intent.getStringExtra(EXTRA_VIDEO_FILE_AS_CAMERA) + "'");

        // Create CPU monitor
        cpuMonitor = new CpuMonitor(this);
        hudFragment.setCpuMonitor(cpuMonitor);

        // Send intent arguments to fragments.
        callFragment.setArguments(intent.getExtras());
        hudFragment.setArguments(intent.getExtras());
        // Activate call and HUD fragments and start the call.
        FragmentTransaction ft = getFragmentManager().beginTransaction();
        ft.add(R.id.call_fragment_container, callFragment);
        ft.add(R.id.hud_fragment_container, hudFragment);
        ft.commit();

        // For command line execution run connection for <runTimeMs> and exit.
        if (commandLineRun && runTimeMs > 0) {
            (new Handler()).postDelayed(new Runnable() {
            @Override
            public void run() {
              disconnect();
            }
          }, runTimeMs);
        }

        createRoom = intent.getBooleanExtra(EXTRA_CREATE_ROOM, true);
        if (screencaptureEnabled) {
            try {
                MediaProjectionManager mediaProjectionManager =
                        (MediaProjectionManager) getApplication().getSystemService(
                                Context.MEDIA_PROJECTION_SERVICE);
                startActivityForResult(
                        mediaProjectionManager.createScreenCaptureIntent(), CAPTURE_PERMISSION_REQUEST_CODE);
            } catch (Exception e) {
                logAndToast("Start media projection activity exception " + e);
                screencaptureEnabled = false;
            }
        }

        BeeSDK.sharedInstance().registerNetMonitor(this);

        BeeSDK.sharedInstance().openSession(new BeeAsyncHandler(){
            @Override
            public void OpenSessionHandler(BeeErrorCode code, int handler, final Vector<BeeOpenSessionParam.BeeSDKCapability> capabilities) {
                if (code != BeeErrorCode.kBeeErrorCode_Success) {
                    runOnUiThread(new Runnable() {
                        @Override
                        public void run() {
                            logAndToast("Bee onOpenSession fail.");
                        }
                    });

                    return;
                }

                handler_ = handler;
                beeCapabilities = capabilities;

                start();
            }
        });
    }


    private void swapRenderer(int index) {
        if (index < 0 || index >= maxPartyCount) {
            return;
        }

        //Swap viewgroup.
        PercentFrameLayout tmpLayout = mainRendererLayout;
        mainRendererLayout = subRendererLayouts[index];
        subRendererLayouts[index] = tmpLayout;

        //Swap renderer.
        BeeSurfaceViewRenderer tmpRenderer = subRenderers[index];
        subRenderers[index] = mainRenderer[0];
        mainRenderer[0] = tmpRenderer;

        //Update UI.
        updateVideoView(true);
    }

    @Override
    public void onActivityResult(int requestCode, int resultCode, Intent data) {
        if (requestCode != CAPTURE_PERMISSION_REQUEST_CODE)
            return;
        mediaProjectionPermissionResultCode = resultCode;
        mediaProjectionPermissionResultData = data;
        startCall();
    }

    private boolean useCamera2() {
        return Camera2Enumerator.isSupported(this) && getIntent().getBooleanExtra(EXTRA_CAMERA2, true);
    }

    // Activity interfaces
    @Override
    public void onPause() {
        super.onPause();
        activityRunning = false;
        // Don't stop the video when using screencapture to allow user to show other apps to the remote
        // end.

        if (beeVideoRoom != null && !screencaptureEnabled) {
            //beeVideoRoom.stopVideoCapture();
        }

        if (cpuMonitor != null) {
            cpuMonitor.pause();
        }
    }

    @Override
    public void onResume() {
        super.onResume();
        activityRunning = true;
        if (cpuMonitor != null) {
            cpuMonitor.resume();
        }
    }

    @Override
    protected void onDestroy() {
        Thread.setDefaultUncaughtExceptionHandler(null);
        disconnect();
        if (logToast != null) {
            logToast.cancel();
        }
        activityRunning = false;
        BeeSDK.sharedInstance().unRegisterNetMonitor(this);
        BeeSDK.sharedInstance().closeSession(handler_, null);
        super.onDestroy();
    }

    @Override
    protected void onRestart() {
        super.onRestart();
        Logging.d(TAG, "onRestart");
        updateNetTypeLayout(currentConnectionType);
        start();
    }

    @Override
    protected void onStop() {
        super.onStop();
        Logging.d(TAG, "onStop");
        hangUpCall();
    }

    @Override
    public void onCallReconnect() {
        callStartedTimeMs = System.currentTimeMillis();
        synchronized (allowNotWifiObject) {
            allowNotWifiObject.notify();

            allowNotWifi = true;
        }
    }

    // CallFragment.OnCallEvents interface implementation.
    @Override
    public void onCallHangUp() {
        disconnect();
    }

    @Override
    public void onCameraSwitch() {
        if (beeVideoRoom != null) {
            beeVideoRoom.switchCamera();
        }
    }

    @Override
    public void onVideoScalingSwitch(ScalingType scalingType) {
        this.scalingType = scalingType;
        updateVideoView(false);
    }

    @Override
    public void onCaptureFormatChange(int width, int height, int framerate) {
        if (beeVideoRoom != null) {
            //changeCaptureFormat(width, height, framerate);
        }
    }

    @Override
    public boolean onToggleMic() {
        if (beeVideoRoom != null) {
            micEnabled = !micEnabled;
            //beeVideoRoom.setAudioEnabled(micEnabled);
        }
        return micEnabled;
    }

    // Helper functions.
    private void toggleCallControlFragmentVisibility() {
        if (!iceConnected() || !callFragment.isAdded()) {
            return;
        }

        // Show/hide call control fragment
        callControlFragmentVisible = !callControlFragmentVisible;
        FragmentTransaction ft = getFragmentManager().beginTransaction();
        if (callControlFragmentVisible) {
            //ft.show(callFragment);
            ft.show(hudFragment);
        } else {
            //ft.hide(callFragment);
            ft.hide(hudFragment);
        }
        ft.setTransition(FragmentTransaction.TRANSIT_FRAGMENT_FADE);
        ft.commit();
    }

    @Override
    public void onConnectionTypeChanged(NetworkMonitorAutoDetect.ConnectionType connectionType) {
        updateNetTypeLayout(connectionType);
        currentConnectionType = connectionType;
        userTableClear();
    }

    private void updateNetTypeLayout(NetworkMonitorAutoDetect.ConnectionType connectionType) {
        if (connectionType == CONNECTION_NONE) {
            reconnectLayout(false);
            notNetLayout(true);
        } else {
            if (connectionType == CONNECTION_WIFI) {
                reconnectLayout(false);
                notNetLayout(false);
                callStartedTimeMs = System.currentTimeMillis();
            } else if (connectionType != CONNECTION_WIFI) {
                if (false == allowNotWifi) {
                    reconnectLayout(true);
                } else {
                    reconnectLayout(false);
                }
                notNetLayout(false);
            }
        }
    }

    private void notNetLayout(final boolean isShow) {
        runOnUiThread(new Runnable() {
            @Override
            public void run() {
                callFragment.setNotNetLayout(isShow);
            }
        });
    }

    private void reconnectLayout(final boolean isShow) {
        runOnUiThread(new Runnable() {
            @Override
            public void run() {
                callFragment.setReconnectLayout(isShow);
                /*if (isShow) {
                    new AlertDialog.Builder(context).setMessage("正在使用非wifi网络，是否连接？").
                            setPositiveButton("继续播放", new DialogInterface.OnClickListener() {
                                @Override
                                public void onClick(DialogInterface dialog, int which) {
                                    allowNotWifi = true;
                                }
                            }).
                            setNegativeButton("取消播放", new DialogInterface.OnClickListener() {
                                @Override
                                public void onClick(DialogInterface dialog, int which) {
                                    allowNotWifi = false;
                                }
                    }).show();
                }*/
            }
        });
    }

    private void userTableClear() {
        for(Map.Entry<String, User> entry : userTable.entrySet()) {
            User user = entry.getValue();
            if (hasVideo(user.mediaType) && user.videoRendererWrapper != null) {
                int videoRendererToStop = onVideoRendererStop(user.videoRendererWrapper.remoteIndex);
                if (videoRendererToStop != -1) {
                    user.videoRendererWrapper.yPos = SUB_RECTS[videoRendererToStop].top;
                }
                recycleVideoRenderer(user.videoRendererWrapper);
            }
        }
        userTable.clear();
    }

    //Reset all controls in this layout.
    private void updateVideoView(boolean swap) {
        Boolean mainRendererSwapped = false;
        Boolean subRendererSwappedFlag[] = new Boolean[maxPartyCount];
        for (int i = 0;i < maxPartyCount;++i) {
            subRendererSwappedFlag[i] = false;
        }

        RelativeLayout mainLayout = findViewById(R.id.main_layout);
        FrameLayout callLayout = findViewById(R.id.call_fragment_container);
        FrameLayout hudLayout = findViewById(R.id.hud_fragment_container);

        //If some controls need to reset position, remove all necessary controls.
        if (swap) {
            mainLayout.removeView(mainRendererLayout);
            mainLayout.removeView(callLayout);
            mainLayout.removeView(hudLayout);
            mainLayout.addView(mainRendererLayout); //Move main renderer layout to bottom level, so add first.
        }

        if (mainRendererLayout != null) {
            mainRendererLayout.setPosition(MAIN_X, MAIN_Y, MAIN_WIDTH, MAIN_HEIGHT); //Set main layout position.
            if (mainRenderer!= null && mainRenderer[0] != null) {
                mainRenderer[0].setScalingType(scalingType); //Main renderer have scaling type to set by scaling button.
                mainRenderer[0].setMirror(mainRenderer[0] == localRenderer[0]); //Set mirror if is local renderer.
                if (swap) {
                    mainRenderer[0].setVisibility(View.GONE);
                    mainRendererLayout.removeView(mainRenderer[0]); //Remove view first.
                    mainRenderer[0].setZOrderOnTop(false); //Get rid of control z-order from main renderer.
                    mainRenderer[0].setZOrderMediaOverlay(false); //Get rid of media z-order from main renderer.
                    mainRendererSwapped = true;
                }
                mainRenderer[0].requestLayout(); //Request layout to be active.
            }
        }

        //As above, reset all sub layouts and sub renderers.
        for (int i = 0;i < maxPartyCount;++i) {
            if (subRenderers != null && subRendererLayouts != null && subRendererLayouts[i] != null && subRendererStartedFlag[i]) {
                subRendererLayouts[i].setPosition(SUB_RECTS[i].left, SUB_RECTS[i].top, SUB_RECTS[i].width(), SUB_RECTS[i].height());
                if (subRenderers[i] != null) {
                    subRenderers[i].setScalingType(ScalingType.SCALE_ASPECT_FIT);
                    subRenderers[i].setMirror(subRenderers[i] == localRenderer[0]);
                    if (swap) {
                        mainLayout.removeView(subRendererLayouts[i]);
                        subRendererLayouts[i].removeView(subRenderers[i]);
                        subRenderers[i].setVisibility(View.GONE);
                        subRenderers[i].setZOrderOnTop(true); //Get the top z-order.
                        subRenderers[i].setZOrderMediaOverlay(true); //Get the top z-order.
                        subRendererSwappedFlag[i] = true;
                    }
                    subRenderers[i].requestLayout();
                }
            }
        }

        //Add views back to layouts.
        for (int i = 0;i < maxPartyCount;++i) {
            if (subRenderers != null && subRendererLayouts != null && subRendererSwappedFlag[i]) {
                subRendererLayouts[i].addView(subRenderers[i], new ViewGroup.LayoutParams(ViewGroup.LayoutParams.WRAP_CONTENT, ViewGroup.LayoutParams.WRAP_CONTENT));
                subRenderers[i].setVisibility(View.VISIBLE);
                mainLayout.addView(subRendererLayouts[i]);
            }
        }

        if (mainRendererSwapped && mainRenderer != null) {
            mainRendererLayout.addView(mainRenderer[0], new ViewGroup.LayoutParams(ViewGroup.LayoutParams.WRAP_CONTENT, ViewGroup.LayoutParams.WRAP_CONTENT));
            mainRenderer[0].setVisibility(View.VISIBLE);
        }

        //Top most control add last.
        if (swap) {
            mainLayout.addView(callLayout);
            mainLayout.addView(hudLayout);
        }
    }

    private void start() {
		for (BeeOpenSessionParam.BeeSDKCapability capability : beeCapabilities) {
			if (capability.getSvcCode() == kBeeSvcType_VideoRoom) {
				startCall();
			}

			if (capability.getSvcCode() == kBeeSvcType_WhiteBoard) {
				//startDraw();
			}
		}
    }

    private void startCall() {
        if (beeVideoRoom == null) {
            beeVideoRoom = new BeeVideoRoom(context, handler_, TOKEN, true, allowNotWifi, beeTimeout, this);
            beeVideoRoom.setupPushStream(roomParameters.uid, mediatype, null, null, localRenderer[0], new BeeAsyncHandler(){
                @Override
                public void SetupPushStreamHandler(BeeErrorCode code) {
                    if (code == BeeErrorCode.kBeeErrorCode_Success) {
                        beeVideoRoom.join(roomParameters.roomName, roomParameters.uid, roomParameters.nickName, createRoom, BeeVideoRoomDefine.VideoRoomRole.eVideoRoomRole_Party);
                    }
                }
            });
        }
    }

    private void hangUpCall() {
        userTableClear();

        if (beeVideoRoom != null) {
            beeVideoRoom.leave();
            beeVideoRoom.dispose();
            beeVideoRoom = null;
        }

        if (beeWhiteBoard != null) {
            beeWhiteBoard.leave();
            beeWhiteBoard.dispose();
            beeWhiteBoard = null;
        }
    }

    // Disconnect from remote resources, dispose of local resources, and exit.
    private void disconnect() {
        activityRunning = false;

        if (mainRenderer != null) {
            if (mainRenderer[0] != null) {
                mainRenderer[0].release();
                mainRenderer[0] = null;
            }
            mainRenderer = null;
        }

        if (subRenderers != null) {
            for (int i = 0;i < maxPartyCount;++i) {
                if (subRenderers[i] != null) {
                    subRenderers[i].release();
                    subRenderers[i] = null;
                }
            }
            subRenderers = null;
        }

        if (iceConnected() && !isError) {
            setResult(RESULT_OK);
        } else {
            setResult(RESULT_CANCELED);
        }

        finish();
    }

    private void disconnectWithErrorMessage(final String errorMessage) {
        if (commandLineRun || !activityRunning) {
            Log.e(TAG, "Critical error: " + errorMessage);
            disconnect();
        } else {
            new AlertDialog.Builder(this)
            .setTitle(getText(R.string.channel_error_title))
            .setMessage(errorMessage)
            .setCancelable(false)
            .setNeutralButton(R.string.ok,
                new DialogInterface.OnClickListener() {
                    @Override
                    public void onClick(DialogInterface dialog, int id) {
                        dialog.cancel();
                        disconnect();
                    }
                })
            .create()
            .show();
        }
    }

    // Log |msg| and Toast about it.
    private void logAndToast(String msg) {
        Log.d(TAG, msg);
        if (logToast != null) {
            logToast.cancel();
        }
        logToast = Toast.makeText(context, msg, Toast.LENGTH_SHORT);
        logToast.show();
    }

    private void reportError(final String description) {
        runOnUiThread(new Runnable() {
        @Override
        public void run() {
            if (!isError) {
                isError = true;
                disconnectWithErrorMessage(description);
            }
        }
        });
    }

    public void onVideoRendererStart(int index) {
        final long delta = System.currentTimeMillis() - callStartedTimeMs;
        final int i = index;
        runOnUiThread(new Runnable() {
            @Override
            public void run() {
                logAndToast("ICE connected, delay=" + delta + "ms");

                if (i < 0 || i >= maxPartyCount) {
                    return;
                }

                if (remoteRenderers == null || subRenderers == null) {
                    return;
                }

                int connectedSubIndex = 0;
                for (int j = 0; j < maxPartyCount;++j) {
                    //Search for sub renderer corresponding to remote renderer index.
                    //There MUST be one sub renderer matches it, app MUST ensure this.
                    //HOW?? In original state, it can be ensured, but if a party has
                    //taken the main place already, once it leaved, app should move it
                    //back to sub place.
                    if (remoteRenderers[i] == subRenderers[j]) {
                        subRendererStartedFlag[j] = true;
                        connectedSubIndex = j;
                        break;
                    }
                }

                //If the main renderer is local renderer now, should swap theirs place.
                if (mainRenderer[0] == localRenderer[0]) {
                    swapRenderer(connectedSubIndex);
                } else {
                    updateVideoView(true);
                }

                connectedVideoCount++;
            }
        });
    }

    /**
     * onVideoRendererStop
     * @param index : Remote video renderer index.
     * @return : Sub video renderer index to stop.If RemoteVideoRenderer[index] is main renderer,
     * swap it with a started sub renderer,or just disable certain sub renderer, always return its
     * sub index to get y pos.
     */
    public int onVideoRendererStop(int index) {
        long delta = System.currentTimeMillis() - callStartedTimeMs;

        logAndToast("ICE disconnected, delay=" + delta + "ms");
        if (index < 0 || index >= maxPartyCount || mainRenderer == null) {
            return -1;
        }

        int videoRendererToStop = -1;
        //If the party in main place leaving.
        if (remoteRenderers[index] == mainRenderer[0]) {
            //Should search for a sub renderer to take the main place.
            for (int j = 0;j < maxPartyCount;++j) {
                //Do not use the local renderer to take the main place if possible.
                if (subRendererStartedFlag[j] && ((subRenderers[j] != localRenderer[0] && connectedVideoCount > 1) || connectedVideoCount == 1)) {
                    swapRenderer(j);
                    subRendererStartedFlag[j] = false;
                    videoRendererToStop = j;
                    break;
                }
            }
        } else {
            for (int j = 0;j < maxPartyCount;++j) {
                //Just disable the sub renderer.
                if (remoteRenderers[index] == subRenderers[j]) {
                    updateVideoView(true);
                    subRendererStartedFlag[j] = false;
                    videoRendererToStop = j;
                    break;
                }
            }
        }

        connectedVideoCount--;
        return videoRendererToStop;
    }

    private void initSubVideoPosition() {
        SUB_RECTS = new Rect[maxPartyCount];
        for (int i = 0;i < maxPartyCount;++i) {
            SUB_RECTS[i] = new Rect();
        }
        final int percent = 25;

        SUB_RECTS[0].left = 72;
        SUB_RECTS[0].top = 75;
        SUB_RECTS[0].right = SUB_RECTS[0].left + percent;
        SUB_RECTS[0].bottom = SUB_RECTS[0].top + percent;

        SUB_RECTS[1].left = 72;
        SUB_RECTS[1].top = 50;
        SUB_RECTS[1].right = SUB_RECTS[1].left + percent;
        SUB_RECTS[1].bottom = SUB_RECTS[1].top + percent;

        SUB_RECTS[2].left = 72;
        SUB_RECTS[2].top = 25;
        SUB_RECTS[2].right = SUB_RECTS[2].left + percent;
        SUB_RECTS[2].bottom = SUB_RECTS[2].top + percent;

        SUB_RECTS[3].left = 72;
        SUB_RECTS[3].top = 0;
        SUB_RECTS[3].right = SUB_RECTS[3].left + percent;
        SUB_RECTS[3].bottom = SUB_RECTS[3].top + percent;
    }

    private Boolean iceConnected() {
        return connectedCount > 0;
    }

    private String getFilePath(Context context) {
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

    public void onJoin(final String streamName, final BeeErrorCode error, final String msg) {
        runOnUiThread(new Runnable() {
            @Override
            public void run() {
                Logging.d(TAG, "onJoin streamName:" + streamName + " error:" + error + ".");
                if (error == BeeErrorCode.kBeeErrorCode_Success) {
                    if (!statsEnabled) {
                        //beeVideoRoom.enableStatsEvents(null,null, BeePartyType.eBeePartyType_Local, STAT_CALLBACK_PERIOD);
                        statsEnabled = true;
                    }
                } else {
                    logAndToast("Join failed " + error + " msg:" + msg + ".");
                }
            }
        });
    }

    @Override
    public void onLeave(final String streamName, final BeeErrorCode error, final String msg) {
        runOnUiThread(new Runnable() {
            @Override
            public void run() {
                Logging.d(TAG, "streamName " + streamName +" onLeave " + " error:" + error + ".");
            }
        });
    }

    @Override
    public void onMembers(final BeeVideoRoomDefine.BeeVideoRoomMemberInfo[] remoteMembers) {
        runOnUiThread(new Runnable() {
            @Override
            public void run() {
                Logging.d(TAG, "Report " + remoteMembers.length + " members.");
                for (BeeVideoRoomDefine.BeeVideoRoomMemberInfo member : remoteMembers) {
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
                        if (hasVideo(user.mediaType) && user.videoRendererWrapper != null) {
                            onVideoRendererStart(user.videoRendererWrapper.remoteIndex);
                        }
                        user.onConnected();
                    } else {
                        if (hasVideo(user.mediaType) && user.videoRendererWrapper != null) {
                            int videoRendererToStop = onVideoRendererStop(user.videoRendererWrapper.remoteIndex);
                            if (videoRendererToStop != -1) {
                                //Update y pos of renderer, so as to always get the most bottom one.
                                user.videoRendererWrapper.yPos = SUB_RECTS[videoRendererToStop].top;
                            }
                            recycleVideoRenderer(user.videoRendererWrapper);
                        }
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
                        if (hasVideo(user.mediaType) && user.videoRendererWrapper != null) {
                            int videoRendererToStop = onVideoRendererStop(user.videoRendererWrapper.remoteIndex);
                            if (videoRendererToStop != -1) {
                                //Update y pos of renderer, so as to always get the most bottom one.
                                user.videoRendererWrapper.yPos = SUB_RECTS[videoRendererToStop].top;
                            }
                            recycleVideoRenderer(user.videoRendererWrapper);
                        }

                        userTable.remove(streamName);
                    }
                }
            }
        });
    }

    @Override
    public void onAudioInputLevel(BeeErrorCode error, String msg, String streamName, int level) {

    }

    @Override
    public void onAudioOutputLevel(BeeErrorCode error, String msg, String uid, String streamName, int level) {

    }

    @Override
    public void onSlowLink(String uid, String streamName, BeeVideoRoomDefine.VideoRoomPartyType partyType, String info) {

    }

    @Override
    public boolean onNotWifiConnect() {
        boolean isAllow = false;
        do {
            if (allowNotWifi) {
                isAllow = true;
                break;
            } else {
                //ui
                reconnectLayout(true);
                notNetLayout(false);

                synchronized (allowNotWifiObject) {
                    try {
                        allowNotWifiObject.wait();
                    } catch (InterruptedException e) {
                        e.printStackTrace();
                    }

                    isAllow = allowNotWifi;
                }
            }

        } while (false);

        return isAllow;
    }

    private void createUser(BeeVideoRoomDefine.BeeVideoRoomMemberInfo memberInfo) {
        if (memberInfo == null) {
            return;
        }

        if (null != userTable.get(memberInfo.streamName)) {
            return;
        }

        User user = new User();
        user.uid = memberInfo.uid;
        user.streamName = memberInfo.streamName;
        user.nickName = memberInfo.nickName;
        user.partyType = memberInfo.partyType;
        user.mediaType = memberInfo.mediaType;
        user.role = memberInfo.role;
        user.userState = UserState.eUserState_Idle;

        if (hasVideo(user.mediaType)) {
            VideoRendererWrapper videoRenderer = applyVideoRenderer();
            if (videoRenderer != null) {
                user.videoRendererWrapper = videoRenderer;
            }
        }

        userTable.put(user.streamName, user);
        user.connect(user.mediaType);
    }

    private VideoRendererWrapper applyVideoRenderer() {
        if (idleRendererQueue.isEmpty()) {
            return null;
        } else {
            VideoRendererWrapper wrapper = idleRendererQueue.poll();
            wrapper.renderer.setVisibility(View.VISIBLE);
            return wrapper;
        }
    }

    private void recycleVideoRenderer(VideoRendererWrapper videoRenderer) {
        if (videoRenderer != null) {
            videoRenderer.renderer.setVisibility(View.GONE);
            idleRendererQueue.add(videoRenderer);
        }
    }

    private void startDraw() {
        if (beeWhiteBoard == null) {
            beeWhiteBoard = new BeeWhiteBoard(beeWhiteBoardView, handler_, TOKEN, beeTimeout, null);
            beeWhiteBoard.join(roomParameters.roomName, roomParameters.uid, roomParameters.nickName, false,
                    BeeWhiteBoardDefine.BeeWhiteBoardRole.eBeeWhiteBoardRole_Student);
        }
    }
}
