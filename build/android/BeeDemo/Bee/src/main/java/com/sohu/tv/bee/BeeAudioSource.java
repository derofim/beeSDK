/**
 *  @file        BeeAudioSource.java
 *  @brief       BeeSDK音频源声明文件.
 *  @author      fangdali
 *  @copyright   sohu-inc
 *  @version     2.0.0.1
 */
package com.sohu.tv.bee;

import com.sohu.tv.bee.BeeSystemDefine.BeeErrorCode;

/// BeeSDK音频源类.
public class BeeAudioSource {
    protected long nativeBeeAudioSource;
    protected boolean levelControl;
    protected boolean echoCancel;
    protected boolean gainControl;
    protected boolean highPassFilter;
    protected boolean noiceSuppression;
    protected boolean opened;

    /**
     *  @brief  BeeSDK音频源类构造函数.
     *  @param  levelControl        是否使能输入音量控制.
     *  @param  echoCancel          是否使能回声消除.
     *  @param  gainControl         是否使能增益控制.
     *  @param  highPassFilter      是否使能高通滤波器.
     *  @param  noiceSuppression    是否使能噪声抑制.
     *  @return BeeSDK音频源类对象.
     */
    BeeAudioSource(boolean levelControl, boolean echoCancel, boolean gainControl,
                   boolean highPassFilter, boolean noiceSuppression) {
        this.nativeBeeAudioSource = 0;
        this.levelControl = levelControl;
        this.echoCancel = echoCancel;
        this.gainControl = gainControl;
        this.highPassFilter = highPassFilter;
        this.noiceSuppression = noiceSuppression;
        this.opened = false;
    }

    /**
     *  @brief  打开音频源.
     *  @return 错误码.
     */
    public BeeErrorCode open() {
        BeeErrorCode ret = BeeErrorCode.kBeeErrorCode_Success;
        do {
            if (opened) {
                break;
            }

            nativeBeeAudioSource = nativeCreateBeeAudioSource(levelControl, echoCancel, gainControl, highPassFilter, noiceSuppression);
            if (nativeBeeAudioSource == 0) {
                ret = BeeErrorCode.kBeeErrorCode_Null_Pointer;
                break;
            }

            opened = true;
        } while (false);

        return ret;
    }

    /**
     *  @brief  关闭音频源.
     */
    void dispose() {
        if (nativeBeeAudioSource != 0) {
            nativeFreeBeeAudioSource(nativeBeeAudioSource);
            nativeBeeAudioSource = 0;
        }
    }

    /**
     *  @brief  返回音频源底层指针.
     *  @return 音频源底层指针.
     */
    public long getNativeBeeVideoSource() {
        return nativeBeeAudioSource;
    }

    /**
     *  @brief  返回音频源是否打开.
     *  @return 音频源是否打开.
     */
    public boolean isOpened() {
        return opened;
    }

    private static native long nativeCreateBeeAudioSource(boolean level_control, boolean echo_cancel,
                          boolean gain_control, boolean high_pass_filter, boolean noise_suppression);
    private static native void nativeFreeBeeAudioSource(long nativeBeeVideoSource);
}
