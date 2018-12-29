/**
 *  @file        BeeVideoSource.java
 *  @brief       BeeSDK视频源声明文件.
 *  @author      fangdali
 *  @copyright   sohu-inc
 *  @version     2.0.0.1
 */
package com.sohu.tv.bee;

import android.content.Context;

import com.sohu.tv.bee.BeeSystemDefine.BeeErrorCode;

import org.webrtc.CameraVideoCapturer;
import org.webrtc.EglBase;
import org.webrtc.Logging;
import org.webrtc.VideoCapturer;
import org.webrtc.VideoSource;

/// BeeSDK视频源类.
public class BeeVideoSource {
    static public class BeeVideoSourceSink {
        public void onOpen(boolean success) {
        }
        public void onClosed() {
        }
    }

    private String TAG = "BeeVideoSource";
    protected VideoSource internalVideoSource;
    protected VideoCapturer internalVideoCapturer;
    protected VideoCapturer.AndroidVideoTrackSourceObserver videoTrackSourceObserver;
    protected BeeVideoSourceSink beeVideoSourceOpenSink;
    protected Context appContext;
    protected EglBase.Context eglContext;
    protected long nativeBeeVideoSource;
    /// 视频宽.
    protected int videoWidth;
    /// 视频高.
    protected int videoHeight;
    /// 视频帧率.
    protected int videoFps;
    /// 是否抓屏.
    protected boolean isScreencast;
    protected boolean opened;

    /**
     *  @brief  视频源类构造函数.
     *  @param  isScreencast    是否抓屏.
     *  @param  width           视频源输出图像宽度.
     *  @param  height          视频源输出图像高度.
     *  @param  fps             视频源输出帧率.
     *  @param  appContext      android上下文.
     *  @param  eglContext      egl上下文.
     *  @return 视频源类对象.
     *  @note   必须在派生类构造函数调用.
     */
    public BeeVideoSource(boolean isScreencast, int width, int height, int fps, Context appContext, EglBase.Context eglContext) {
        this.videoWidth = width;
        this.videoHeight = height;
        this.videoFps = fps;
        this.isScreencast = isScreencast;
        this.appContext = appContext;
        this.eglContext = eglContext;
        this.nativeBeeVideoSource = 0;
        this.opened = false;
    }

    /**
     *  @brief 视频源释放.
     */
    public void dispose() {
        if (internalVideoCapturer != null) {
            internalVideoCapturer.dispose();
            internalVideoCapturer = null;
        }

        if (internalVideoSource != null) {
            //internalVideoSource.dispose(); //Do not call this or will cause double delete.
            internalVideoSource = null;
        }

        if (nativeBeeVideoSource != 0) {
            nativeFreeBeeVideoSource(nativeBeeVideoSource);
            nativeBeeVideoSource = 0;
        }
    }

    protected boolean openInput(long nativeRtcVideoSource) {
        //To create internal VideoCapturer(Camera,Screencast) in derived class,
        //if using external capturer, do not implement this.
        return true;
    }

    /**
     *  @brief  打开视频源.
     *  @param  openSink  视频源回调接口.
     *  @return 错误码.
     *  @note   该方法必须被调用.
     */
    public BeeErrorCode open(BeeVideoSourceSink openSink) {
        BeeErrorCode ret = BeeErrorCode.kBeeErrorCode_Success;
        do {
            if (opened) {
                break;
            }

            if (appContext == null || eglContext == null) {
                ret = BeeErrorCode.kBeeErrorCode_Invalid_Param;
                break;
            }

            //Create native bee video source pointer.
            nativeBeeVideoSource = nativeCreateBeeVideoSource(
                    videoWidth, videoHeight, videoFps, eglContext, isScreencast);
            if (nativeBeeVideoSource == 0) {
                ret = BeeErrorCode.kBeeErrorCode_Null_Pointer;
                break;
            }

            //Get native rtc video source pointer.
            long nativeRtcVideoSource = nativeGetRtcVideoSource(nativeBeeVideoSource);
            if (nativeRtcVideoSource == 0) {
                ret = BeeErrorCode.kBeeErrorCode_Null_Pointer;
                break;
            }

            //Wrap native WebRTC VideoTrackSourceInterface
            internalVideoSource = new VideoSource(nativeRtcVideoSource);

            if (!openInput(nativeRtcVideoSource)) {
                Logging.e(TAG, "Open video source input failed.");
                ret = BeeErrorCode.kBeeErrorCode_Invalid_State;
                break;
            }

            //Bind native WebRTC VideoTrackSourceInterface to capturer observer.
            if (videoTrackSourceObserver == null) {
                videoTrackSourceObserver = new VideoCapturer.AndroidVideoTrackSourceObserver(nativeRtcVideoSource, null);
            }

            if (internalVideoCapturer == null) {
                if (openSink != null) {
                    openSink.onOpen(true);
                }
            } else {
                //If using internal video capturer, now initialize and start it.
                //VideoSource.initializeVideoCapturer(appContext, internalVideoCapturer, nativeRtcVideoSource, videoTrackSourceObserver);
                nativeInitializeVideoCapturer(appContext, internalVideoCapturer, nativeBeeVideoSource, videoTrackSourceObserver);
                internalVideoCapturer.startCapture(videoWidth, videoHeight, videoFps);
                beeVideoSourceOpenSink = openSink;
            }

            opened = true;
        } while (false);

        if (ret != BeeErrorCode.kBeeErrorCode_Success) {
            if (openSink != null) {
                openSink.onOpen(false);
            }
        }
        return ret;
    }

    /**
     * @brief 获取视频源底部指针.
     * @return 视频源指针.
     */
    public long getNativeBeeVideoSource() {
        return nativeBeeVideoSource;
    }

    /**
     * @brief  视频源是否打开.
     * @return 返回视频源是否打开.
     */
    public boolean isOpened() {
        return opened;
    }

    /**
     * @brief 改变视频源制式.
     * @param width  视频源宽度.
     * @param height 视频源高度.
     * @param fps    视频源帧数.
     */
    public void changeCaptureFormat(int width, int height, int fps) {
        if (nativeBeeVideoSource == 0) {
            Logging.e(TAG, "Failed to change capture format.");
            return;
        }

        nativeAdaptOutputFormat(nativeBeeVideoSource, width, height, fps);
    }

    public void switchCapture() {
        if (internalVideoCapturer != null) {
            if (internalVideoCapturer instanceof CameraVideoCapturer) {
                Logging.d(TAG, "Switch camera");
                CameraVideoCapturer cameraVideoCapturer = (CameraVideoCapturer) internalVideoCapturer;
                cameraVideoCapturer.switchCamera(null);
            } else {
                Logging.d(TAG, "Will not switch camera, video caputurer is not a camera");
            }
        }
    }

    protected void onByteBufferFrameCaptured(
            byte[] data, int width, int height, int rotation, long timeStamp) {
        if (videoTrackSourceObserver == null) {
            return;
        }

        videoTrackSourceObserver.onByteBufferFrameCaptured(data, width, height, rotation, timeStamp);
    }

    protected void onTextureFrameCaptured(
            int width, int height, int oesTextureId, float[] transformMatrix, int rotation, long timestamp) {
        if (videoTrackSourceObserver == null) {
            return;
        }

        videoTrackSourceObserver.onTextureFrameCaptured(width, height, oesTextureId, transformMatrix, rotation, timestamp);
    }

    private static native long nativeCreateBeeVideoSource(
            int videoWidth, int videoHeight, int videoFps, EglBase.Context eglContext, boolean is_screencast);

    private static native void nativeAdaptOutputFormat(
            long nativeBeeVideoSource, int width, int height, int fps);

    private static native void nativeInitializeVideoCapturer(
            Context appContext, VideoCapturer j_video_capturer, long nativeBeeVideoSource,
            VideoCapturer.CapturerObserver j_frame_observer);

    private static native void nativeFreeBeeVideoSource(long nativeBeeVideoSource);

    private static native long nativeGetRtcVideoSource(long nativeBeeVideoSource);
}
