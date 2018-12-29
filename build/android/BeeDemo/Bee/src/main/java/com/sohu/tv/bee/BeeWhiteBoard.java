/**
 *  @file        BeeWhiteBoard.java
 *  @brief       BeeSDK白板类声明文件.
 *  @author      fangdali
 *  @copyright   sohu-inc
 *  @version     2.0.0.1
 */
package com.sohu.tv.bee;

import android.util.Log;
import android.view.View;

import com.sohu.tv.bee.BeeSystemDefine.BeeErrorCode;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.concurrent.ScheduledExecutorService;
import com.sohu.tv.bee.BeeWhiteBoardDefine.BeeWhiteBoardRole;
import com.sohu.tv.bee.writeBoard.action.Action;
import com.sohu.tv.bee.writeBoard.action.MyEraser;
import com.sohu.tv.bee.writeBoard.action.MyPath;
import com.sohu.tv.bee.writeBoard.action.MyText;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;
import org.webrtc.Logging;
import org.webrtc.NetworkMonitorAutoDetect;

import static com.sohu.tv.bee.BeeSDK.errorToString;
import static com.sohu.tv.bee.BeeSystemFuncion.convertARGBToRGB;
import static com.sohu.tv.bee.BeeSystemFuncion.convertRGBToARGB;
import static com.sohu.tv.bee.BeeSystemFuncion.toBeeErrorCode;
import static org.webrtc.NetworkMonitorAutoDetect.ConnectionType.CONNECTION_NONE;
import static org.webrtc.NetworkMonitorAutoDetect.ConnectionType.CONNECTION_WIFI;

/// BeeSDK白板类.
public class BeeWhiteBoard extends BeeSDKService implements BeeWhiteBoardViewDelegate {
    private final static String TAG = BeeWhiteBoardDrawView.class.getSimpleName();
    private final static ScheduledExecutorService executor = BeeSDK.getExecutor();
    private int handle = -1;
    private String token = null;
    private int timeout = 0;
    /// 白板View.
    private BeeWhiteBoardView beeWhiteBoardDrawView = null;
    private BeeWhiteBoardSink whiteBoardSink = null;
    private String roomName = null;
    private String uid = null;
    private String nick_name = null;
    private boolean creator = false;
    private BeeWhiteBoardRole role = BeeWhiteBoardRole.eBeeWhiteBoardRole_None;
    private HashMap<String, BeeWhiteBoardUser> userTable = new HashMap<>();
    private static final HashMap<String, BeeWhiteBoardDefine.BeeDrawingMode> toolMap = new HashMap<String, BeeWhiteBoardDefine.BeeDrawingMode>() {
        {
            put("pen", BeeWhiteBoardDefine.BeeDrawingMode.eBeeDrawingMode_Pen);
            put("eraser", BeeWhiteBoardDefine.BeeDrawingMode.eBeeDrawingMode_Eraser);
            put("laserpen", BeeWhiteBoardDefine.BeeDrawingMode.eBeeDrawingMode_Laser);
            put("text", BeeWhiteBoardDefine.BeeDrawingMode.eBeeDrawingMode_Text);
        }
    };

    private class BeeWhiteBoardUser {
        private String uid = null;
        private String nick_name = null;
        private BeeWhiteBoardRole role = BeeWhiteBoardRole.eBeeWhiteBoardRole_None;
        private boolean newLine = true;

        public BeeWhiteBoardUser(String uid, String nick_name, BeeWhiteBoardRole role) {
            this.uid = uid;
            this.nick_name = nick_name;
            this.role = role;
        }
    }

    /**
     *  @brief  白板类构造函数.
     *  @param  view        白板View，用于绘制白板元素.
     *  @param  handle      会话句柄.
     *  @param  token       APP令牌，每个APP必须绑定一个令牌用于鉴权.
     *  @param  timeout     白板接口超时时间，单位ms.
     *  @param  sink        白板类接口回调对象.
     *  @note   白板类对象.
     */
    public BeeWhiteBoard(View view, int handle, String token, int timeout, BeeWhiteBoardSink sink) {
        super(BeeWhiteBoardDefine.kBeeSvcType_WhiteBoard);
        this.handle = handle;
        this.token = token;
        this.timeout = timeout;
        this.beeWhiteBoardDrawView = (BeeWhiteBoardView) view;
        this.whiteBoardSink = sink;
        if (beeWhiteBoardDrawView != null) {
            beeWhiteBoardDrawView.setWhiteBoard(this);
        }
    }

    /**
     *  @brief 白板业务释放.
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
     *  @brief  加入白板房间.
     *  @param  roomName    白板房间名.
     *  @param  uid         本地用户唯一标识.
     *  @param  nick_name    本地用户昵称.
     *  @param  creator     是否是白板创建者.
     *  @param  role        白板角色.
     *  @note   结果通过BeeWhiteBoardSink::onJoin返回.
     *  @see    BeeWhiteBoardSink::onJoin.
     */
    public void join(final String roomName, final String uid, final String nick_name, final boolean creator, final BeeWhiteBoardRole role) {
        executor.execute(new Runnable() {
            @Override
            public void run() {
                Logging.d(TAG, "beeWhiteBoardInit join");
                joinInternal(roomName, uid, nick_name, creator, role);
            }
        });
    }

    /**
     *  @brief  离开白板房间.
     *  @note   结果通过BeeWhiteBoardSink::onLeave返回.
     *  @see    BeeWhiteBoardSink::onLeave.
     */
    public void leave() {
        executor.execute(new Runnable() {
            @Override
            public void run() {
                leaveInternal();
            }
        });
    }

    /**
     *  @brief  适用于非wifi网络下，重新连接白板.
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
     *  @brief  清除白板当前页屏幕.
     */
    public void clearAll() {
        executor.execute(new Runnable() {
            @Override
            public void run() {
                if (beeWhiteBoardDrawView != null) {
                    beeWhiteBoardDrawView.clearAll();
                }

                notifyClearAll();
            }
        });
    }

    /**
     * @brief   取消白板上一次绘制.
     */
    public void undo() {
        executor.execute(new Runnable() {
            @Override
            public void run() {
                if (beeWhiteBoardDrawView != null) {
                    beeWhiteBoardDrawView.undo();
                }

                notifyUndo();
            }
        });
    }

    /**
     * @brief   重做白板上一次取消的绘制.
     */
    public void redo() {
        executor.execute(new Runnable() {
            @Override
            public void run() {
                if (beeWhiteBoardDrawView != null) {
                    beeWhiteBoardDrawView.redo();
                }

                notifyRedo();
            }
        });
    }

    /**
     * @brief   设置白板本地绘制模式.
     * @param   mode        绘制模式.
     */
    public void setDrawingMode(final BeeWhiteBoardDefine.BeeDrawingMode mode) {
        executor.execute(new Runnable() {
            @Override
            public void run() {
                if (beeWhiteBoardDrawView != null) {
                    beeWhiteBoardDrawView.setDrawingMode(mode);
                }
            }
        });
    }

    /**
     * @brief   设置白板本地绘制线条颜色.
     * @param   rgbColor    线条颜色.
     */
    public void setLineColor(final int rgbColor) {
        executor.execute(new Runnable() {
            @Override
            public void run() {
                if (beeWhiteBoardDrawView != null) {
                    beeWhiteBoardDrawView.setLineColor(rgbColor);
                }
            }
        });
    }

    /**
     * @brief   设置白板本地绘制线条宽度.
     * @param   width       线条宽度.
     */
    public void setLineWidth(final int width) {
        executor.execute(new Runnable() {
            @Override
            public void run() {
                if (beeWhiteBoardDrawView != null) {
                    beeWhiteBoardDrawView.setLineWidth(width);
                }
            }
        });
    }

    /**
     * @brief   关闭白板绘制，默认打开.
     */
    public void lockDrawing() {
        executor.execute(new Runnable() {
            @Override
            public void run() {
                if (beeWhiteBoardDrawView != null) {
                    beeWhiteBoardDrawView.lockDrawing();
                }
            }
        });
    }

    /**
     * @brief   打开白板绘制，默认打开.
     */
    public void unlockDrawing() {
        executor.execute(new Runnable() {
            @Override
            public void run() {
                if (beeWhiteBoardDrawView != null) {
                    beeWhiteBoardDrawView.unlockDrawing();
                }
            }
        });
    }

    /**
     * @brief   重置白板的显示位置为初始位置.
     */
    public void resetFrame() {
        executor.execute(new Runnable() {
            @Override
            public void run() {
                if (beeWhiteBoardDrawView != null) {
                    beeWhiteBoardDrawView.resetFrame();
                }
            }
        });
    }

    public void nextPage() {
        executor.execute(new Runnable() {
            @Override
            public void run() {
                if (beeWhiteBoardDrawView != null) {
                    beeWhiteBoardDrawView.nextpage();
                }

                notifyNextPage();
            }
        });
    }

    public void prePage() {
        executor.execute(new Runnable() {
            @Override
            public void run() {
                if (beeWhiteBoardDrawView != null) {
                    beeWhiteBoardDrawView.prepage();
                }

                notifyPrePage();
            }
        });
    }

    @Override
    public void onDrawingLine(final float x0, final float y0, final float x1, final float y1, final int color, final int width, final BeeWhiteBoardDefine.BeeDrawingMode mode) {
        executor.execute(new Runnable() {
            @Override
            public void run() {
                JSONArray targets = new JSONArray();
                targets.put("all");
                String cmd = "bcast";
                String sColor = convertARGBToRGB(color);
                String type = getToolFromDrawingMode(mode);

                JSONObject jsonObject = new JSONObject();
                try {
                    jsonObject.put("cmd", "drawing");
                    jsonObject.put("color", sColor);
                    jsonObject.put("strokeWidth", width);
                    jsonObject.put("x0", x0);
                    jsonObject.put("y0", y0);
                    jsonObject.put("x1", x1);
                    jsonObject.put("y1", y1);
                    jsonObject.put("tool", type);
                    jsonObject.put("type", "line");
                } catch (JSONException e) {
                    Log.e(TAG, String.valueOf(e));
                    return;
                }

                sendDataInternal(cmd, targets, jsonObject, new BeeAsyncHandler() {
                    @Override
                    public void SendDataHandler(BeeErrorCode code) {
                        if (code != BeeErrorCode.kBeeErrorCode_Success) {
                            Logging.e(TAG, "Send onDrawingLine failed");
                        }
                    }
                });
            }
        });
    }

    @Override
    public void onDrawLineEnd() {
        executor.execute(new Runnable() {
            @Override
            public void run() {
                JSONArray targets = new JSONArray();
                targets.put("all");
                String cmd = "bcast";
                JSONObject jsonObject = new JSONObject();
                try {
                    jsonObject.put("cmd", "savedata");
                    jsonObject.put("type", "line");

                } catch (JSONException e) {
                    Log.e(TAG, String.valueOf(e));
                    return;
                }

                sendDataInternal(cmd, targets, jsonObject, new BeeAsyncHandler() {
                    @Override
                    public void SendDataHandler(BeeErrorCode code) {
                        if (code != BeeErrorCode.kBeeErrorCode_Success) {
                            Logging.e(TAG, "Send onDrawLineEnd failed");
                        }
                    }
                });
            }
        });
    }

    @Override
    public void onConnectionTypeChanged(final NetworkMonitorAutoDetect.ConnectionType connectionType) {
        //isAllowNotWifiConnection = false;
        if (connectionType != CONNECTION_NONE) {
            // 4G/wifi -> wifi (leave && reconnect)
            if (currentNetType != CONNECTION_NONE) {
                leave();
            }

            reConnect();
        } else {
            leave();
        }

        currentNetType = connectionType;

    }

    private void joinInternal(String roomName, String uid, String nick_name, boolean creator, BeeWhiteBoardRole role) {
        BeeErrorCode ret = BeeErrorCode.kBeeErrorCode_Success;
        do {
            Logging.d(TAG, "joinInternal");
            if (roomName == null || uid == null || nick_name == null || token == null) {
                ret = BeeErrorCode.kBeeErrorCode_Invalid_Param;
                break;
            }

            Logging.d(TAG, "Joining white board:" + roomName + ", creator:" + (creator?"YES":"NO") + ", uid:" + uid + ", nick name:" + nick_name + ".");
            if (false == isRegister()) {
                ret = register(handle);
                if (ret != BeeErrorCode.kBeeErrorCode_Success) {
                    Logging.e(TAG, "White board service register failed.");
                    break;
                }
            }

            this.roomName = roomName;
            this.uid = uid;
            this.nick_name = nick_name;
            this.creator = creator;
            this.role = role;

            if (!isOnline()/* || (isOnlineButNotWifi() && !isAllowNotWifiConnection)*/) {
                Logging.d(TAG, "current is not allow connection.");
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
            } catch (JSONException e) {
                Log.e(TAG, String.valueOf(e));
                return;
            }
            String args = jsonObject.toString();
            ret = execute(cmd, args, timeout);
        } while (false);

        Logging.d(TAG, "Join white board " + roomName + " return " + ret + ".");
        if (ret != BeeErrorCode.kBeeErrorCode_Success) {
            if (whiteBoardSink != null) {
                whiteBoardSink.onJoin(ret, errorToString(ret));
            }
        }
    }

    private void leaveInternal() {
        BeeErrorCode ret = BeeErrorCode.kBeeErrorCode_Success;
        do {
            Logging.d(TAG, "Leaving from white Board.");

            if (false == isRegister()) {
                Logging.e(TAG, "White Board service not register.");
                ret = BeeErrorCode.kBeeErrorCode_Service_Not_Registered;
                break;
            }

            String cmd = "Leave";
            JSONObject jsonObject = new JSONObject();
            try {
                jsonObject.put("room_name", this.roomName);
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

        Logging.d(TAG, "Leave white board " + this.roomName + " return " + ret + ".");
        if (whiteBoardSink != null) {
            whiteBoardSink.onLeave(ret, errorToString(ret));
            whiteBoardSink = null;
        }
    }

    private void reConnectInternal() {
        /*if (isOnlineButNotWifi()) {
            isAllowNotWifiConnection = true;
        }*/
        joinInternal(roomName, uid, nick_name, false, role);
    }

    private void disposeInternal() {
        super.dispose();
    }

    private void notifyClearAll() {
        JSONArray targets = new JSONArray();
        targets.put("all");
        String cmd = "bcast";
        JSONObject jsonObject = new JSONObject();
        try {
            jsonObject.put("cmd", "clear");
            jsonObject.put("type", "line");
        } catch (JSONException e) {
            Log.e(TAG, String.valueOf(e));
            return;
        }

        sendDataInternal(cmd, targets, jsonObject, new BeeAsyncHandler() {
            @Override
            public void SendDataHandler(BeeErrorCode code) {
                if (code != BeeErrorCode.kBeeErrorCode_Success) {
                    Logging.e(TAG,"Send notifyClearAll failed");
                }
            }
        });
    }

    private void notifyUndo() {
        JSONArray targets = new JSONArray();
        targets.put("all");
        String cmd = "bcast";
        JSONObject jsonObject = new JSONObject();
        try {
            jsonObject.put("cmd", "undo");
            jsonObject.put("type", "line");
        } catch (JSONException e) {
            Log.e(TAG, String.valueOf(e));
            return;
        }

        sendDataInternal(cmd, targets, jsonObject, new BeeAsyncHandler() {
            @Override
            public void SendDataHandler(BeeErrorCode code) {
                if (code != BeeErrorCode.kBeeErrorCode_Success) {
                    Logging.e(TAG,"Send notifyUndo failed");
                }
            }
        });
    }

    private void notifyRedo() {
        JSONArray targets = new JSONArray();
        targets.put("all");
        String cmd = "bcast";
        JSONObject jsonObject = new JSONObject();
        try {
            jsonObject.put("cmd", "redo");
            jsonObject.put("type", "line");
        } catch (JSONException e) {
            Log.e(TAG, String.valueOf(e));
            return;
        }

        sendDataInternal(cmd, targets, jsonObject, new BeeAsyncHandler() {
            @Override
            public void SendDataHandler(BeeErrorCode code) {
                if (code != BeeErrorCode.kBeeErrorCode_Success) {
                    Logging.e(TAG,"Send notifyRedo failed");
                }
            }
        });
    }

    public void getBoardInfo(String teacher) {
        if (teacher == null)
            return;

        String cmd = "bcast";
        JSONArray targets = new JSONArray();
        targets.put(teacher);
        JSONObject jsonObject = new JSONObject();
        try {
            jsonObject.put("cmd", "boardinfo");
            jsonObject.put("type", "line");
        } catch (JSONException e) {
            Log.e(TAG, String.valueOf(e));
            return;
        }

        sendDataInternal(cmd, targets, jsonObject, new BeeAsyncHandler() {
            @Override
            public void SendDataHandler(BeeErrorCode code) {
                if (code != BeeErrorCode.kBeeErrorCode_Success) {
                    Logging.e(TAG,"Send getBoardInfo failed");
                }
            }
        });
    }

    public void notifyNextPage() {
        JSONArray targets = new JSONArray();
        targets.put("all");
        String cmd = "bcast";
        JSONObject jsonObject = new JSONObject();
        try {
            jsonObject.put("cmd", "nextpage");
            jsonObject.put("type", "line");

        } catch (JSONException e) {
            Log.e(TAG, String.valueOf(e));
            return;
        }

        sendDataInternal(cmd, targets, jsonObject, new BeeAsyncHandler() {
            @Override
            public void SendDataHandler(BeeErrorCode code) {
                if (code != BeeErrorCode.kBeeErrorCode_Success) {
                    Logging.e(TAG,"Send notifyNextPage failed");
                }
            }
        });
    }

    public void notifyPrePage() {
        JSONArray targets = new JSONArray();
        targets.put("all");
        String cmd = "bcast";
        JSONObject jsonObject = new JSONObject();
        try {
            jsonObject.put("cmd", "prepage");
            jsonObject.put("type", "line");

        } catch (JSONException e) {
            Log.e(TAG, String.valueOf(e));
            return;
        }

        sendDataInternal(cmd, targets, jsonObject, new BeeAsyncHandler() {
            @Override
            public void SendDataHandler(BeeErrorCode code) {
                if (code != BeeErrorCode.kBeeErrorCode_Success) {
                    Logging.e(TAG,"Send notifyPrePage failed");
                }
            }
        });
    }

    private void sendDataInternal(String cmd, JSONArray targets, JSONObject data, BeeAsyncHandler handler) {
        BeeErrorCode ret = BeeErrorCode.kBeeErrorCode_Success;
        do {
            if (cmd == null || data == null || targets == null) {
                ret = BeeErrorCode.kBeeErrorCode_Invalid_Param;
                break;
            }

            if (false == isRegister()) {
                Logging.e(TAG, "White board service not registered.");
                ret = BeeErrorCode.kBeeErrorCode_Service_Not_Registered;
            }

            String cmds= "SendMessage";
            JSONObject jsonObject = new JSONObject();
            try {
                jsonObject.put("cmd", cmd);
                jsonObject.put("uid", uid);
                jsonObject.put("uidlist", targets);
                jsonObject.put("msg", data);
            } catch (JSONException e) {
                Log.e(TAG, String.valueOf(e));
                return;
            }
            String args = jsonObject.toString();

            ret = execute(cmds, args, timeout);
            if (ret != BeeErrorCode.kBeeErrorCode_Success) {
                Logging.e(TAG,"SendMessage failed with error " + ret + ".");
            }
        } while (false);

        if (handler != null) {
            handler.SendDataHandler(ret);
        }
    }

    @Override
    protected void handleData(String data) {
        if (data == null) {
            Logging.w(TAG, "White Board data null.");
            return;
        }

        String roomName = null;
        String uid = null;
        String nick_name = null;
        BeeErrorCode err = BeeErrorCode.kBeeErrorCode_Success;
        String msg = null;
        BeeWhiteBoardDefine.WhiteBoardMsgType eType = BeeWhiteBoardDefine.WhiteBoardMsgType.eWhiteBoardMsgType_Count;
        BeeWhiteBoardRole eRole = BeeWhiteBoardRole.eBeeWhiteBoardRole_None;
        JSONObject jsonObj = null;
        try {
            jsonObj = new JSONObject(data);
            if (jsonObj.has("type")) {
                int type = jsonObj.getInt("type");
                if (type >= BeeWhiteBoardDefine.WhiteBoardMsgType.eWhiteBoardMsgType_Local_Join.ordinal() && type < BeeWhiteBoardDefine.WhiteBoardMsgType.eWhiteBoardMsgType_Count.ordinal()) {
                    eType = BeeWhiteBoardDefine.WhiteBoardMsgType.values()[type];
                } else {
                    Logging.e(TAG, "Got invalid video room notify type " + type + ".");
                    return;
                }
            }

            if (jsonObj.has("ec")) {
                err = toBeeErrorCode(jsonObj.getInt("ec"));
                if (err == BeeErrorCode.kBeeErrorCode_Not_Compatible) {
                    Logging.e(TAG, "Invalid error code " + err + ".");
                    return;
                }
            }

            if (jsonObj.has("role")) {
                int ret = jsonObj.getInt("role");
                eRole = BeeWhiteBoardRole.values()[ret];
                if (eRole == BeeWhiteBoardRole.eBeeWhiteBoardRole_None) {
                    Logging.e(TAG, "Invalid white board role " + eRole + ".");
                    return;
                }
            }

            if (jsonObj.has("room_name")) {
                roomName = jsonObj.getString("room_name");
            }

            if (jsonObj.has("nick_name")) {
                nick_name = jsonObj.getString("nick_name");
            }

            if (jsonObj.has("uid")) {
                uid = jsonObj.getString("uid");
            }

            if (jsonObj.has("msg")) {
                msg = jsonObj.getString("msg");
            }
        } catch (JSONException e) {
            e.printStackTrace();
        }

        switch (eType) {
            case eWhiteBoardMsgType_Local_Join: {
                handleLocalMemberJoined(err, msg);
                break;
            }
            case eWhiteBoardMsgType_Local_Leave: {
                handleLocalMemberLeaved(err, msg);
                break;
            }
            case eWhiteBoardMsgType_Remote_Join: {
                handleRemoteMemberJoined(uid, nick_name, eRole);
                break;
            }
            case eWhiteBoardMsgType_Remote_Leave: {
                handleRemoteMemberLeaved(uid);
                break;
            }
            case eWhiteBoardMsgType_Message: {
                handleMessage(jsonObj);
            }
            default:
                break;
        }
    }

    private void handleLocalMemberJoined(final BeeErrorCode err, final String msg) {
        executor.execute(new Runnable() {
            @Override
            public void run() {
                if (whiteBoardSink != null) {
                    whiteBoardSink.onJoin(err, msg);
                }
            }
        });
    }

    private void handleLocalMemberLeaved(final BeeErrorCode err, final String msg) {
        executor.execute(new Runnable() {
            @Override
            public void run() {
                if (whiteBoardSink != null) {
                    whiteBoardSink.onLeave(err, msg);
                }
            }
        });
    }

    private void handleRemoteMemberJoined(final String uid, final String nick_name, final BeeWhiteBoardRole role) {
        executor.execute(new Runnable() {
            @Override
            public void run() {
                BeeWhiteBoardUser beeWhiteBoardUser = new BeeWhiteBoardUser(uid, nick_name, role);
                userTable.put(uid, beeWhiteBoardUser);

                if (beeWhiteBoardUser.role == BeeWhiteBoardRole.eBeeWhiteBoardRole_Teacher) {
                    getBoardInfo(beeWhiteBoardUser.uid);
                }
            }
        });
    }

    private void handleRemoteMemberLeaved(final String uid) {
        executor.execute(new Runnable() {
            @Override
            public void run() {
                if (beeWhiteBoardDrawView != null) {
                    beeWhiteBoardDrawView.remove(uid);
                }
                if (uid != null) {
                    userTable.remove(uid);
                }
            }
        });
    }

    private void handleMessage(JSONObject data) {
        String from = null;
        JSONObject msg = new JSONObject();

        do {
            if (data == null) {
                break;
            }

            try {
                if (data.has("from")) {
                    from = data.getString("from");
                } else {
                    Logging.e(TAG, "No message source present.");
                    break;
                }

                if (data.has("message")) {
                    JSONObject whiteBoardData = data.getJSONObject("message");
                    if (whiteBoardData.has("msg")) {
                        msg = whiteBoardData.getJSONObject("msg");
                    }
                } else {
                    Logging.e(TAG, "No message present.");
                    break;
                }
            } catch (JSONException e) {
                e.printStackTrace();
            }
        } while (false);

        if (from != null && msg != null) {
            handleWhiteBoardMessage(from, msg);
        }
    }

    private void handleWhiteBoardMessage(final String from, final JSONObject msg) {
        executor.execute(new Runnable() {
            @Override
            public void run() {
                String cmd = null;
                String data = null;
                try {
                    if (msg.has("cmd")) {
                        cmd = msg.getString("cmd");
                    }
                    if (msg.has("data")) {
                        data = msg.getString("data");
                    }
                } catch (JSONException e) {
                    e.printStackTrace();
                }

                BeeWhiteBoardUser user = userTable.get(from);
                if (user == null) {
                    Logging.e(TAG, "White board message received from invalid source " + from + ".");
                    return;
                }

                if (cmd.equals("drawing")) {
                    BeeWhiteBoardDefine.BeeDrawingMode mode = handleDrawing(from, msg, user.newLine);
                    if (BeeWhiteBoardDefine.BeeDrawingMode.eBeeDrawingMode_Pen == mode ||
                            BeeWhiteBoardDefine.BeeDrawingMode.eBeeDrawingMode_Eraser == mode) {
                        user.newLine = false;
                    }
                } else if (cmd.equals("savedata")) {
                    handleSaveData(from);
                    user.newLine = true;
                } else if (cmd.equals("clear")) {
                    handleClear();
                } else if (cmd.equals("undo")) {
                    handleUndo();
                } else if (cmd.equals("redo")) {
                    handleRedo();
                } else if (cmd.equals("boardinfoback")) {
                    handleBoardInfo(from, msg);
                } else if (cmd.equals("nextpage")) {
                    handleNextPage();
                } else if (cmd.equals("prepage")) {
                    handlePrePage();
                }
            }
        });
    }

    private BeeWhiteBoardDefine.BeeDrawingMode handleDrawing(String from, JSONObject msg, boolean newLine) {
        float nX0=0.1f;
        float nY0=0.1f;
        float nX1=0.1f;
        float nY1=0.1f;
        BeeWhiteBoardDefine.BeeDrawingMode mode = BeeWhiteBoardDefine.BeeDrawingMode.eBeeDrawingMode_None;

        String color = null;
        String lineWidth = null;
        String tool = null;
        String data = null;
        try {
            String x0 = msg.getString("x0");
            String y0 = msg.getString("y0");
            String x1 = msg.getString("x1");
            String y1 = msg.getString("y1");
            if (null == x0 || null == y0 || null == x1 || null == y1) {
                return mode;
            }

            if (msg.has("color")) {
                color = msg.getString("color");
            }
            if (msg.has("strokeWidth")) {
                lineWidth = msg.getString("strokeWidth");
            }
            if (msg.has("tool")) {
                tool = msg.getString("tool");
            }
            if (msg.has("data")) {
                data = msg.getString("data");
            }

            nX0 = Float.parseFloat(x0);
            nY0 = Float.parseFloat(y0);
            nX1 = Float.parseFloat(x1);
            nY1 = Float.parseFloat(y1);
            int nLineWidth = Integer.parseInt(lineWidth);
            int nColor = Integer.parseInt(color.substring(1, color.length()), 16);
            mode = getDrawingModeFromTool(tool);

            if (beeWhiteBoardDrawView != null) {
                if (mode == BeeWhiteBoardDefine.BeeDrawingMode.eBeeDrawingMode_Text) {
                    beeWhiteBoardDrawView.drawText(nX0, nY0, nColor, nLineWidth, mode, data, from);
                } else {
                    if (newLine) {
                        beeWhiteBoardDrawView.lineBegin(nX0, nY0, nColor, nLineWidth, mode, from);
                        beeWhiteBoardDrawView.lineMove(nX1, nY1, mode, from);
                    } else {
                        beeWhiteBoardDrawView.lineMove(nX1, nY1, mode, from);
                    }
                }
            }
        } catch (JSONException e) {
            Log.e(TAG, String.valueOf(e));
        }

        return mode;
    }

    private void handleSaveData(String from) {
        if (beeWhiteBoardDrawView != null) {
            beeWhiteBoardDrawView.lineEnd(from);
        }
    }

    private void handleClear() {
        if (beeWhiteBoardDrawView != null) {
            beeWhiteBoardDrawView.clearAll();
        }
    }

    private void handleUndo() {
        if (beeWhiteBoardDrawView != null) {
            beeWhiteBoardDrawView.undo();
        }
    }

    private void handleRedo() {
        if (beeWhiteBoardDrawView != null) {
            beeWhiteBoardDrawView.redo();
        }
    }

    private void handleBoardInfo(String from, JSONObject msg) {
        float pageW = 0.0f;
        float pageH = 0.0f;
        float imgW = 0.0f;
        float imgH = 0.0f;
        float imgOfferX = 0.0f;
        float imgOfferY = 0.0f;
        int nCurrentPage = 0;              //current page
        int pageCount = 0;          //all page count
        boolean isDoc = false;       //is not use ppt
        String imgBaseUrl = null;
        String imgFormat = null;
        String imgName = null;
        String pageStartId = null;
        String pageEndId = null;
        HashMap<Integer, BeeWhiteBoardPage> boardPageMap = new HashMap<>();

        if (beeWhiteBoardDrawView == null) {
            return;
        }

        try {
            JSONObject dataObject = msg.getJSONObject("data");
            //screen info
            JSONObject resInfoObject = dataObject.getJSONObject("resinfo");
            String w = resInfoObject.getString("w");
            String h = resInfoObject.getString("h");
            if (w == null || h == null) {
                return;
            }
            pageW = Float.parseFloat(w);
            pageH = Float.parseFloat(h);
            if (pageW != 0 && pageH != 0) {
                beeWhiteBoardDrawView.setOriginBoardSize(pageW, pageH);
            }

            //offer info
            JSONObject pageOfferObject = resInfoObject.getJSONObject("locinfo");
            String ox = pageOfferObject.getString("x");
            String oy = pageOfferObject.getString("y");
            String iw = pageOfferObject.getString("w");
            String ih = pageOfferObject.getString("h");
            if (ox == null || oy == null || iw == null || ih == null) {
                return;
            }
            imgOfferX = Float.parseFloat(ox);
            imgOfferY = Float.parseFloat(oy);
            imgW = Float.parseFloat(iw);
            imgH = Float.parseFloat(ih);
            beeWhiteBoardDrawView.setImgOfferValue(imgOfferX, imgOfferY, imgW, imgH);

            //page info
            nCurrentPage = dataObject.getInt("page");

            JSONObject imgObject = dataObject.getJSONObject("imgjson");
            if (imgObject != null) {
                if ("" != imgObject.optString("baseurl")) {
                    isDoc = true;
                } else {
                    isDoc = false;
                }

                if (imgObject.has("baseurl")) {
                    imgBaseUrl = imgObject.getString("baseurl");
                }

                if (imgObject.has("format")) {
                    imgFormat = imgObject.getString("format");
                }

                if (imgObject.has("name")) {
                    imgName = imgObject.getString("name");
                }

                if (imgObject.has("startidx")) {
                    pageStartId = imgObject.getString("startidx");
                }

                if (imgObject.has("endidx")) {
                    pageEndId = imgObject.getString("endidx");
                }

                if (pageStartId != null && pageEndId != null) {
                    if (Integer.parseInt(pageEndId) < Integer.parseInt(pageStartId))
                        return;
                    pageCount = Integer.parseInt(pageEndId) - Integer.parseInt(pageStartId) + 1;
                }

                JSONArray pageArray = dataObject.getJSONArray("reducerinfo");
                for (int i = 0; i < pageArray.length(); i++) {
                    String url = imgBaseUrl + imgName + "-" + i + "." + imgFormat;
                    BeeWhiteBoardPage beeWhiteBoardPage = new BeeWhiteBoardPage(from, url, pageW, pageH);
                    boardPageMap.put(i, beeWhiteBoardPage);

                    JSONObject pageObject = pageArray.getJSONObject(i);
                    if (pageObject != null) {
                        //undo
                        if (pageObject.has("past")) {
                            JSONArray undoArray = pageObject.getJSONArray("past");
                            if (undoArray != null) {
                                for (int j = 0; j < undoArray.length(); j++) {
                                    JSONObject undoObject = undoArray.getJSONObject(j);
                                    if (!undoObject.has("path")) {
                                        continue;
                                    }
                                    String data = null;
                                    String color = undoObject.getString("lc");
                                    String lineWidth = undoObject.getString("lw");
                                    if (undoObject.has("data")) {
                                        data = undoObject.getString("data");
                                    }
                                    JSONArray pathArray = undoObject.getJSONArray("path");
                                    ArrayList<Integer> pathList = new ArrayList<Integer>();
                                    if (pathArray != null) {
                                        for (int z = 0; z < pathArray.length(); z++) {
                                            pathList.add(pathArray.getInt(z));
                                        }
                                    }

                                    int nLineWidth = Integer.parseInt(lineWidth);
                                    int nColor = Integer.parseInt(color.substring(1, color.length()), 16);
                                    Action myPath = null;
                                    String tool = undoObject.getString("tool");
                                    if (tool.equals("laserpen")) {
                                        continue;//don`t save laser path
                                    } else if (tool.equals("eraser")) {
                                        if (isDoc) {
                                            myPath = new MyEraser((pathList.get(0) + imgOfferX) * beeWhiteBoardDrawView.getxZoom(), (pathList.get(1) + imgOfferY) * beeWhiteBoardDrawView.getyZoom(), convertRGBToARGB(nColor), nLineWidth);
                                        } else {
                                            myPath = new MyEraser(pathList.get(0) * beeWhiteBoardDrawView.getxZoom(), pathList.get(1) * beeWhiteBoardDrawView.getyZoom(), convertRGBToARGB(nColor), nLineWidth);
                                        }
                                    } else if (tool.equals("pen")) {
                                        if (isDoc) {
                                            myPath = new MyPath((pathList.get(0) + imgOfferX) * beeWhiteBoardDrawView.getxZoom(), (pathList.get(1) + imgOfferY) * beeWhiteBoardDrawView.getyZoom(), convertRGBToARGB(nColor), nLineWidth);
                                        } else {
                                            myPath = new MyPath(pathList.get(0) * beeWhiteBoardDrawView.getxZoom(), pathList.get(1) * beeWhiteBoardDrawView.getyZoom(), convertRGBToARGB(nColor), nLineWidth);
                                        }
                                    } else if (tool.equals("text")) {
                                        if (isDoc) {
                                            myPath = new MyText((pathList.get(0) + imgOfferX) * beeWhiteBoardDrawView.getxZoom(), (pathList.get(1) + imgOfferY) * beeWhiteBoardDrawView.getyZoom(), convertRGBToARGB(nColor), nLineWidth, data);
                                        } else {
                                            myPath = new MyText(pathList.get(0) * beeWhiteBoardDrawView.getxZoom(), pathList.get(1) * beeWhiteBoardDrawView.getyZoom(), convertRGBToARGB(nColor), nLineWidth, data);
                                        }
                                    }

                                    for (int x = 2; x < pathList.size(); x = x + 2) {
                                        if (isDoc) {
                                            myPath.onMove((pathList.get(x) + imgOfferX) * beeWhiteBoardDrawView.getxZoom(), (pathList.get(x + 1) + imgOfferY) * beeWhiteBoardDrawView.getyZoom());
                                        } else {
                                            myPath.onMove(pathList.get(x) * beeWhiteBoardDrawView.getxZoom(), pathList.get(x + 1) * beeWhiteBoardDrawView.getyZoom());
                                        }
                                    }

                                    beeWhiteBoardPage.pushUndo(myPath);
                                }
                            }
                        }

                        //redo
                        if (pageObject.has("future")) {
                            JSONArray redoArray = pageObject.getJSONArray("future");
                            if (redoArray != null) {
                                for (int j = 0; j < redoArray.length(); j++) {
                                    JSONObject redoObject = redoArray.getJSONObject(j);
                                    if (!redoObject.has("path")) {
                                        continue;
                                    }
                                    String data = null;
                                    String color = redoObject.getString("lc");
                                    String lineWidth = redoObject.getString("lw");
                                    if (redoObject.has("data")) {
                                        data = redoObject.getString("data");
                                    }
                                    JSONArray pathArray = redoObject.getJSONArray("path");
                                    ArrayList<Integer> pathList = new ArrayList<Integer>();
                                    if (pathArray != null) {
                                        for (int z = 0; z < pathArray.length(); z++) {
                                            pathList.add(pathArray.getInt(z));
                                        }
                                    }

                                    int nLineWidth = Integer.parseInt(lineWidth);
                                    int nColor = Integer.parseInt(color.substring(1, color.length()), 16);
                                    Action myAction = null;
                                    String tool = redoObject.getString("tool");
                                    if (tool.equals("text")) {
                                        if (isDoc) {
                                            myAction = new MyText((pathList.get(0) + imgOfferX) * beeWhiteBoardDrawView.getxZoom(), (pathList.get(1) + imgOfferY) * beeWhiteBoardDrawView.getyZoom(), convertRGBToARGB(nColor), nLineWidth, data);
                                        } else {
                                            myAction = new MyText(pathList.get(0) * beeWhiteBoardDrawView.getxZoom(), pathList.get(1) * beeWhiteBoardDrawView.getyZoom(), convertRGBToARGB(nColor), nLineWidth, data);
                                        }
                                    } else {
                                        if (tool.equals("laserpen")) {
                                            continue;//don`t save laser path
                                        } else if (tool.equals("eraser")) {
                                            if (isDoc) {
                                                myAction = new MyEraser((pathList.get(0) + imgOfferX) * beeWhiteBoardDrawView.getxZoom(), (pathList.get(1) + imgOfferY) * beeWhiteBoardDrawView.getyZoom(), convertRGBToARGB(nColor), nLineWidth);
                                            } else {
                                                myAction = new MyEraser(pathList.get(0) * beeWhiteBoardDrawView.getxZoom(), pathList.get(1) * beeWhiteBoardDrawView.getyZoom(), convertRGBToARGB(nColor), nLineWidth);
                                            }
                                        } else if (tool.equals("pen")) {
                                            if (isDoc) {
                                                myAction = new MyPath((pathList.get(0) + imgOfferX) * beeWhiteBoardDrawView.getxZoom(), (pathList.get(1) + imgOfferY) * beeWhiteBoardDrawView.getyZoom(), convertRGBToARGB(nColor), nLineWidth);
                                            } else {
                                                myAction = new MyPath(pathList.get(0) * beeWhiteBoardDrawView.getxZoom(), pathList.get(1) * beeWhiteBoardDrawView.getyZoom(), convertRGBToARGB(nColor), nLineWidth);
                                            }
                                        }

                                        for (int x = 2; x < pathList.size(); x = x + 2) {
                                            if (isDoc) {
                                                myAction.onMove((pathList.get(x) + imgOfferX) * beeWhiteBoardDrawView.getxZoom(), (pathList.get(x + 1) + imgOfferY) * beeWhiteBoardDrawView.getyZoom());
                                            } else {
                                                myAction.onMove(pathList.get(x) * beeWhiteBoardDrawView.getxZoom(), pathList.get(x + 1) * beeWhiteBoardDrawView.getyZoom());
                                            }
                                        }
                                    }

                                    beeWhiteBoardPage.pushRedo(myAction);
                                }
                                beeWhiteBoardPage.reverseRedoStack();
                            }
                        }
                    }
                }
            } else {
                Log.w(TAG, "imgjson is null.");
            }
        } catch (JSONException e) {
            Log.e(TAG, String.valueOf(e));
        }

        beeWhiteBoardDrawView.setBoardInfo(isDoc, boardPageMap, nCurrentPage);
    }

    private void handleNextPage() {
        if (beeWhiteBoardDrawView != null) {
            beeWhiteBoardDrawView.nextpage();
        }
    }

    private void handlePrePage() {
        if (beeWhiteBoardDrawView != null) {
            beeWhiteBoardDrawView.prepage();
        }
    }

    private static BeeWhiteBoardDefine.BeeDrawingMode getDrawingModeFromTool(String tool) {
        BeeWhiteBoardDefine.BeeDrawingMode mode = BeeWhiteBoardDefine.BeeDrawingMode.eBeeDrawingMode_None;
        do {
            mode = toolMap.get(tool);
            if (mode == null) {
                break;
            }
        } while(false);

        return mode;
    }

    private static String getToolFromDrawingMode(BeeWhiteBoardDefine.BeeDrawingMode mode) {
        String tool = null;
        if (mode == BeeWhiteBoardDefine.BeeDrawingMode.eBeeDrawingMode_Pen) {
            tool = "pen";
        } else if (mode == BeeWhiteBoardDefine.BeeDrawingMode.eBeeDrawingMode_Eraser) {
            tool = "eraser";
        } else if (mode == BeeWhiteBoardDefine.BeeDrawingMode.eBeeDrawingMode_Laser) {
            tool = "laserpen";
        } else if (mode == BeeWhiteBoardDefine.BeeDrawingMode.eBeeDrawingMode_Text) {
            tool = "text";
        }

        return tool;
    }
}
