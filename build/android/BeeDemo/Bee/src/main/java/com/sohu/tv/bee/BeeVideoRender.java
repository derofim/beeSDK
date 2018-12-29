/**
 *  @file        BeeVideoRender.java
 *  @brief       BeeSDK默认渲染器声明文件.
 *  @author      fangdali
 *  @copyright   sohu-inc
 *  @version     2.0.0.1
 */
package com.sohu.tv.bee;

import com.sohu.tv.bee.BeeSystemDefine.BeeErrorCode;

import org.webrtc.VideoRenderer;

/// 渲染器类.
public class BeeVideoRender {
    private long nativeBeeVideoRenderer;
    private VideoRenderer internalVideoRenderer;
    private boolean opened;

    /**
     * @brief 渲染器构造函数.
     * @param view 标准视图.
     */
    public BeeVideoRender(BeeSurfaceViewRenderer view) {
        nativeBeeVideoRenderer = 0;
        internalVideoRenderer = new VideoRenderer(view);
        opened = false;
    }

    /**
     * @brief 渲染器释放.
     */
    public void dispose() {
        if (internalVideoRenderer != null) {
            //rtcVideoRenderer.dispose(); //Do not call this or will cause double delete.
            internalVideoRenderer = null;
        }

        if (nativeBeeVideoRenderer != 0) {
            nativeFreeBeeVideoRenderer(nativeBeeVideoRenderer); //This will delete rtcVideoRenderer too.
            nativeBeeVideoRenderer = 0;
        }
    }

    /**
     * @brief  打开渲染器
     * @return 错误码
     */
    public BeeErrorCode open() {
        BeeErrorCode ret = BeeErrorCode.kBeeErrorCode_Success;
        do {
            if (opened) {
                break;
            }

            nativeBeeVideoRenderer = nativeCreateBeeVideoRenderer(internalVideoRenderer.getNativeVideoRenderer());
            if (nativeBeeVideoRenderer == 0) {
                ret = BeeErrorCode.kBeeErrorCode_Null_Pointer;
                break;
            }

            opened = true;
        } while (false);
        return ret;
    }

    /**
     * @brief  获取渲染器底部指针.
     * @return 渲染器指针.
     */
    public long getNativeBeeVideoRenderer() {
        return nativeBeeVideoRenderer;
    }

    /**
     * @brief  渲染器是否打开.
     * @return 是否打开.
     */
    public boolean isOpened() {
        return opened;
    }

    private static native long nativeCreateBeeVideoRenderer(long nativeRtcVideoRenderer);
    private static native void nativeFreeBeeVideoRenderer(long nativeBeeVideoRenderer);
}
