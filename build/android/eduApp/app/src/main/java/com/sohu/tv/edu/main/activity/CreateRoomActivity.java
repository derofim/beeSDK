package com.sohu.tv.edu.main.activity;

import android.Manifest;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.pm.PackageManager;
import android.graphics.Color;
import android.os.Build;
import android.preference.PreferenceManager;
import android.support.annotation.NonNull;
import android.support.v4.app.ActivityCompat;
import android.support.v4.content.ContextCompat;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.EditText;
import android.widget.ListView;
import android.widget.RelativeLayout;
import android.widget.TextView;
import android.widget.Toast;

import com.sohu.tv.bee.BeeAsyncHandler;
import com.sohu.tv.bee.BeeOpenSessionParam;
import com.sohu.tv.bee.BeeSDK;
import com.sohu.tv.bee.BeeSystemDefine;
import com.sohu.tv.bee.BeeSystemParam;
import com.sohu.tv.edu.R;

import java.util.ArrayList;
import java.util.List;
import java.util.Vector;

import com.sohu.tv.edu.base.ui.MyToolbar;
import com.sohu.tv.edu.base.ui.popmenu.DropPopMenu;
import com.sohu.tv.edu.base.ui.popmenu.MenuItem;

import org.json.JSONArray;
import org.json.JSONException;

import butterknife.Bind;
import butterknife.ButterKnife;

public class CreateRoomActivity extends AppCompatActivity implements AdapterView.OnItemClickListener {
    private static final String TAG = CreateRoomActivity.class.getSimpleName();
    @Bind(R.id.histroy_listview)
    ListView histroyListView;
    @Bind(R.id.warning_layout)
    RelativeLayout warning_layout;
    @Bind(R.id.room_name_et)
    EditText roomNameEditText;
    @Bind(R.id.uname_et)
    EditText unameEditText;
    @Bind(R.id.createRoom_btn)
    Button CreateRoomBtn;
    @Bind(R.id.addRoom_btn)
    Button AddRoomBtn;
    @Bind(R.id.my_toolbar)
    MyToolbar myToolbar;
    @Bind(R.id.deletehistroy_btn)
    TextView DeleteAllRoom;

    private final static String EXTRA_UNAME = "UNAME";
    private final static String EXTRA_ROOM_NAME = "ROOM_NAME";
    private final static String EXTRA_CREATE_MODE = "EXTRA_MODE";
    private final static String CREATE_ROOM_SUCCESS_FLAG = "CREATE_ROOM_SUCCESS_FLAG";
    private final static String CREATE_ROOM_ERROR_FLAG = "CREATE_ROOM_ERROR_FLAG";
    private final static int CREATEROOM_REQUEST_CODE = 0;
    private final static int ABOUT_REQUEST_CODE = 1;
    private final static int SETTING_REQUEST_CODE = 2;
    private final static String keyprefRoom = "room_preference";
    private final static String keyprefRoomList = "room_list_preference";

    private ArrayList<String> roomList;
    private ArrayAdapter<String> histroyAdapter;
    private SharedPreferences roomPreferences;
    private SharedPreferences.Editor roomPreferences_editor;

    private BeeSystemParam bee_param_ = new BeeSystemParam();
    private boolean beeInitialized = false;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_create_room);
        ButterKnife.bind(this);

        activityInit();
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        beeSdkUnit();
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        super.onActivityResult(requestCode, resultCode, data);
        if (resultCode == RESULT_OK && requestCode == CREATEROOM_REQUEST_CODE) {
            warning_layout.setVisibility(View.GONE);
            Bundle bundle = data.getExtras();
            if (bundle != null) {
                String roomName = bundle.getString(CREATE_ROOM_SUCCESS_FLAG);
                if (roomName != null) {
                    if (!roomList.contains(roomName)) {
                        histroyAdapter.add(roomName);
                        histroyAdapter.notifyDataSetChanged();

                        String roomListJson = new JSONArray(roomList).toString();
                        SharedPreferences.Editor editor = roomPreferences.edit();
                        editor.putString(keyprefRoom, roomName);
                        editor.putString(keyprefRoomList, roomListJson);
                        editor.commit();
                    }
                }

                String error = bundle.getString(CREATE_ROOM_ERROR_FLAG);
                if (error != null) {
                    Toast.makeText(this, error, Toast.LENGTH_LONG).show();
                }
            }
        }
    }

    private void activityInit() {
        roomPreferences = PreferenceManager.getDefaultSharedPreferences(this);
        roomList = new ArrayList<String>();
        String roomName = roomPreferences.getString(keyprefRoom, "");
        String roomListJson = roomPreferences.getString(keyprefRoomList, null);
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
        histroyAdapter = new ArrayAdapter<String>(this, R.layout.create_room_histroy_items, roomList);
        histroyListView.setAdapter(histroyAdapter);
        histroyListView.setOnItemClickListener(this);

        warning_layout.setVisibility(View.GONE);
        roomNameEditText.setText(roomName);
        CreateRoomBtn.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                String roomName = roomNameEditText.getText().toString();
                String uname = unameEditText.getText().toString();
                if (true == checkInputAuthorized()) {
                    if (beeInitialized) {
                        ChatRoom(true, roomName, uname);
                    } else {
                        Toast.makeText(CreateRoomActivity.this, "bee sdk init failed.", Toast.LENGTH_SHORT).show();
                    }
                } else {
                    warning_layout.setVisibility(View.VISIBLE);
                }
            }
        });
        AddRoomBtn.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                String roomName = roomNameEditText.getText().toString();
                String uname = unameEditText.getText().toString();
                if (true == checkInputAuthorized()) {
                    if (beeInitialized) {
                        ChatRoom(false, roomName, uname);
                    } else {
                        Toast.makeText(CreateRoomActivity.this, "bee sdk init failed", Toast.LENGTH_SHORT).show();
                    }
                } else {
                    warning_layout.setVisibility(View.VISIBLE);
                }
            }
        });
        myToolbar.setRightTitleClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                ShowPopMenu(view);
            }
        });
        DeleteAllRoom.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                clearRoomList();
            }
        });

        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) {
            if (false == requestPermissions()) {
                beeSdkInit();
            }
        } else {
            beeSdkInit();
        }
    }

    private void beeSdkInit() {
        String logPath = "";//intent.getStringExtra(EXTRA_LOG_PATH);
        boolean enableStatusd = false;//intent.getBooleanExtra(EXTRA_ENABLE_STATUSD, true);
        String deviceId = "0123456789";//intent.getStringExtra(EXTRA_DEVICE_ID);
        boolean hwEncoder = true;
        boolean hwDecoder = true;
        initBeeParam(logPath, enableStatusd, deviceId, hwEncoder, hwDecoder, bee_param_);

        BeeSDK.sharedInstance().init(getApplicationContext(), bee_param_, 10000, null, new BeeAsyncHandler() {
            @Override
            public void InitHandler(BeeSystemDefine.BeeErrorCode ec) {
                if (ec != BeeSystemDefine.BeeErrorCode.kBeeErrorCode_Success) {
                    Log.d(TAG, "bee sdk init failed, ret = " + ec + ".");
                    return;
                }

                beeInitialized = true;
            }
        });
    }

    private void beeSdkUnit() {
        if (false == beeInitialized) {
            return;
        }

        beeInitialized = false;
        BeeSDK.sharedInstance().unit(new BeeAsyncHandler() {
            @Override
            public void UnInitHandler(BeeSystemDefine.BeeErrorCode ec) {
                if (ec != BeeSystemDefine.BeeErrorCode.kBeeErrorCode_Success) {
                    Log.d(TAG, "bee sdk unit failed, ret = " + ec + ".");
                    return;
                }
            }
        });
    }

    //bee sdk
    private void initBeeParam(String logPath, boolean enableStatusd, String deviceId, boolean hwEncoder, boolean hwDecoder, BeeSystemParam param) {
        param.platform_type     = BeeSystemDefine.BeePlatformType.kPlatformType_PC.ordinal();
        param.app_name          = "eduApp";
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
        param.enable_video_encoder_hw = hwEncoder;
        param.enable_video_decoder_hw = hwDecoder;
    }


    private void ShowPopMenu(View view) {
        DropPopMenu dropPopMenu = new DropPopMenu(this);
        dropPopMenu.setTriangleIndicatorViewColor(Color.WHITE);
        dropPopMenu.setBackgroundResource(R.drawable.bg_drop_pop_menu_white_shap);
        dropPopMenu.setItemTextColor(Color.BLACK);
        dropPopMenu.setIsShowIcon(true);

        dropPopMenu.setOnItemClickListener(new DropPopMenu.OnItemClickListener() {
            @Override
            public void onItemClick(AdapterView<?> adapterView, View view, int position, long id, MenuItem menuItem) {
                switch (position) {
                    case 0: {

                    }
                    break;
                    case 1: {
                        Intent settingIntent = new Intent(CreateRoomActivity.this, SettingActivity.class);
                        startActivityForResult(settingIntent, SETTING_REQUEST_CODE);
                    }
                    break;
                    case 2: {
                        Intent aboutIntent = new Intent(CreateRoomActivity.this, AboutActivity.class);
                        startActivityForResult(aboutIntent, ABOUT_REQUEST_CODE);
                    }
                    break;
                    case 3: {
                        finish();
                    }
                    break;
                    default:
                        break;
                }
                //Toast.makeText(view.getContext(), "点击了 " +menuItem.getItemTitle(), Toast.LENGTH_SHORT).show();
            }
        });
        dropPopMenu.setMenuList(getIconMenuList());

        dropPopMenu.show(view);
    }

    private List<MenuItem> getIconMenuList() {
        List<MenuItem> list = new ArrayList<>();
        list.add(new MenuItem(1, R.drawable.ic_create_room_upload,  "上传日志"));
        list.add(new MenuItem(2, R.drawable.ic_create_room_setup, "设置"));
        list.add(new MenuItem(3, R.drawable.ic_create_room_about, "关于"));
        list.add(new MenuItem(4, R.drawable.ic_create_room_exit, "退出"));
        return list;
    }

    private void ChatRoom(boolean bCreate, String roomName, String uname) {
        Intent chatRoomIntent = new Intent(CreateRoomActivity.this, ClassRoomActivity.class);
        chatRoomIntent.addFlags(Intent.FLAG_ACTIVITY_CLEAR_TOP);
        chatRoomIntent.putExtra(EXTRA_ROOM_NAME, roomName);
        chatRoomIntent.putExtra(EXTRA_UNAME, uname);
        chatRoomIntent.putExtra(EXTRA_CREATE_MODE, bCreate);
        startActivityForResult(chatRoomIntent, CREATEROOM_REQUEST_CODE);
    }

    @Override
    public void onItemClick(AdapterView<?> adapterView, View view, int i, long l) {
        String roomName = histroyListView.getItemAtPosition(i).toString();
        String uname = unameEditText.getText().toString();
        if (true == checkInputAuthorized()) {
            ChatRoom(false, roomName, uname);
        } else {
            warning_layout.setVisibility(View.VISIBLE);
        }

    }

    private boolean checkInputAuthorized() {
        String uname = unameEditText.getText().toString();
        String roomName = roomNameEditText.getText().toString();

        if ( (uname == null || roomName == null) ||
            (uname != null && uname.isEmpty()) ||
            (roomName != null && roomName.isEmpty())) {
            return false;
        }

        return true;
    }

    private void clearRoomList() {
        SharedPreferences.Editor editor = roomPreferences.edit();
        editor.clear();
        editor.commit();
        histroyAdapter.clear();
    }

    private boolean requestPermissions() {
        boolean ret = false;
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
                    String msg = "Need permission " + permission;
                    Log.d(TAG, msg);
                    Toast.makeText(this, msg, Toast.LENGTH_SHORT).show();
                }
                notGrantedPermissionsList.add(permission);
            }
        }

        if (!notGrantedPermissionsList.isEmpty()) {
            String[] notGrantedPermissionsArray = notGrantedPermissionsList.toArray(new String[notGrantedPermissionsList.size()]);
            ActivityCompat.requestPermissions(this, notGrantedPermissionsArray, 10000);

            ret = true;
        }

        return ret;
    }

    @Override
    public void onRequestPermissionsResult(int requestCode, @NonNull String[] permissions, @NonNull int[] grantResults) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults);
        if (requestCode == 10000) {
            for (int i = 0; i < permissions.length; i++) {
                if (grantResults[i] == PackageManager.PERMISSION_GRANTED) {
                    //String msg = "Permission " + permissions[i] + "granted.";
                    //Log.d(TAG, msg);
                    //Toast.makeText(this, msg, Toast.LENGTH_SHORT).show();
                } else {
                    String msg = "Permission " + permissions[i] + "not granted.";
                    Log.d(TAG, msg);
                    Toast.makeText(this, msg, Toast.LENGTH_SHORT).show();

                    setResult(RESULT_CANCELED);
                    finish();
                    return;
                }
            }

            beeSdkInit();
        }
    }
}
