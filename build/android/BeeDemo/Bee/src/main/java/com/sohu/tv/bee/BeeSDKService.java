/**
 *  @file        BeeSDKService.java
 *  @brief       BeeSDK通用业务类声明文件.
 *  @author      fangdali
 *  @copyright   sohu-inc
 *  @version     2.0.0.1
 */
package com.sohu.tv.bee;

import com.sohu.tv.bee.BeeSystemDefine.BeeErrorCode;

import org.webrtc.EglBase;
import org.webrtc.Logging;
import org.webrtc.NetworkMonitor;
import org.webrtc.NetworkMonitorAutoDetect;

import static com.sohu.tv.bee.BeeSystemFuncion.toBeeErrorCode;
import static org.webrtc.NetworkMonitorAutoDetect.ConnectionType.CONNECTION_NONE;
import static org.webrtc.NetworkMonitorAutoDetect.ConnectionType.CONNECTION_WIFI;

/// BeeSDK通用业务类，作为所有业务类的基类.
public class BeeSDKService implements NetworkMonitor.NetworkObserver{
    private final static String TAG = BeeSDKService.class.getSimpleName();
    private long nativeSDKService;
    private final int svcCode;
    private int handle = 0;
    private boolean isRegister = false;
    protected boolean isAllowNotWifiConnection = false;   //is Allow non-wifi network connections
    protected NetworkMonitorAutoDetect.ConnectionType currentNetType = NetworkMonitor.getConnectionType();

    protected enum connectionNetType {
        eConnectionNetType_NONE,
        eConnectionNetType_WIFI,
        eConnectionNetType_NOT_WIFI
    }

    /**
     *  @brief  通用业务类构造函数.
     *  @param  svcCode     业务码.
     *  @note   必须在派生类调用.
     */
    public BeeSDKService(int svcCode) {
        this.svcCode = svcCode;
        nativeSDKService = nativeCreateSDKService(svcCode);
        registerNetStatus();
    }

    /**
     * @brief 释放业务类对象.
     */
    public void dispose() {
        unRegisterNetStatus();
        if (nativeSDKService != 0) {
            nativeFreeSDKService(nativeSDKService);
            nativeSDKService = 0;
        }
    }

    /**
     *  @brief  返回是否已经注册.
     *  @return 是否已经注册.
     */
    protected boolean isRegister() {
        return isRegister;
    }

    /**
     *  @brief  注册业务到一个会话.
     *  @param  handle      会话句柄.
     *  @note   每个业务都必须注册到一个会话中才能获取业务数据.
     *  @return 错误码.
     */
    protected BeeErrorCode register(int handle) {
        BeeErrorCode ret = BeeErrorCode.kBeeErrorCode_Success;
        do {
            this.handle = handle;

            ret = toBeeErrorCode(nativeReg(nativeSDKService, handle));
            isRegister = true;
        } while (false);
        return ret;
    }

    /**
     *  @brief  解除业务到会话的注册.
     *  @return 错误码.
     */
    protected BeeErrorCode unRegister() {
        BeeErrorCode ret = BeeErrorCode.kBeeErrorCode_Success;
        do {
            ret = toBeeErrorCode(nativeUnReg(nativeSDKService));
            isRegister = false;
        } while (false);
        return ret;
    }


    /**
     *  @brief  执行一个命令.
     *  @param  command     命令名称.
     *  @param  args        命令参数.
     *  @param  timeout     执行命令的超时时间，单位ms.
     *  @note   目前该方法是异步方法，通过BeeSDKServiceDataProtocol返回数据.
     *  @return 错误码.
     */
    protected BeeErrorCode execute(String command, String args, int timeout) {
        BeeErrorCode ret = BeeErrorCode.kBeeErrorCode_Success;
        do {
            if (command == null || args == null) {
                ret = BeeErrorCode.kBeeErrorCode_Invalid_Param;
                break;
            }

            ret = toBeeErrorCode(nativeExecute(nativeSDKService, command, args, timeout));
        } while (false);
        return ret;
    }

    /**
     *  @brief  通知业务数据.
     *  @param  data    业务数据.
     */
    protected void handleData(String data) {
        //do nothing
    }

    protected void registerNetStatus() {
        if (NetworkMonitor.isInitialized()) {
            NetworkMonitor.addNetworkObserver(this);
        }
    }

    protected void unRegisterNetStatus() {
        if (NetworkMonitor.isInitialized()) {
            NetworkMonitor.removeNetworkObserver(this);
        }
    }

    protected boolean isOnline() {
        return getCurrentNetType() != CONNECTION_NONE;
    }

    protected boolean isOnlineButNotWifi() {
        return isOnline() && (getCurrentNetType() != CONNECTION_WIFI);
    }

    protected boolean isOnlineAndIsWifi() {
        return isOnline() && (getCurrentNetType() == CONNECTION_WIFI);
    }

    private NetworkMonitorAutoDetect.ConnectionType getCurrentNetType() {
        if (NetworkMonitor.isInitialized()) {
            return NetworkMonitor.getConnectionType();
        } else {
            return CONNECTION_NONE;
        }
    }

    @Override
    public void onConnectionTypeChanged(NetworkMonitorAutoDetect.ConnectionType connectionType) {
        //do nothing
    }

    private native long nativeCreateSDKService(int svcCode);
    private native void nativeFreeSDKService(long nativeSDKService);
    private native int nativeReg(long nativeSDKService, int handle);
    private native int nativeUnReg(long nativeSDKService);
    private native int nativeExecute(long nativeSDKService, String command, String args, int timeout);
}
