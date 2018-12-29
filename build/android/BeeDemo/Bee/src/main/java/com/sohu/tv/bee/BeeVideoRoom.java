/**
 *  @file        BeeVideoRoom.java
 *  @brief       BeeSDK视频会议声明文件.
 *  @author      fangdali
 *  @copyright   sohu-inc
 *  @version     2.0.0.1
 */
package com.sohu.tv.bee;

import android.content.Context;
import android.os.Handler;
import android.os.Looper;
import android.provider.ContactsContract;
import android.util.Log;

import com.sohu.tv.bee.BeeVideoRoomDefine.VideoRoomMediaType;
import com.sohu.tv.bee.BeeVideoRoomDefine.VideoRoomRole;
import com.sohu.tv.bee.BeeVideoRoomDefine.VideoRoomMsgType;
import com.sohu.tv.bee.BeeVideoRoomDefine.VideoRoomPartyType;
import com.sohu.tv.bee.BeeSystemDefine.BeeErrorCode;
import com.sohu.tv.bee.BeeVideoSource.BeeVideoSourceSink;
import com.sohu.tv.bee.util.AudioRecordPermissionCheckUtils;
import com.sohu.tv.bee.util.CameraPermissionCheckUtils;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;
import org.webrtc.EglBase;
import org.webrtc.Logging;
import org.webrtc.NetworkMonitor;
import org.webrtc.NetworkMonitorAutoDetect;

import java.util.HashMap;
import java.util.concurrent.ScheduledExecutorService;

import static com.sohu.tv.bee.BeeSDK.errorToString;
import static com.sohu.tv.bee.BeeSystemFuncion.toBeeErrorCode;
import static com.sohu.tv.bee.BeeVideoRoomDefine.BeeVideoRoomMemberInfo;
import static com.sohu.tv.bee.BeeVideoRoomDefine.VideoRoomPartyType.eVideoRoomPartyType_Local;
import static com.sohu.tv.bee.BeeVideoRoomDefine.VideoRoomPartyType.eVideoRoomPartyType_Remote;
import static org.webrtc.NetworkMonitorAutoDetect.ConnectionType.CONNECTION_NONE;
import static org.webrtc.NetworkMonitorAutoDetect.ConnectionType.CONNECTION_WIFI;

/// BeeSDK视频会议类
public class BeeVideoRoom extends BeeSDKService {
    private final static String TAG = BeeVideoRoom.class.getSimpleName();
    private final static ScheduledExecutorService executor = BeeSDK.getExecutor();
    private int videoWidth = 640;
    private int videoHeight = 480;
    private int videoFps = 30;
    private boolean userCamera2 = true;
    private boolean captureToTexture = true;
    private String streamName = null;
    private int mediaType = 0;
    private Context context = null;
    private Handler mainHandler = null;
    private EglBase rootEglBase = null;

    private AppRTCAudioManager audioManager = null;
    private int handle = -1;
    private String token = null;
    private String uid = null;
    private String nick_name = null;
    private String roomName = null;
    private boolean creator = false;
    private VideoRoomRole role = VideoRoomRole.eVideoRoomRole_None;
    private int timeout = 0;
    private BeeVideoRoomSink videoRoomSink = null;
    private BeeAudioSource audioSource = null;
    private BeeVideoSource videoSource = null;
    private BeeSurfaceViewRenderer beeSurfaceViewRenderer = null;
    private BeeVideoRender videoRender = null;
    private HashMap<String, BeeVideoRender> userTable = new HashMap<>();
    private boolean bCameraStop = true;
    private static Object CameraObject = new Object();
    private boolean isAutoConnect = false;

    private BeeVideoSourceSink videoSourceSink = new BeeVideoSourceSink() {
        @Override
        public void onOpen(boolean success) {
            Logging.d(TAG, "camera opened " + (success ? "success" : "failed"));
            bCameraStop = false;
        }

        @Override
        public void onClosed() {
            Logging.d(TAG, "camera closed.");
            bCameraStop = true;
            synchronized (CameraObject) {
                CameraObject.notify();
            }
        }
    };

    /**
     *  @brief  视频会议类构造函数.
     *  @param  context                 android上下文.
     *  @param  handle                  会话句柄.
     *  @param  token                   APP令牌，每个APP必须绑定一个令牌用于鉴权.
     *  @param  isAutoConnect           视频网络改变是否自动连接（设置false，则用户自己处理网络改变时的连接）.
     *  @param  isNotWifiAutoConnect    视频在非wifi下是否自动连接.
     *  @param  timeout                 视频会议的接口调用超时时间，单位ms.
     *  @param  sink                    视频会议类接口回调对象.
     *  @return 视频会议类对象.
     *  @see    BeeVideoRoomSink
     */
    public BeeVideoRoom(final Context context, int handle, String token, boolean isAutoConnect, boolean isNotWifiAutoConnect, int timeout, BeeVideoRoomSink sink) {
        super(BeeVideoRoomDefine.kBeeSvcType_VideoRoom);
        this.context = context;
        this.handle = handle;
        this.token = token;
        this.isAutoConnect = isAutoConnect;
        this.isAllowNotWifiConnection = isNotWifiAutoConnect;
        this.timeout = timeout;
        this.videoRoomSink = sink;
        this.rootEglBase = BeeSDK.sharedInstance().getRootEglBase();

        //need main thread
        mainHandler = new Handler(Looper.getMainLooper());
        mainHandler.post(new Runnable() {
            @Override
            public void run() {
                audioManager = AppRTCAudioManager.create(context);
                audioManager.start(null);
            }
        });
    }

    /**
     * @brief  视频会议释放.
     */
    @Override
    public void dispose() {
        executor.execute(new Runnable() {
            @Override
            public void run() {
                disposeInternal();
            }
        });
    }

    /**
     * @brief  判断媒体类型中是否包含视频.
     * @param  mediaType 媒体类型.
     * @return 是否包含视频.
     */
    public static boolean hasVideo(int mediaType) {
        return VideoRoomMediaType.eBeeVideoRoomMediaType_Video.getValue() == (mediaType & VideoRoomMediaType.eBeeVideoRoomMediaType_Video.getValue());
    }

    /**
     * @brief  判断媒体类型中是否包含音频.
     * @param  mediaType 媒体类型.
     * @return 是否包含音频.
     */
    public static boolean hasAudio(int mediaType) {
        return VideoRoomMediaType.eBeeVideoRoomMediaType_Audio.getValue() == (mediaType & VideoRoomMediaType.eBeeVideoRoomMediaType_Audio.getValue());
    }

    /**
     *  @brief  加入视频会议.
     *  @param  roomName    房间名.
     *  @param  uid         本地用户唯一标识.
     *  @param  nickName    本地用户昵称.
     *  @param  creator     是否是会议创建者.
     *  @param  role        会议室角色.
     *  @note   结果通过BeeVideoRoomSink::onJoin返回，并通过BeeVideoRoomSink::onMembers得到成员信息通知.
     *  @see    BeeVideoRoomSink::onJoin，BeeVideoRoomSink::onMembers.
     */
    public void join(final String roomName, final String uid, final String nickName, final boolean creator, final VideoRoomRole role) {
        executor.execute(new Runnable() {
            @Override
            public void run() {
                joinInternal(roomName, uid, nickName, creator, role);
            }
        });
    }

    /**
     *  @brief  离开视频会议.
     *  @note   结果通过BeeVideoRoomSink::onLeave返回.
     *  @see    BeeVideoRoomSink::onLeave.
     */
    public void leave() {
        executor.execute(new Runnable() {
            @Override
            public void run() {
                leaveInternal(0);
            }
        });
    }

    /**
     *  @brief  拉一路流.
     *  @param  uid             远端用户唯一标识.
     *  @param  streamName      流名.
     *  @param  mediaType       拉流的媒体类型.
     *  @param  videoRenderer   显示所拉流的视频渲染器.
     *  @note   结果通过BeeVideoRoomSink::onConnect返回.
     *  @see    BeeVideoRoomSink::onConnect.
     */
    public void connect(final String uid, final String streamName, final int mediaType, final BeeSurfaceViewRenderer videoRenderer) {
        executor.execute(new Runnable() {
            @Override
            public void run() {
                connectInternal(uid, streamName, mediaType, videoRenderer);
            }
        });
    }

    /**
     *  @brief  断一路流.
     *  @param  uid             远端用户唯一标识.
     *  @param  streamName      流名.
     *  @param  reason          原因(保留).
     *  @note   结果通过BeeVideoRoomSink::onDisConnect返回.
     *  @see    BeeVideoRoomSink::onDisConnect.
     */
    public void disconnect(final String uid, final String streamName, final int reason) {
        executor.execute(new Runnable() {
            @Override
            public void run() {
                disconnectInternal(uid, streamName, reason);
            }
        });
    }

    /**
     *  @brief  适用于非wifi网络下，重新连接一路流.
     */
    public void reConnect() {
        executor.execute(new Runnable() {
            @Override
            public void run() {
                reConnectInternal();
            }
        });
    }

    /**
     *  @brief  设置推流参数.
     *  @param  streamName      推流流名.
     *  @param  mediaType       推流媒体类型.
     *  @param  audioSource     推流音频源，设置为nil将使用内部默认音频源.
     *  @param  videoSource     推流视频源，设置为nil将使用内部默认视频源，数据采集自摄像头，默认前置摄像头.
     *  @param  videoRenderer   推流的本地渲染器.
     *  @param  handler         本方法的调用结果回调.
     */
    public void setupPushStream(final String streamName, final int mediaType, final BeeAudioSource audioSource, final BeeVideoSource videoSource, final BeeSurfaceViewRenderer videoRenderer, final BeeAsyncHandler handler) {
        executor.execute(new Runnable() {
            @Override
            public void run() {
                setupPushStreamInternal(streamName, mediaType, audioSource, videoSource, videoRenderer, handler);
            }
        });
    }

    /**
     *  @brief  获取当前推流的音频输入音量.
     *  @param  streamName      流名.
     *  @note   结果通过BeeVideoRoomSink::onAudioInputLevel返回.
     *  @see    BeeVideoRoomSink::onAudioInputLevel.
     */
    public void getAudioInputLevel(final String streamName) {
        executor.execute(new Runnable() {
            @Override
            public void run() {
                getAudioInputLevelInternal(streamName);
            }
        });
    }

    /**
     *  @brief  获取一路流的音频输出音量.
     *  @param  uid             远端用户唯一标识.
     *  @param  streamName      流名.
     *  @note   结果通过BeeVideoRoomSink::onAudioOutputLevel返回.
     *  @see    BeeVideoRoomSink::onAudioOutputLevel.
     */
    public void getAudioOutputLevel(final String uid, final String streamName) {
        executor.execute(new Runnable() {
            @Override
            public void run() {
                getAudioOutputLevelInternal(uid, streamName);
            }
        });
    }

    /**
     *  @brief  修改声音输出（保留）.
     */
    public void changeAudioRoute() {
        executor.execute(new Runnable() {
            @Override
            public void run() {

            }
        });
    }

    /**
     *  @brief  切换前后置摄像头.
     */
    public void switchCamera() {
        executor.execute(new Runnable() {
            @Override
            public void run() {
                switchCameraInternal();
            }
        });
    }

    /**
     *  @brief  使能某个流的状态获取（保留）.
     *  @param  uid             用户唯一标识.
     *  @param  streamName      流名.
     *  @param  partyType       会议成员类型.
     *  @param  period          获取状态的时间间隔，单位是ms.
     */
    public void enableStatsEvents(String uid, String streamName, VideoRoomPartyType partyType, int period) {
        executor.execute(new Runnable() {
            @Override
            public void run() {

            }
        });
    }

    /**
     *  @brief  禁用某个流的状态获取（保留）.
     *  @param  uid             用户唯一标识.
     *  @param  streamName      流名.
     *  @param  partyType       会议成员类型.
     */
    public void disableStatsEvents(String uid, String streamName, VideoRoomPartyType partyType) {
        executor.execute(new Runnable() {
            @Override
            public void run() {

            }
        });
    }

    private void joinInternal(String roomName, String uid, String nick_name, boolean creator, VideoRoomRole role) {
        BeeErrorCode ret = BeeErrorCode.kBeeErrorCode_Success;
        do {
            if (roomName == null || uid == null || nick_name == null || this.token == null) {
                ret = BeeErrorCode.kBeeErrorCode_Invalid_Param;
                break;
            }

            if (false == isRegister()) {
                ret = register(handle);
                if (ret != BeeErrorCode.kBeeErrorCode_Success) {
                    Logging.e(TAG, "VideoRoom service register failed.");
                    break;
                }
            }

            Logging.d(TAG, "Joining room:" + roomName + ", creator:" + creator + ", role:" + role + ", uid:" + uid + ", nick name:" +  nick_name);
            this.roomName = roomName;
            this.uid = uid;
            this.nick_name = nick_name;
            this.creator = creator;
            this.role = role;

            //check current net status
            if (!isOnline()) {
                ret = BeeErrorCode.kBeeErrorCode_No_Network;
                break;
            }

            if (isOnlineButNotWifi() && !isAllowNotWifiConnection) {
                Logging.d(TAG, "current is not allow connection.");
                notWifiReConnect();
                ret = BeeErrorCode.kBeeErrorCode_Network_Not_Allow;
                break;
            }

            String cmd = "Join";
            JSONObject jsonObject = new JSONObject();
            try {
                jsonObject.put("room_name", this.roomName);
                jsonObject.put("create", this.creator);
                jsonObject.put("uid", this.uid);
                jsonObject.put("nick_name", this.nick_name);
                jsonObject.put("token", this.token);
                jsonObject.put("role", this.role.ordinal());
                if (willPushVideo() || willPushAudio()) {
                    JSONObject pushObject = new JSONObject();
                    pushObject.put("stream_name", streamName+System.currentTimeMillis());

                    JSONObject audioObject = new JSONObject();
                    audioObject.put("present", willPushAudio());
                    audioObject.put("source", audioSource!=null?audioSource.getNativeBeeVideoSource():0);
                    pushObject.put("audio", audioObject);

                    JSONObject videoObject = new JSONObject();
                    videoObject.put("present", willPushVideo());
                    videoObject.put("renderer", videoRender!=null?videoRender.getNativeBeeVideoRenderer():0);
                    videoObject.put("source", videoSource!=null?videoSource.getNativeBeeVideoSource():0);
                    pushObject.put("video", videoObject);

                    jsonObject.put("push", pushObject);
                }
            } catch (JSONException e) {
                Log.e(TAG, String.valueOf(e));
                return;
            }
            String args = jsonObject.toString();
            ret = execute(cmd, args, timeout);
        } while (false);

        if (ret != BeeErrorCode.kBeeErrorCode_Success && videoRoomSink != null) {
            videoRoomSink.onJoin(streamName, ret, errorToString(ret));
        }
    }

    private void leaveInternal(int reason) {
        BeeErrorCode ret = BeeErrorCode.kBeeErrorCode_Success;
        do {

            Logging.d(TAG, "Leaving from video room.");

            if (false == isRegister()) {
                Logging.e(TAG,"VideoRoom service not registered.");
                ret = BeeErrorCode.kBeeErrorCode_Service_Not_Registered;
                break;
            }

            String cmd = "Leave";
            JSONObject jsonObject = new JSONObject();
            try {
                jsonObject.put("room_name", this.roomName);
                jsonObject.put("reason", reason); //Trick:6666 won't do any communication to server, just close locally.
            } catch (JSONException e) {
                Log.e(TAG, String.valueOf(e));
                return;
            }
            String args = jsonObject.toString();
            ret = execute(cmd, args, timeout);
        } while (false);

        if (isRegister()) {
            unRegister();
        }

        //isAllowNotWifiConnection = false;

        if (ret != BeeErrorCode.kBeeErrorCode_Success && videoRoomSink != null) {
            videoRoomSink.onLeave(streamName, ret, errorToString(ret));
        }
    }

    private void connectInternal(String uid, String streamName, int mediaType, final BeeSurfaceViewRenderer view) {
        BeeErrorCode ret = BeeErrorCode.kBeeErrorCode_Success;
        do {
            if (uid == null || streamName == null || view == null) {
                ret = BeeErrorCode.kBeeErrorCode_Invalid_Param;
                break;
            }
            Logging.d(TAG, "Connecting stream, uid:"+ uid + ", stream name:"+ streamName + ", media type: " + mediaType);

            if (false == isRegister()) {
                Logging.d(TAG, "VideoRoom service not registered.");
                ret = BeeErrorCode.kBeeErrorCode_Service_Not_Registered;
                break;
            }

            mainHandler.post(new Runnable() {
                @Override
                public void run() {
                    if (false == view.isInitialized()) {
                        view.init(rootEglBase.getEglBaseContext(), null);
                    }
                }
            });

            BeeVideoRender videoRenderer = new BeeVideoRender(view);
            ret = videoRenderer.open();
            if (ret != BeeErrorCode.kBeeErrorCode_Success) {
                break;
            }
            userTable.put(uid, videoRenderer);

            String cmd = "ConnectStream";
            JSONObject jsonObject = new JSONObject();
            try {
                jsonObject.put("room_name", this.roomName);
                jsonObject.put("uid", uid);
                jsonObject.put("stream_name", streamName);
                jsonObject.put("pull_audio", willPullAudio(mediaType));
                jsonObject.put("pull_video", willPullVideo(mediaType));
                jsonObject.put("renderer", (videoRenderer != null)?videoRenderer.getNativeBeeVideoRenderer():0);
            } catch (JSONException e) {
                Log.e(TAG, String.valueOf(e));
                return;
            }
            String args = jsonObject.toString();
            ret = execute(cmd, args, timeout);
        } while (false);

        if (ret != BeeErrorCode.kBeeErrorCode_Success && videoRoomSink != null) {
            videoRoomSink.onConnect(uid, streamName, ret, errorToString(ret));
        }
    }

    private void disconnectInternal(String uid, String streamName, int reason) {
        BeeErrorCode ret = BeeErrorCode.kBeeErrorCode_Success;
        do {
            if (uid == null || streamName == null) {
                ret = BeeErrorCode.kBeeErrorCode_Invalid_Param;
                break;
            }

            Logging.d(TAG, "Disconnecting stream, uid:" + uid + ", stream name:" + streamName + ", reason:" + reason + ".");
            if (false == isRegister()) {
                Logging.e(TAG, "VideoRoom service not registered.");
                ret = BeeErrorCode.kBeeErrorCode_Service_Not_Registered;
                break;
            }

            String cmd = "DisconnectStream";
            JSONObject jsonObject = new JSONObject();
            try {
                jsonObject.put("room_name", this.roomName);
                jsonObject.put("uid", uid);
                jsonObject.put("stream_name", streamName);
                jsonObject.put("type", eVideoRoomPartyType_Remote.ordinal());
                jsonObject.put("reason", reason);
            } catch (JSONException e) {
                Log.e(TAG, String.valueOf(e));
                return;
            }
            String args = jsonObject.toString();
            ret = execute(cmd, args, timeout);
        } while (false);

        Logging.d(TAG, "Disconnect stream, uid:" + uid + ", stream name:" + streamName + " return " + ret + ".");
        if (ret != BeeErrorCode.kBeeErrorCode_Success && videoRoomSink != null) {
            videoRoomSink.onDisConnect(uid, streamName, ret, errorToString(ret));
        }
    }

    private void setupPushStreamInternal(String streamName, int mediaType, BeeAudioSource audioSource, BeeVideoSource videoSource, final BeeSurfaceViewRenderer view,  BeeAsyncHandler beeAsyncHandler) {
        Logging.d(TAG, "camera closed setupPushStreamInternal");
        BeeErrorCode ret = BeeErrorCode.kBeeErrorCode_Success;
        do {
            if (streamName == null || view == null) {
                ret = BeeErrorCode.kBeeErrorCode_Invalid_Param;
                break;
            }

            this.streamName = streamName;
            this.mediaType = mediaType;
            this.audioSource = audioSource;
            this.videoSource = videoSource;
            this.beeSurfaceViewRenderer = view;

            mainHandler.post(new Runnable() {
                @Override
                public void run() {
                    if (false == view.isInitialized()) {
                        view.init(rootEglBase.getEglBaseContext(), null);
                    }
                }
            });

            if (this.videoRender == null) {
                this.videoRender = new BeeVideoRender(view);
                ret = this.videoRender.open();
                if (ret != BeeErrorCode.kBeeErrorCode_Success) {
                    break;
                }
            }

            if (willPushVideo()) {
                if (!CameraPermissionCheckUtils.checkCameraPermission()) {
                    Logging.e(TAG, "Permission camera is not granted");
                    ret = BeeErrorCode.kBeeErrorCode_Webrtc_Open_Video_Capture_Fail;
                    break;
                }

                if (this.videoSource == null) {
                    if (videoSource == null) {
                        this.videoSource = new BeeVideoSourceCamera(videoWidth, videoHeight, videoFps, context, rootEglBase.getEglBaseContext(), userCamera2, captureToTexture);
                    } else {
                        this.videoSource = videoSource;
                    }

                    ret = this.videoSource.open(videoSourceSink);
                    if (ret != BeeErrorCode.kBeeErrorCode_Success) {
                        break;
                    }
                }
            }

            if (willPushAudio()) {
                if (!AudioRecordPermissionCheckUtils.checkAudioRecordPermission()) {
                    Logging.e(TAG, "Permission audio record is not granted");
                    ret = BeeErrorCode.kBeeErrorCode_Webrtc_Create_Local_Stream_Fail;
                    break;
                }

                if (this.audioSource == null) {
                    if (audioSource == null) {
                        this.audioSource = new BeeAudioSource(true, true, true, true, true);
                    } else {
                        this.audioSource = audioSource;
                    }

                    ret = this.audioSource.open();
                    if (ret != BeeErrorCode.kBeeErrorCode_Success) {
                        break;
                    }
                }
            }
        } while (false);

        if (beeAsyncHandler != null) {
            beeAsyncHandler.SetupPushStreamHandler(ret);
        }
    }

    private void disposeInternal() {
        if (videoSource != null) {
            Logging.d(TAG, "videoSource dispose.");
            videoSource.dispose();
            videoSource = null;
        }

        if (audioSource != null) {
            audioSource.dispose();
            audioSource = null;
        }

        if (videoRender != null) {
            Logging.d(TAG, "videoRender dispose.");
            videoRender.dispose();
            videoRender = null;
        }

        if (videoRoomSink != null) {
            videoRoomSink = null;
        }

        mainHandler.post(new Runnable() {
            @Override
            public void run() {
                if (audioManager != null) {
                    audioManager.stop();
                    audioManager = null;
                }
            }
        });

        super.dispose();

        // wait camera stop
        Logging.d(TAG, "camera closing.");
        if (willPushVideo() && bCameraStop == false) {
            synchronized (CameraObject) {
                try {
                    CameraObject.wait(2000);
                } catch (InterruptedException e) {
                    e.printStackTrace();
                }
            }
        }
    }

    private void getAudioInputLevelInternal(String streamName) {
        BeeErrorCode ret = BeeErrorCode.kBeeErrorCode_Success;
        do {
            if (uid == null || streamName == null) {
                ret = BeeErrorCode.kBeeErrorCode_Invalid_Param;
                break;
            }

            if (false == isRegister()) {
                Logging.e(TAG, "VideoRoom service not registered.");
                ret = BeeErrorCode.kBeeErrorCode_Service_Not_Registered;
                break;
            }

            String cmd = "GetAudioInputLevel";
            JSONObject jsonObject = new JSONObject();
            try {
                jsonObject.put("room_name", this.roomName);
                jsonObject.put("uid", uid);
                jsonObject.put("stream_name", streamName);
            } catch (JSONException e) {
                Log.e(TAG, String.valueOf(e));
                return;
            }
            String args = jsonObject.toString();
            ret = execute(cmd, args, timeout);
        } while (false);

        if (ret != BeeErrorCode.kBeeErrorCode_Success && videoRoomSink != null) {
            videoRoomSink.onAudioInputLevel(ret, errorToString(ret), streamName, -1);
        }
    }

    private void getAudioOutputLevelInternal(String uid, String streamName) {
        BeeErrorCode ret = BeeErrorCode.kBeeErrorCode_Success;
        do {
            if (uid == null || streamName == null) {
                ret = BeeErrorCode.kBeeErrorCode_Invalid_Param;
                break;
            }

            if (false == isRegister()) {
                Logging.e(TAG, "VideoRoom service not registered.");
                ret = BeeErrorCode.kBeeErrorCode_Service_Not_Registered;
                break;
            }

            String cmd = "GetAudioOutputLevel";
            JSONObject jsonObject = new JSONObject();
            try {
                jsonObject.put("room_name", this.roomName);
                jsonObject.put("uid", uid);
                jsonObject.put("stream_name", streamName);
            } catch (JSONException e) {
                Log.e(TAG, String.valueOf(e));
                return;
            }
            String args = jsonObject.toString();
            ret = execute(cmd, args, timeout);
        } while (false);

        if (ret != BeeErrorCode.kBeeErrorCode_Success && videoRoomSink != null) {
            videoRoomSink.onAudioOutputLevel(ret, errorToString(ret), uid, streamName, -1);
        }
    }

    private void switchCameraInternal() {
        if (videoSource == null) {
            Logging.e(TAG, "Failed to switch camera.");
            return; // No video is sent or only one camera is available or error happened.
        }

        videoSource.switchCapture();
    }

    private void reConnectInternal() {
        joinInternal(roomName, uid, nick_name, false, role);
    }

    private boolean willPushAudio() {
        return (this.mediaType & VideoRoomMediaType.eBeeVideoRoomMediaType_Audio.getValue()) == VideoRoomMediaType.eBeeVideoRoomMediaType_Audio.getValue();
    }

    private boolean willPushVideo() {
        return (this.mediaType & VideoRoomMediaType.eBeeVideoRoomMediaType_Video.getValue()) == VideoRoomMediaType.eBeeVideoRoomMediaType_Video.getValue();
    }

    private boolean willPullAudio(int mediaType) {
        return (mediaType & VideoRoomMediaType.eBeeVideoRoomMediaType_Audio.getValue()) == VideoRoomMediaType.eBeeVideoRoomMediaType_Audio.getValue();
    }

    private boolean willPullVideo(int mediaType) {
        return (mediaType & VideoRoomMediaType.eBeeVideoRoomMediaType_Video.getValue()) == VideoRoomMediaType.eBeeVideoRoomMediaType_Video.getValue();
    }

    @Override
    public void onConnectionTypeChanged(final NetworkMonitorAutoDetect.ConnectionType connectionType) {
        if (isAutoConnect) {
            Logging.d(TAG, "Monitor--> net changed:" + currentNetType + "->" + connectionType);
            if (currentNetType != CONNECTION_NONE || connectionType == CONNECTION_NONE) {
                leave();
            }

            currentNetType = connectionType;
            if (connectionType == CONNECTION_WIFI) {
                //wifi
                reConnect();
            } else {
                //not wifi(4G/3G...)
                notWifiReConnect();
            }
        }
    }

    private void notWifiReConnect() {
        if (currentNetType != CONNECTION_NONE) {
            if (isAllowNotWifiConnection) {
                //already allow, can connect.
                reConnect();
            } else {
                //not allow, need user decide.
                new Thread(new Runnable() {
                    @Override
                    public void run() {
                        if (videoRoomSink != null) {
                            isAllowNotWifiConnection = videoRoomSink.onNotWifiConnect();
                            if (true == isAllowNotWifiConnection) {
                                reConnect();
                            }
                        }
                    }
                }).start();
            }
        }
    }

    @Override
    protected void handleData(String data) {
        if (data == null) {
            Logging.w(TAG, "Video room data null.");
            return;
        }

        try {
            String roomName = null;
            String uid = null;
            String streamName = null;
            BeeErrorCode ret = BeeErrorCode.kBeeErrorCode_Success;
            VideoRoomMsgType eType = VideoRoomMsgType.eVideoRoomMsgType_Count;
            String errMsg = "";

            JSONObject jsonObj = new JSONObject(data);
            if (jsonObj.has("type")) {
                int type = jsonObj.getInt("type");
                if (type >= VideoRoomMsgType.eVideoRoomMsgType_Local_Party_Join.ordinal() && type < VideoRoomMsgType.eVideoRoomMsgType_Count.ordinal()) {
                    eType = VideoRoomMsgType.values()[type];
                } else {
                    Logging.e(TAG, "Got invalid video room notify type " + type + ".");
                    return;
                }
            }

            if (jsonObj.has("ec")) {
                ret = toBeeErrorCode(jsonObj.getInt("ec"));
            }

            if (jsonObj.has("room_name")) {
                roomName = jsonObj.getString("room_name");
            }

            if (jsonObj.has("stream_name")) {
                streamName = jsonObj.getString("stream_name");
            }

            if (jsonObj.has("uid")) {
                uid = jsonObj.getString("uid");
            }

            if (jsonObj.has("msg")) {
                errMsg = jsonObj.getString("msg");
            }

            switch (eType) {
                case eVideoRoomMsgType_Local_Party_Join:
                    handleLocalMemberJoined(streamName, ret, errMsg);
                    break;
                case eVideoRoomMsgType_Remote_Party_Join:
                    handleRemoteMemberJoined(uid, streamName, ret, errMsg);
                    break;
                case eVideoRoomMsgType_Local_Leave:
                    handleLocalMemberLeaved(streamName, ret, errMsg);
                    break;
                case eVideoRoomMsgType_Remote_Leave:
                    handleRemoteMemberLeaved(uid, streamName, ret, errMsg);
                    break;
                case eVideoRoomMsgType_Local_Slow_Link: {
                    String slowInfo = null;
                    if (jsonObj.has("info")) {
                        slowInfo = jsonObj.getString("info");
                    }
                    handleLocalMemberSlowLink(uid, streamName, slowInfo);
                    break;
                }
                case eVideoRoomMsgType_Remote_Slow_Link: {
                    String slowInfo = null;
                    if (jsonObj.has("info")) {
                        slowInfo = jsonObj.getString("info");
                    }
                    handleRemoteMemberSlowLink(uid, streamName, slowInfo);
                    break;
                }
                case eVideoRoomMsgType_Remote_Members: {
                    JSONArray membersJsonArray = null;
                    if (jsonObj.has("members")) {
                        membersJsonArray = jsonObj.getJSONArray("members");
                    }
                    handleRemoteMembers(membersJsonArray);
                    break;
                }
                /*case eVideoRoomMsgType_Message: {
                    String from = null;
                    String message = null;
                    BeeSvcType svcType = BeeSvcType.eBeeSvcType_None;
                    if (jsonObj.has("from") && jsonObj.has("message")) {
                        from = jsonObj.getString("from");
                        int type = jsonObj.getInt("svc");
                        if (type >= BeeSvcType.eBeeSvcType_None.getValue() && type <= BeeSvcType.eBeeSvcType_Doc.getValue()) {
                            svcType = BeeSvcType.values()[type];
                        }
                        message = jsonObj.getString("message");
                    }
                    handleMessage(from, message, svcType);
                    break;
                }*/
                case eVideoRoomMsgType_Audio_Input_Level: {
                    int level = -1;
                    if (jsonObj.has("level")) {
                        level = jsonObj.getInt("level");
                    }
                    handleAudioInputLevel(ret, errorToString(ret), uid, streamName, level);
                    break;
                }
                case eVideoRoomMsgType_Audio_Output_Level: {
                    int level = -1;
                    if (jsonObj.has("level")) {
                        level = jsonObj.getInt("level");
                    }
                    handleAudioOutputLevel(ret, errorToString(ret), uid, streamName, level);
                    break;
                }
            }
        } catch (JSONException e) {
            e.printStackTrace();
        }

    }

    private void handleLocalMemberJoined(final String streamName, final BeeErrorCode error, final String msg) {
        executor.execute(new Runnable() {
            @Override
            public void run() {
                if (videoRoomSink != null) {
                    videoRoomSink.onJoin(streamName, error, msg);
                }
            }
        });
    }

    private void handleRemoteMemberJoined(final String uid, final String streamName, final BeeErrorCode error, final String msg) {
        executor.execute(new Runnable() {
            @Override
            public void run() {
                if (videoRoomSink != null) {
                    videoRoomSink.onConnect(uid, streamName, error, msg);
                }
            }
        });
    }

    private void handleLocalMemberLeaved(final String streamName, final BeeErrorCode error, final String msg) {
        executor.execute(new Runnable() {
            @Override
            public void run() {
                if (videoRoomSink != null) {
                    videoRoomSink.onLeave(streamName, error, msg);
                }
            }
        });
    }

    private void handleRemoteMemberLeaved(final String uid, final String streamName, final BeeErrorCode error, final String msg) {
        executor.execute(new Runnable() {
            @Override
            public void run() {
                if (videoRoomSink != null) {
                    videoRoomSink.onDisConnect(uid, streamName, error, msg);
                }

                if (userTable.get(uid) != null) {
                    BeeVideoRender videoRender = userTable.get(uid);
                    videoRender.dispose();
                    videoRender = null;
                    userTable.remove(uid);
                }
            }
        });
    }

    private void handleLocalMemberSlowLink(String uid, String streamName, String info) {
        if (videoRoomSink != null) {
            videoRoomSink.onSlowLink(uid, streamName, eVideoRoomPartyType_Local, info);
        }
    }

    private void handleRemoteMemberSlowLink(String uid, String streamName, String info) {
        if (videoRoomSink != null) {
            videoRoomSink.onSlowLink(uid, streamName, eVideoRoomPartyType_Remote, info);
        }
    }

    private void handleRemoteMembers(JSONArray membersJsonArray) {
        if (membersJsonArray == null) {
            return;
        }

        int count = membersJsonArray.length();
        if (count == 0) {
            return;
        }

        try {
            BeeVideoRoomMemberInfo[] members = new BeeVideoRoomMemberInfo[count];
            for (int i = 0; i < membersJsonArray.length(); ++i) {
                String uid = null;
                String nickName = null;
                String streamName = null;
                int mediaType = 0;
                VideoRoomRole role = VideoRoomRole.eVideoRoomRole_None;

                JSONObject memberObj = membersJsonArray.getJSONObject(i);
                if (memberObj == null) { //Maybe error format json.
                    break;
                }

                //uid must be present.
                if (!memberObj.has("uid")) {
                    continue;
                } else {
                    uid = memberObj.getString("uid");
                }
                if (memberObj.has("nick_name")) {
                    nickName = memberObj.getString("nick_name");
                }
                if (memberObj.has("stream_name")) {
                    streamName = memberObj.getString("stream_name");
                }
                if (memberObj.has("media_type")) {
                    mediaType = memberObj.getInt("media_type");
                }

                if (memberObj.has("role")) {
                    int ret = memberObj.getInt("role");
                    if (ret >= VideoRoomRole.eVideoRoomRole_None.ordinal() && ret <= VideoRoomRole.eVideoRoomRole_Party.ordinal()) {
                        role = VideoRoomRole.values()[ret];
                    }
                }

                members[i] = new BeeVideoRoomMemberInfo(uid, nickName, streamName, mediaType, VideoRoomPartyType.eVideoRoomPartyType_Remote, role);
            }

            if (videoRoomSink != null) {
                videoRoomSink.onMembers(members);
            }
        } catch (JSONException e) {
            e.printStackTrace();
        }
    }

    private void handleAudioInputLevel(final BeeErrorCode error, final String msg, final String uid, final String streamName, final int level) {
        executor.execute(new Runnable() {
            @Override
            public void run() {
                if (videoRoomSink != null) {
                    videoRoomSink.onAudioInputLevel(error, msg, streamName, level);
                }
            }
        });
    }

    private void handleAudioOutputLevel(final BeeErrorCode error, final String msg, final String uid, final String streamName, final int level) {
        executor.execute(new Runnable() {
            @Override
            public void run() {
                if (videoRoomSink != null) {
                    videoRoomSink.onAudioOutputLevel(error, msg, uid, streamName, level);
                }
            }
        });
    }
}
