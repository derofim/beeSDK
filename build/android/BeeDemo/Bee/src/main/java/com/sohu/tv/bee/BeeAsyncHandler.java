/**
 *  @file        BeeAsyncHandler.java
 *  @brief       异步回调接口声明文件.
 *  @author      fangdali
 *  @copyright   sohu-inc
 *  @version     2.0.0.1
 */
package com.sohu.tv.bee;

import android.graphics.Bitmap;

import com.sohu.tv.bee.BeeSystemDefine.BeeErrorCode;
import java.util.Vector;

/// 异步回调接口类
public class BeeAsyncHandler {
    /**
     *  @brief  BeeSDK.init回调.
     *  @param  ec  错误码.
     */
    public void InitHandler(BeeErrorCode ec) {}

    /**
     *  @brief  BeeSDK.unit回调.
     *  @param  ec  错误码.
     */
    public void UnInitHandler(BeeErrorCode ec) {}

    /**
     *  @brief  BeeSDK.open回调.
     *  @param  ec              错误码.
     *  @param  handle          会话句柄.
     *  @param  capabilities    会话的业务能力数组.
     */
    public void OpenSessionHandler(BeeErrorCode ec, int handle, Vector<BeeOpenSessionParam.BeeSDKCapability> capabilities) {}

    /**
     *  @brief  BeeSDK.close回调.
     *  @param  ec  错误码.
     */
    public void CloseSessionHandler(BeeErrorCode ec) {}

    /**
     *  @brief  BeeVideoRoom.setupPushStream回调.
     *  @param  ec  错误码.
     */
    public void SetupPushStreamHandler(BeeErrorCode ec) {}

    /**
     *  @brief  BeeWhiteBoard.sendData回调.
     *  @param  ec  错误码.
     *  @note   内部使用.
     */
    public void SendDataHandler(BeeErrorCode ec) {}

    /**
     *  @brief  BeeWhiteBoardView.getDocFromUrl回调.
     *  @param  ec      错误码.
     *  @param  bitmap  源文件.
     *  @param  err     具体错误信息.
     *  @note   内部使用.
     */
    public void LoadDocHandler(BeeErrorCode ec, Bitmap bitmap, String err) {}
}
