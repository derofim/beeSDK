/**
 *  @file        BeeWhiteBoardSink.java
 *  @brief       BeeSDK白板回调声明文件.
 *  @author      fangdali
 *  @copyright   sohu-inc
 *  @version     2.0.0.1
 */
package com.sohu.tv.bee;

import com.sohu.tv.bee.BeeSystemDefine.BeeErrorCode;

/// BeeSDK白板回调协议.
public interface BeeWhiteBoardSink {
    /**
     *  @brief  加入白板结果回调.
     *  @param  error       错误码.
     *  @param  msg         错误描述.
     */
    void onJoin(BeeErrorCode error, String msg);

    /**
     *  @brief  离开白板结果回调.
     *  @param  error       错误码.
     *  @param  msg         错误描述.
     */
    void onLeave(BeeErrorCode error, String msg);
}
