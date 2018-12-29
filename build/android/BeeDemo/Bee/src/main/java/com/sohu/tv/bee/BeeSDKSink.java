/**
 *  @file        BeeSDKSink.java
 *  @brief       BeeSDK获取通用通知的协议文件.
 *  @author      fangdali
 *  @copyright   sohu-inc
 *  @version     2.0.0.1
 */
package com.sohu.tv.bee;

import com.sohu.tv.bee.BeeSystemDefine.BeeErrorCode;

/// BeeSDK通用通知协议.
public interface BeeSDKSink {

    /**
     *  @brief  日志回调.
     *  @param  log     日志行.
     */
    void onLog(String log);

    /**
     *  @brief  系统通知.
     *  @param  ec1     BeeSDK错误码.
     *  @param  ec2     平台相关错误码.
     */
    void onNotify(BeeErrorCode ec1, int ec2);
}