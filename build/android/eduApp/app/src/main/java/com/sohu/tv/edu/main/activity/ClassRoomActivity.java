package com.sohu.tv.edu.main.activity;

import android.app.AlertDialog;
import android.app.Service;
import android.graphics.Color;
import android.graphics.drawable.ColorDrawable;
import android.content.DialogInterface;
import android.media.AudioManager;
import android.provider.Settings;
import android.content.Context;
import android.content.Intent;
import android.graphics.PixelFormat;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.DisplayMetrics;
import android.util.Log;
import android.view.KeyEvent;
import android.view.MotionEvent;
import android.view.View;
import android.view.WindowManager;
import android.widget.Button;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.RelativeLayout;
import android.widget.TextView;
import android.widget.Toast;
import android.widget.VideoView;

import com.sohu.tv.bee.BeeAsyncHandler;
import com.sohu.tv.bee.BeeOpenSessionParam;
import com.sohu.tv.bee.BeeSDK;
import com.sohu.tv.bee.BeeSurfaceViewRenderer;
import com.sohu.tv.bee.BeeSystemDefine.BeeErrorCode;
import com.sohu.tv.bee.BeeSystemDefine;
import com.sohu.tv.bee.BeeSystemParam;
import com.sohu.tv.bee.BeeVideoRoom;
import com.sohu.tv.bee.BeeVideoRoomDefine;
import com.sohu.tv.bee.BeeVideoRoomSink;
import com.sohu.tv.bee.BeeWhiteBoard;
import com.sohu.tv.bee.BeeWhiteBoardDefine;
import com.sohu.tv.bee.BeeWhiteBoardSink;
import com.sohu.tv.bee.BeeWhiteBoardVideoView;
import com.sohu.tv.bee.BeeWhiteBoardView;
import com.sohu.tv.bee.writeBoard.ActionTypeEnum;
import com.sohu.tv.bee.writeBoard.SupportActionType;
import com.sohu.tv.bee.writeBoard.action.MyPath;
import com.sohu.tv.edu.R;

import org.webrtc.Logging;
import org.webrtc.NetworkMonitor;
import org.webrtc.NetworkMonitorAutoDetect;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.Timer;
import java.util.Vector;

import butterknife.Bind;
import butterknife.ButterKnife;

import static com.sohu.tv.bee.BeeVideoRoomDefine.kBeeSvcType_VideoRoom;
import static com.sohu.tv.bee.BeeWhiteBoardDefine.kBeeSvcType_WhiteBoard;

public class ClassRoomActivity extends AppCompatActivity
        implements View.OnClickListener, BeeVideoRoomSink, BeeWhiteBoardSink, NetworkMonitor.NetworkObserver {
    private static final String TAG = ClassRoomActivity.class.getSimpleName();
    @Bind(R.id.back_arrow)
    ImageView back_btn;
    @Bind(R.id.all_video_layout)
    LinearLayout all_video_layout;
    @Bind(R.id.master_view)
    BeeSurfaceViewRenderer master_view;
    @Bind(R.id.first_view)
    BeeSurfaceViewRenderer first_view;
    @Bind(R.id.second_view)
    BeeSurfaceViewRenderer second_view;
    @Bind(R.id.third_view)
    BeeSurfaceViewRenderer third_view;
    @Bind(R.id.master_uname)
    TextView master_uname;
    @Bind(R.id.first_uname)
    TextView first_uname;
    @Bind(R.id.second_uname)
    TextView second_uname;
    @Bind(R.id.third_uname)
    TextView third_uname;
    @Bind(R.id.room_id)
    TextView roomName_id;

    // net layout
    @Bind(R.id.reconnect_layout)
    RelativeLayout reconnect_layout;
    @Bind(R.id.reConnect_btn)
    Button reconnect_btn;
    @Bind(R.id.no_wifi_layout)
    RelativeLayout noWifiLayout;
    @Bind(R.id.no_net_view)
    TextView noNetView;

    // 画板
    @Bind(R.id.whiteblack)
    BeeWhiteBoardView whiteBoardSurfaceView;
    // 操作
    @Bind(R.id.black_pen_btn)
    TextView black_color_select_btn;
    @Bind(R.id.rad_pen_btn)
    TextView red_color_select_btn;
    @Bind(R.id.blue_pen_btn)
    TextView blue_color_select_btn;
    @Bind(R.id.select_pen_btn)
    TextView color_select_btn;

    @Bind(R.id.line_width1_btn)
    TextView line_width1_btn;
    @Bind(R.id.line_width2_btn)
    TextView line_width2_btn;
    @Bind(R.id.line_width3_btn)
    TextView line_width3_btn;
    @Bind(R.id.reset_btn)
    TextView reset_btn;
    @Bind(R.id.eraser_btn)
    TextView eraser_btn;
    @Bind(R.id.laser_pointer_btn)
    TextView laser_btn;
    @Bind(R.id.undo_btn)
    TextView undo_btn;
    @Bind(R.id.redo_btn)
    TextView redo_btn;
    @Bind(R.id.clear_all_btn)
    TextView clear_all_btn;
    @Bind(R.id.prepage_btn)
    TextView prepage_btn;
    @Bind(R.id.nextpage_btn)
    TextView nextpage_btn;
    @Bind(R.id.showpage)
    TextView showpage_tv;
    // 提示
    @Bind(R.id.file_loading_text)
    TextView file_loading_tx;
    //画板
    @Bind(R.id.control_layout)
    RelativeLayout controlLayout;
    @Bind(R.id.select_color_layout)
    LinearLayout selectColorLayout;
    //color
    @Bind(R.id.color1_00ff00)
    TextView color1_btn;
    @Bind(R.id.color2_ff00f7)
    TextView color2_btn;
    @Bind(R.id.color3_00a2ff)
    TextView color3_btn;
    @Bind(R.id.color4_9000ff)
    TextView color4_btn;
    @Bind(R.id.color5_00fff7)
    TextView color5_btn;
    @Bind(R.id.color6_ffbb00)
    TextView color6_btn;
    @Bind(R.id.color7_e1ff00)
    TextView color7_btn;
    @Bind(R.id.color8_00ffb2)
    TextView color8_btn;

    private final static String EXTRA_UNAME = "UNAME";
    private final static String EXTRA_ROOM_NAME = "ROOM_NAME";
    private final static String EXTRA_CREATE_MODE = "EXTRA_MODE";
    private final static String CREATE_ROOM_SUCCESS_FLAG = "CREATE_ROOM_SUCCESS_FLAG";
    private final static String CREATE_ROOM_ERROR_FLAG = "CREATE_ROOM_ERROR_FLAG";

    private boolean statsEnabled = false;
    // Peer connection statistics callback period in ms.
    private static final int STAT_CALLBACK_PERIOD = 1000;

    private static final int WRITE_EXTERNAL_STORAGE_REQUEST_CODE = 1;
    private static final int CAPTURE_PERMISSION_REQUEST_CODE = 1;
    public static int screenWidth;
    public static int screenHeight;
    public static float screenDensity;
    //classroom
    private int beeHandle = -1;
    private Vector<BeeOpenSessionParam.BeeSDKCapability> beeCapabilities = new Vector<>();
    private final String beeToken = "5eb73bb4918354cee213903c3940c0e6183f289d";
    private final int beeTimeout = 10000;
    private String localNickName = null;
    private String roomName = null;
    //videoroom
    private boolean isCreateRoom = false;
    private int connectedCount = 0;
    private BeeVideoRoom beeVideoRoom = null;
    private boolean screencaptureEnabled = false;
    private static Intent mediaProjectionPermissionResultData;
    private static int mediaProjectionPermissionResultCode;
    private User localUser = new User();
    private HashMap<String, User> userTable = new HashMap<>();
    private Toast logToast;
    private Timer audioStatsTimer = null;
    private String localUid = null;     //local uid
    private VideoRendererWrapper TeacherVideoRenderer;
    private ArrayList<VideoRendererWrapper> StudentVideoRenderer = new ArrayList<VideoRendererWrapper>();

    //whiteboard
    private BeeWhiteBoard beeWhiteBoard = null;

    //network
    private NetType currentConnectionType = NetType.eNetType_NONE;
    private boolean isAllowNotWifiConnect = false;
    private Object notWifiLockObject = new Object();

    //volume
    private int kPermutation[] = {
        0, 1, 2, 3, 4, 4, 5, 5, 5, 5, 6,
        6, 6, 6, 6, 7, 7, 7, 7, 8, 8, 8,
        9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9
    };

    //User
    private enum UserState {
        eUserState_Idle,
        eUserState_Connecting,
        eUserState_Connected,
        eUserState_DisConnecting
    }

    private class User {
        private String uid;
        private String nickName;
        private String streamName;
        private String extraData;
        private int mediaType;
        private BeeVideoRoomDefine.VideoRoomPartyType partyType;
        private BeeVideoRoomDefine.VideoRoomRole role;
        private UserState userState = UserState.eUserState_Idle;
        private VideoRendererWrapper videoRendererWrapper;

        private void connect(int mediaType) {
            if (beeVideoRoom != null) {
                beeVideoRoom.connect(uid, streamName, mediaType, videoRendererWrapper==null?null:videoRendererWrapper.getRenderer());
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

    public enum VideoPosition {
        master_position(0),
        first_position(1),
        second_position(2),
        third_position(3);

        private final int value;
        VideoPosition(int value) {
            this.value = value;
        }
    }

    public class VideoRendererWrapper {
        VideoRendererWrapper(Boolean used, VideoPosition pos, TextView tv, BeeSurfaceViewRenderer renderer) {
            this.used = used;
            this.pos = pos;
            this.uname_textview = tv;
            this.renderer = renderer;
        }
        private boolean used;
        private String uname;
        private VideoPosition pos;
        private TextView uname_textview;
        private BeeSurfaceViewRenderer renderer;

        public void setUname(String uname) {
            this.uname = uname;
        }

        public void setUsed(boolean used) {
            this.used = used;
        }

        public boolean isUsed() {
            return used;
        }

        public String getUname() {
            return uname;
        }

        public VideoPosition getPos() {
            return pos;
        }

        public BeeSurfaceViewRenderer getRenderer() {
            return renderer;
        }
    }

    private enum NetType {
        eNetType_NONE,
        eNetType_NOWIFI,
        eNetType_WIFI
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_class_room);
        ButterKnife.bind(this);

        getWindow().setFormat(PixelFormat.TRANSLUCENT);
        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
        getScreenSize();
        parseIntent();
        activityInit();
        beeSdkOpenSession();
    }

    @Override
    protected void onRestart() {
        super.onRestart();
        Log.d(TAG, "---->onRestart");
        updateNetLayout(currentConnectionType);
        start();
    }

    @Override
    protected void onStop() {
        super.onStop();
        Log.d(TAG, "---->onStop");
        userTable.clear();
        revertVideoRenderer();
        whiteBoardDispose();
        videoRoomDispose();
    }

    @Override
    protected void onDestroy() {
        // renderer dispose
        videoRendererDispose();

        if (audioStatsTimer != null) {
            audioStatsTimer.cancel();
            audioStatsTimer = null;
        }

        if (logToast != null) {
            logToast.cancel();
        }

        beeSdkCloseSession();
        ButterKnife.unbind(this);
        super.onDestroy();
    }

    @Override
    public void onActivityResult(int requestCode, int resultCode, Intent data) {
        if (requestCode != CAPTURE_PERMISSION_REQUEST_CODE)
            return;
        mediaProjectionPermissionResultCode = resultCode;
        mediaProjectionPermissionResultData = data;
    }

    @Override
    public boolean dispatchTouchEvent(MotionEvent ev) {
        if (ev.getAction() == MotionEvent.ACTION_DOWN) {
            if (isHidSelectColorLayout(ev)) {
                hideSelectColorLayout();
            }
        }
        return super.dispatchTouchEvent(ev);
    }

    @Override
    public boolean onKeyDown(int keyCode, KeyEvent event) {
        switch (keyCode) {
            case KeyEvent.KEYCODE_BACK: {
                showBackDialog();
                return false;
            }
        }

        return super.onKeyDown(keyCode, event);
    }

    @Override
    public void onConnectionTypeChanged(NetworkMonitorAutoDetect.ConnectionType connectionType) {
        Logging.d(TAG, "Connection Type = " + connectionType);
        NetType netType = NetType.eNetType_NONE;
        if (connectionType != NetworkMonitorAutoDetect.ConnectionType.CONNECTION_NONE) {
            if (connectionType == NetworkMonitorAutoDetect.ConnectionType.CONNECTION_WIFI) {
                netType = NetType.eNetType_WIFI;
            } else {
                netType = NetType.eNetType_NOWIFI;
            }
        }
        updateNetLayout(netType);
        currentConnectionType = netType;

        /*userTable.clear();
        if (TeacherVideoRenderer != null) {
            TeacherVideoRenderer.setUsed(false);
        }
        if (StudentVideoRenderer != null) {
            for (VideoRendererWrapper v : StudentVideoRenderer) {
                if (v != null && (localUser != null && localUser.videoRendererWrapper != null && v != localUser.videoRendererWrapper)) {
                    v.setUsed(false);
                }
            }
        }
        UpdateVideoRenderer();*/
    }

    @Override
    public boolean onNotWifiConnect() {
        boolean isAllow = false;

        do {
            if (isAllowNotWifiConnect) {
                isAllow = true;
                break;
            }

            synchronized (notWifiLockObject) {
                try {
                    notWifiLockObject.wait();
                } catch (InterruptedException e) {
                    e.printStackTrace();
                }

                isAllow = isAllowNotWifiConnect;
            }
        } while (false);

        return isAllow;
    }

    private void parseIntent() {
        localNickName = getIntent().getStringExtra(EXTRA_UNAME);
        roomName = getIntent().getStringExtra(EXTRA_ROOM_NAME);
        isCreateRoom = getIntent().getBooleanExtra(EXTRA_CREATE_MODE, false);
    }

    private void getScreenSize() {
        DisplayMetrics curMetrics = getApplicationContext().getResources().getDisplayMetrics();
        screenWidth = curMetrics.widthPixels;
        screenHeight = curMetrics.heightPixels;
        screenDensity = curMetrics.density;
    }

    private void activityInit() {
        roomName_id.setText("房间号：" + roomName);
        back_btn.setOnClickListener(this);
        reconnect_btn.setOnClickListener(this);

        initVideoRenderer();
        initWhiteBoardView();
    }

    //net type layout
    private void updateNetLayout(final NetType netType) {
        runOnUiThread(new Runnable() {
            @Override
            public void run() {
                if (netType == NetType.eNetType_NONE) {
                    setReconnectLayout(true);
                    setNotWifiLayout(false);
                    setNotNetLayout(true);
                } else if (netType == NetType.eNetType_NOWIFI) {
                    if (false == isAllowNotWifiConnect) {
                        setReconnectLayout(true);
                        setNotWifiLayout(true);
                        setNotNetLayout(false);
                    } else {
                        setReconnectLayout(false);
                        setNotWifiLayout(false);
                        setNotNetLayout(false);
                    }
                } else if (netType == NetType.eNetType_WIFI) {
                    setReconnectLayout(false);
                    setNotWifiLayout(false);
                    setNotNetLayout(false);
                }
            }
        });
    }

    private void setReconnectLayout(boolean isShow) {
        if (isShow) {
            reconnect_layout.setVisibility(View.VISIBLE);
        } else {
            reconnect_layout.setVisibility(View.GONE);
        }
    }

    private void setNotWifiLayout(boolean isShow) {
        if (isShow) {
            noWifiLayout.setVisibility(View.VISIBLE);
        } else {
            noWifiLayout.setVisibility(View.GONE);
        }
    }

    private void setNotNetLayout(boolean isShow) {
        if (isShow) {
            noNetView.setVisibility(View.VISIBLE);
        } else {
            noNetView.setVisibility(View.GONE);
        }
    }

    private void setNotWifiConnect() {
        //update ui
        setReconnectLayout(false);
        setNotWifiLayout(false);
        setNotNetLayout(false);

        synchronized (notWifiLockObject) {
            notWifiLockObject.notifyAll();
            isAllowNotWifiConnect = true;
        }
    }

    //videoRoom layout
    private void initVideoRenderer() {
        TeacherVideoRenderer = new VideoRendererWrapper(false, VideoPosition.master_position, master_uname, master_view);
        StudentVideoRenderer.add(new VideoRendererWrapper(false, VideoPosition.first_position, first_uname, first_view));
        StudentVideoRenderer.add(new VideoRendererWrapper(false, VideoPosition.second_position, second_uname, second_view));
        StudentVideoRenderer.add(new VideoRendererWrapper(false, VideoPosition.third_position, third_uname, third_view));
    }

    private void revertVideoRenderer() {
        if (TeacherVideoRenderer != null) {
            TeacherVideoRenderer.setUsed(false);
        }
        if (StudentVideoRenderer != null) {
            for (VideoRendererWrapper v : StudentVideoRenderer) {
                if (v != null) {
                    v.setUsed(false);
                }
            }
        }
        UpdateVideoRenderer();
    }

    private void videoRendererDispose() {
        //renderer
        if (TeacherVideoRenderer != null && TeacherVideoRenderer.renderer != null) {
            TeacherVideoRenderer.renderer.release();
            TeacherVideoRenderer.renderer = null;
            TeacherVideoRenderer = null;
        }
        if (StudentVideoRenderer != null) {
            for (VideoRendererWrapper v : StudentVideoRenderer) {
                if (v != null && v.renderer != null) {
                    v.renderer.release();
                    v.renderer = null;
                    v = null;
                }
            }
            StudentVideoRenderer = null;
        }
    }

    private void ReleaseVideoRenderer(VideoRendererWrapper videoRendererWrapper) {
        if (TeacherVideoRenderer == null || StudentVideoRenderer == null) {
            return;
        }

        if (videoRendererWrapper.getPos() == TeacherVideoRenderer.getPos()) {
            TeacherVideoRenderer.setUsed(false);
        } else {
            for (VideoRendererWrapper v : StudentVideoRenderer) {
                if (v == videoRendererWrapper && v.isUsed() == true) {
                    v.setUsed(false);
                    break;
                }
            }
        }
    }

    private void UpdateVideoRenderer() {
        //teacher
        if (TeacherVideoRenderer != null) {
            if (TeacherVideoRenderer.isUsed() == true) {
                TeacherVideoRenderer.uname_textview.setText(TeacherVideoRenderer.uname);
                TeacherVideoRenderer.uname_textview.setVisibility(View.VISIBLE);
                TeacherVideoRenderer.renderer.setVisibility(View.VISIBLE);
            } else {
                TeacherVideoRenderer.uname_textview.setVisibility(View.GONE);
                TeacherVideoRenderer.renderer.setVisibility(View.GONE);
            }
        }

        //student
        if (StudentVideoRenderer != null) {
            for (VideoRendererWrapper v : StudentVideoRenderer) {
                if (v.isUsed() == true) {
                    v.uname_textview.setText(v.uname);
                    v.uname_textview.setVisibility(View.VISIBLE);
                    v.renderer.setVisibility(View.VISIBLE);
                } else {
                    v.uname_textview.setVisibility(View.GONE);
                    v.renderer.setVisibility(View.GONE);
                }
            }
        }
    }


    //white board layout
    private void initWhiteBoardView() {
        SupportActionType.getInstance().addSupportActionType(ActionTypeEnum.Path.getValue(), MyPath.class);
        setWhiteBoardListener();
        hidepage();
        pen_init();
        line_init();
        line_width1_btn.setBackgroundResource(R.drawable.line1_p);
        black_color_select_btn.setBackgroundResource(R.drawable.pen_black_p);
        file_loading_tx.setText("互动");
    }

    private void setWhiteBoardListener() {
        black_color_select_btn.setOnClickListener(this);
        red_color_select_btn.setOnClickListener(this);
        blue_color_select_btn.setOnClickListener(this);
        color_select_btn.setOnClickListener(this);

        line_width1_btn.setOnClickListener(this);
        line_width2_btn.setOnClickListener(this);
        line_width3_btn.setOnClickListener(this);

        reset_btn.setOnClickListener(this);
        eraser_btn.setOnClickListener(this);
        laser_btn.setOnClickListener(this);
        undo_btn.setOnClickListener(this);
        redo_btn.setOnClickListener(this);
        clear_all_btn.setOnClickListener(this);
        prepage_btn.setOnClickListener(this);
        nextpage_btn.setOnClickListener(this);

        color1_btn.setOnClickListener(this);
        color2_btn.setOnClickListener(this);
        color3_btn.setOnClickListener(this);
        color4_btn.setOnClickListener(this);
        color5_btn.setOnClickListener(this);
        color6_btn.setOnClickListener(this);
        color7_btn.setOnClickListener(this);
        color8_btn.setOnClickListener(this);
    }

    private void hidepage() {
        prepage_btn.setVisibility(View.INVISIBLE);
        nextpage_btn.setVisibility(View.INVISIBLE);
        showpage_tv.setVisibility(View.INVISIBLE);
    }

    private void line_init() {
        line_width1_btn.setBackgroundResource(R.drawable.line1_n);
        line_width2_btn.setBackgroundResource(R.drawable.line2_n);
        line_width3_btn.setBackgroundResource(R.drawable.line3_n);
    }

    private void pen_init() {
        black_color_select_btn.setBackgroundResource(R.drawable.pen_black_n);
        red_color_select_btn.setBackgroundResource(R.drawable.pen_red_n);
        blue_color_select_btn.setBackgroundResource(R.drawable.pen_blue_n);
        color_select_btn.setBackgroundResource(R.drawable.pen_plus);
        eraser_btn.setBackgroundResource(R.drawable.eraser_n);
        laser_btn.setBackgroundResource(R.drawable.laser_pointer_n);
    }

    private void select_color(TextView view) {
        controlLayout.setVisibility(View.VISIBLE);
        selectColorLayout.setVisibility(View.INVISIBLE);
        int color = ((ColorDrawable)view.getBackground()).getColor();
        pen_init();
        whiteBoardSurfaceView.setLineColor(color);
        whiteBoardSurfaceView.setDrawingMode(BeeWhiteBoardDefine.BeeDrawingMode.eBeeDrawingMode_Pen);
    }

    public boolean isHidSelectColorLayout(MotionEvent e) {
        int l[] = {0, 0};
        if (selectColorLayout == null) {
            return true;
        }
        selectColorLayout.getLocationInWindow(l);
        int left = l[0];
        int top = l[1];
        int bottom = top + selectColorLayout.getHeight();
        int right = left + selectColorLayout.getWidth();

        if (e.getX() > left && e.getX() < right && e.getY() >top && e.getY() < bottom) {
            return false;
        } else {
            return true;
        }
    }

    public void hideSelectColorLayout() {
        if (selectColorLayout != null && selectColorLayout.isShown()) {
            controlLayout.setVisibility(View.VISIBLE);
            selectColorLayout.setVisibility(View.INVISIBLE);
        }
    }

    public void showBackDialog() {
        final AlertDialog.Builder normalDialog = new AlertDialog.Builder(this);
        normalDialog.setIcon(R.drawable.actionbar_white_logo_icon);
        normalDialog.setTitle("注意！");
        normalDialog.setMessage("确定离开房间?");
        normalDialog.setPositiveButton("确定", new DialogInterface.OnClickListener() {
            @Override
            public void onClick(DialogInterface dialog, int which) {
                Intent intent = new Intent();
                intent.putExtra(CREATE_ROOM_SUCCESS_FLAG, roomName);
                setResult(RESULT_OK, intent);
                finish();
            }
        });
        normalDialog.setNegativeButton("取消", new DialogInterface.OnClickListener() {
            @Override
            public void onClick(DialogInterface dialog, int which) {
            }
        });
        normalDialog.show();
    }

    public void logAndToast(String msg) {
        Log.d(TAG, msg);
        if (logToast != null) {
            logToast.cancel();
        }
        logToast = Toast.makeText(this, msg, Toast.LENGTH_SHORT);
        logToast.show();
    }

    @Override
    public void onClick(View view) {
        switch (view.getId()) {
            case R.id.back_arrow:{
                showBackDialog();
            }
                break;
            case R.id.reConnect_btn:{
                setNotWifiConnect();
            }
                break;
            case R.id.black_pen_btn: {
                int nColor = 0xff000000;
                pen_init();
                black_color_select_btn.setBackgroundResource(R.drawable.pen_black_p);
                if (beeWhiteBoard != null) {
                    beeWhiteBoard.setDrawingMode(BeeWhiteBoardDefine.BeeDrawingMode.eBeeDrawingMode_Pen);
                    beeWhiteBoard.setLineColor(nColor);
                }
            }
                break;
            case R.id.rad_pen_btn: {
                int nColor = 0xffff0000;
                pen_init();
                red_color_select_btn.setBackgroundResource(R.drawable.pen_red_p);
                if (beeWhiteBoard != null) {
                    beeWhiteBoard.setDrawingMode(BeeWhiteBoardDefine.BeeDrawingMode.eBeeDrawingMode_Pen);
                    beeWhiteBoard.setLineColor(nColor);
                }
            }
                break;
            case R.id.blue_pen_btn: {
                int nColor = 0xff0000ff;
                pen_init();
                blue_color_select_btn.setBackgroundResource(R.drawable.pen_blue_p);
                if (beeWhiteBoard != null) {
                    beeWhiteBoard.setDrawingMode(BeeWhiteBoardDefine.BeeDrawingMode.eBeeDrawingMode_Pen);
                    beeWhiteBoard.setLineColor(nColor);
                }
            }
                break;
            case R.id.select_pen_btn: {
                //int nColor = 0xff00ff00;
                controlLayout.setVisibility(View.INVISIBLE);
                selectColorLayout.setVisibility(View.VISIBLE);
            }
                break;
            case R.id.line_width1_btn: {
                int nSize = 5;
                line_init();
                line_width1_btn.setBackgroundResource(R.drawable.line1_p);
                if (beeWhiteBoard != null) {
                    beeWhiteBoard.setLineWidth(nSize);
                }
            }
                break;
            case R.id.line_width2_btn: {
                int nSize = 10;
                line_init();
                line_width2_btn.setBackgroundResource(R.drawable.line2_p);
                if (beeWhiteBoard != null) {
                    beeWhiteBoard.setLineWidth(nSize);
                }
            }
                break;
            case R.id.line_width3_btn: {
                int nSize = 15;
                line_init();
                line_width3_btn.setBackgroundResource(R.drawable.line3_p);
                if (beeWhiteBoard != null) {
                    beeWhiteBoard.setLineWidth(nSize);
                }
            }
                break;
            case R.id.reset_btn: {
                if (beeWhiteBoard != null) {
                    beeWhiteBoard.resetFrame();
                }
            }
                break;
            case R.id.eraser_btn: {
                pen_init();
                eraser_btn.setBackgroundResource(R.drawable.eraser_p);
                if (beeWhiteBoard != null) {
                    beeWhiteBoard.setDrawingMode(BeeWhiteBoardDefine.BeeDrawingMode.eBeeDrawingMode_Eraser);
                }
            }
                break;
            case R.id.laser_pointer_btn: {
                pen_init();
                laser_btn.setBackgroundResource(R.drawable.laser_pointer_p);
                if (beeWhiteBoard != null) {
                    beeWhiteBoard.setDrawingMode(BeeWhiteBoardDefine.BeeDrawingMode.eBeeDrawingMode_Laser);
                }
            }
                break;
            case R.id.undo_btn: {
                if (beeWhiteBoard != null) {
                    beeWhiteBoard.undo();
                }
            }
                break;
            case R.id.redo_btn: {
                if (beeWhiteBoard != null) {
                    beeWhiteBoard.redo();
                }
            }
                break;
            case R.id.clear_all_btn: {
                if (beeWhiteBoard != null) {
                    beeWhiteBoard.clearAll();
                }
            }
                break;
            case R.id.prepage_btn: {
                if (beeWhiteBoard != null) {
                    beeWhiteBoard.prePage();
                }
            }
                break;
            case R.id.nextpage_btn: {
                if (beeWhiteBoard != null) {
                    beeWhiteBoard.nextPage();
                }
            }
                break;
            //color
            case R.id.color1_00ff00: {
                select_color(color1_btn);
                color_select_btn.setBackgroundResource(R.drawable.color1_00ff00);
            }
                break;
            case R.id.color2_ff00f7: {
                select_color(color2_btn);
                color_select_btn.setBackgroundResource(R.drawable.color2_ff00f7);
            }
                break;
            case R.id.color3_00a2ff: {
                select_color(color3_btn);
                color_select_btn.setBackgroundResource(R.drawable.color3_00a2ff);
            }
                break;
            case R.id.color4_9000ff: {
                select_color(color4_btn);
                color_select_btn.setBackgroundResource(R.drawable.color4_9000ff);
            }
                break;
            case R.id.color5_00fff7: {
                select_color(color5_btn);
                color_select_btn.setBackgroundResource(R.drawable.color5_00fff7);
            }
                break;
            case R.id.color6_ffbb00: {
                select_color(color6_btn);
                color_select_btn.setBackgroundResource(R.drawable.color6_ffbb00);
            }
                break;
            case R.id.color7_e1ff00: {
                select_color(color7_btn);
                color_select_btn.setBackgroundResource(R.drawable.color7_e1ff00);
            }
                break;
            case R.id.color8_00ffb2: {
                select_color(color8_btn);
                color_select_btn.setBackgroundResource(R.drawable.color8_00ffb2);
            }
                break;
            default:
                break;
        }
    }

    private void beeSdkOpenSession() {
        BeeSDK.sharedInstance().registerNetMonitor(this);
        BeeSDK.sharedInstance().openSession(new BeeAsyncHandler() {
            @Override
            public void OpenSessionHandler(BeeErrorCode ec, int handle, Vector<BeeOpenSessionParam.BeeSDKCapability> capabilities) {
                if (ec != BeeErrorCode.kBeeErrorCode_Success) {
                    Log.d(TAG, "Bee OpenSessionHandler fail.");
                    return;
                }

                beeHandle = handle;
                beeCapabilities = capabilities;
                start();
            }
        });
    }

    private void beeSdkCloseSession() {
        BeeSDK.sharedInstance().unRegisterNetMonitor(this);
        BeeSDK.sharedInstance().closeSession(beeHandle, new BeeAsyncHandler() {
            @Override
            public void CloseSessionHandler(BeeErrorCode ec) {
                if (ec != BeeErrorCode.kBeeErrorCode_Success) {
                    Log.d(TAG, "Bee CloseSessionHandler fail.");
                    return;
                }
            }
        });
    }

    private void start() {
        localUid = Settings.System.getString(getContentResolver(), Settings.System.ANDROID_ID);
        for (BeeOpenSessionParam.BeeSDKCapability capability : beeCapabilities) {
            if (capability.getSvcCode() == kBeeSvcType_WhiteBoard) {
                createWhiteBoard();
            }

            if (capability.getSvcCode() == kBeeSvcType_VideoRoom) {
                createVideoRoom();
            }
        }
    }

    //white board
    private void createWhiteBoard() {
        beeWhiteBoard = new BeeWhiteBoard(whiteBoardSurfaceView, beeHandle, beeToken, beeTimeout, this);
        beeWhiteBoard.join(roomName, localUid, localNickName, false, BeeWhiteBoardDefine.BeeWhiteBoardRole.eBeeWhiteBoardRole_Student);
        beeWhiteBoard.setDrawingMode(BeeWhiteBoardDefine.BeeDrawingMode.eBeeDrawingMode_Pen);
        beeWhiteBoard.setLineColor(Color.BLACK);
    }

    private void whiteBoardDispose() {
        if (beeWhiteBoard != null) {
            beeWhiteBoard.leave();
            beeWhiteBoard.dispose();
            beeWhiteBoard = null;
        }
    }

    @Override
    public void onJoin(final BeeErrorCode ret, String msg) {
        runOnUiThread(new Runnable() {
            @Override
            public void run() {
                if (ret != BeeSystemDefine.BeeErrorCode.kBeeErrorCode_Success) {
                    String error = "whiteBoard onJoin Failed ret = " + ret + ".";
                    logAndToast(error);
                }
            }
        });
    }

    @Override
    public void onLeave(final BeeErrorCode ret, final String msg) {
        runOnUiThread(new Runnable() {
            @Override
            public void run() {
                if (ret != BeeSystemDefine.BeeErrorCode.kBeeErrorCode_Success) {
                    String error = "whiteBoard onLeave Failed ret = " + ret + ".";
                    logAndToast(error);
                } else {
                    Logging.d(TAG, "ret " + ret +" onLeave " + " error:" + msg + ".");
                }
            }
        });
    }

    //video room
    private void createVideoRoom() {
        beeVideoRoom = new BeeVideoRoom(getApplicationContext(), beeHandle, beeToken, true, isAllowNotWifiConnect, beeTimeout, this);

        String streamName = localUid;
        VideoRendererWrapper videoRendererWrapper = applyVideoRenderer(localNickName, BeeVideoRoomDefine.VideoRoomRole.eVideoRoomRole_Party);

        boolean pushVideo = true;
        boolean pushAudio = true;
        boolean pushScreen = false;
        int mediatype = (pushAudio? BeeVideoRoomDefine.VideoRoomMediaType.eBeeVideoRoomMediaType_Audio.getValue():0) |
                (pushVideo? BeeVideoRoomDefine.VideoRoomMediaType.eBeeVideoRoomMediaType_Video.getValue():0);

        if (localUser != null) {
            localUser.uid = localUid;
            localUser.nickName = localNickName;
            localUser.partyType = BeeVideoRoomDefine.VideoRoomPartyType.eVideoRoomPartyType_Local;
            localUser.mediaType = mediatype;
            localUser.extraData = null;
            localUser.userState = UserState.eUserState_Idle;
            localUser.streamName = streamName;
            localUser.videoRendererWrapper = videoRendererWrapper;
        }

        beeVideoRoom.setupPushStream(localUid, mediatype, null, null, videoRendererWrapper.renderer, new BeeAsyncHandler() {
            @Override
            public void SetupPushStreamHandler(BeeErrorCode code) {
                if (code != BeeErrorCode.kBeeErrorCode_Success) {
                    logAndToast("Bee onSetupPushStream fail.");
                    return;
                }

                if (beeVideoRoom != null) {
                    beeVideoRoom.join(roomName, localUid, localNickName, false, BeeVideoRoomDefine.VideoRoomRole.eVideoRoomRole_Party);
                }
            }
        });
    }

    private void videoRoomDispose() {
        if (beeVideoRoom != null) {
            beeVideoRoom.leave();
            beeVideoRoom.dispose();
            beeVideoRoom = null;
        }
    }

    private void createUser(BeeVideoRoomDefine.BeeVideoRoomMemberInfo memberInfo) {
        if (memberInfo == null) {
            return;
        }

        User user = userTable.get(memberInfo.uid);
        if (user == null) {
            user = new User();
            user.uid = memberInfo.uid;

            userTable.put(user.uid, user);
        }

        user.nickName = memberInfo.nickName;
        user.streamName = memberInfo.streamName;
        user.mediaType = memberInfo.mediaType;
        user.partyType = memberInfo.partyType;
        user.role = memberInfo.role;
        user.userState = UserState.eUserState_Idle;
        if (beeVideoRoom.hasVideo(user.mediaType)) {
            if (user.videoRendererWrapper == null) {
                user.videoRendererWrapper = applyVideoRenderer(user.nickName, user.role);
            }
        }

        user.connect(user.mediaType);
    }

    private VideoRendererWrapper applyVideoRenderer(String uname, BeeVideoRoomDefine.VideoRoomRole role) {
        if (role == BeeVideoRoomDefine.VideoRoomRole.eVideoRoomRole_Manager) {
            if (false == TeacherVideoRenderer.isUsed()) {
                TeacherVideoRenderer.setUsed(true);
                TeacherVideoRenderer.setUname(uname);
                return TeacherVideoRenderer;
            }
        } else if (role == BeeVideoRoomDefine.VideoRoomRole.eVideoRoomRole_Party) {
            for (VideoRendererWrapper v : StudentVideoRenderer) {
                if (false == v.isUsed()) {
                    v.setUsed(true);
                    v.setUname(uname);
                    return v;
                }
            }
        }

        return null;
    }

    @Override
    public void onJoin(final String streamName, final BeeSystemDefine.BeeErrorCode error, final String msg) {
        runOnUiThread(new Runnable() {
            @Override
            public void run() {
                Logging.d(TAG, "onJoin streamName:" + streamName + " error:" + error + " msg:" + msg + ".");
                if (error == BeeErrorCode.kBeeErrorCode_Success) {
                    logAndToast("Join success.");
                    if (!statsEnabled) {
                        statsEnabled = true;
                    }
                    UpdateVideoRenderer();

                } else {
                    String err = "Join failed " + error + " msg:" + msg + ".";
                    Log.d(TAG, err);
                    /*if (false == isCreateRoom) {
                        Intent intent = new Intent();
                        intent.putExtra(CREATE_ROOM_ERROR_FLAG, err);
                        setResult(RESULT_OK, intent);
                        finish();
                    }*/
                }
            }
        });
    }

    @Override
    public void onLeave(String streamName, BeeErrorCode error, String msg) {

    }

    @Override
    public void onMembers(final BeeVideoRoomDefine.BeeVideoRoomMemberInfo[] remoteMembers) {
        runOnUiThread(new Runnable() {
            @Override
            public void run() {
                Logging.d(TAG, "Report " + remoteMembers.length + " members.");
                for (int i = 0; i < remoteMembers.length;++i) {
                    createUser(remoteMembers[i]);
                }
            }
        });
    }

    @Override
    public void onConnect(final String uid, final String streamName, final BeeErrorCode error, final String msg) {
        runOnUiThread(new Runnable() {
            @Override
            public void run() {
                Logging.d(TAG, "connect uid:" + uid + "error:" + error);
                User user = userTable.get(uid);
                if (user != null) {
                    UpdateVideoRenderer();
                }
            }
        });
    }

    @Override
    public void onDisConnect(final String uid, final String streamName, final BeeErrorCode error, final String msg) {
        runOnUiThread(new Runnable() {
            @Override
            public void run() {
                User user = userTable.get(uid);
                if (user != null) {
                    user.onDisConnected();
                    ReleaseVideoRenderer(user.videoRendererWrapper);
                    UpdateVideoRenderer();
                    userTable.remove(uid);
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
}
