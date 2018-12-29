/**
 *  @file        BeeOpenSessionParam.java
 *  @brief       BeeSDK打开会话返回信息.
 *  @author      fangdali
 *  @copyright   sohu-inc
 *  @version     2.0.0.1
 */
package com.sohu.tv.bee;

import java.util.Vector;

/// BeeSDK打开会话返回信息.
public class BeeOpenSessionParam {
    /// 会话ID
    private int handle;
    /// 会话错误码
    private int errorCode;
    /// 会话所有业务能力
    private Vector<BeeSDKCapability> capabilities = new Vector<BeeSDKCapability>();

    /// BeeSDK业务能力类.
    public class BeeSDKCapability {
        /// 业务码.
        private int svcCode;
        /// 业务描述.
        private String descriotion;

        public BeeSDKCapability(int svcCode, String descriotion) {
            this.svcCode = svcCode;
            this.descriotion = descriotion;
        }

        public int getSvcCode() {
            return svcCode;
        }

        public String getDescriotion() {
            return descriotion;
        }
    }

    /**
     *  @brief  BeeSDK会话信息类构造函数.
     *  @param  handle        会话ID.
     *  @param  errorCode     会话错误码.
     *  @param  capabilities  会话所有业务能力.
     *  @return BeeSDK音频源类对象.
     */
    public BeeOpenSessionParam(int handle, int errorCode, Vector<BeeSDKCapability> capabilities) {
        this.handle = handle;
        this.errorCode = errorCode;
        this.capabilities = capabilities;
    }

    public int getErrorCode() {
        return errorCode;
    }

    public int getHandle() {
        return handle;
    }

    public Vector<BeeSDKCapability> getCapabilities() {
        return capabilities;
    }
}
