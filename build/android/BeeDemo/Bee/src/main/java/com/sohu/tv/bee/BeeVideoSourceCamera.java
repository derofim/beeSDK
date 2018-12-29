/**
 *  @file        BeeVideoSourceCamera.java
 *  @brief       BeeSDK摄像头视频源声明文件.
 *  @author      fangdali
 *  @copyright   sohu-inc
 *  @version     2.0.0.1
 */
package com.sohu.tv.bee;

import android.content.Context;

import org.webrtc.Camera1Enumerator;
import org.webrtc.Camera2Enumerator;
import org.webrtc.CameraEnumerator;
import org.webrtc.CameraVideoCapturer;
import org.webrtc.EglBase;
import org.webrtc.Logging;
import org.webrtc.VideoCapturer;

/// BeeSDK摄像头视频源类.
public class BeeVideoSourceCamera extends BeeVideoSource implements VideoCapturer.ExternalObserver, CameraVideoCapturer.CameraEventsHandler {
    private String TAG = "BeeVideoSourceCamera";
    private boolean useCamera2 = false;
    private boolean captureToTexture = false;

    /**
     *  @brief  摄像头视频源类构造函数.
     *  @param  width               摄像头输出图像宽度.
     *  @param  height              摄像头输出图像高度.
     *  @param  fps                 摄像头输出帧率.
     *  @param  appContext          android上下文.
     *  @param  eglContext          egl上下文.
     *  @param  useCamera2          是否使用camera2.
     *  @param  captureToTexture    是否使用纹理.
     *  @return 摄像头视频源类对象.
     */
    public BeeVideoSourceCamera(
            int width, int height, int fps, Context appContext, EglBase.Context eglContext,
            boolean useCamera2, boolean captureToTexture) {
        super(false, width, height, fps, appContext, eglContext);
        this.useCamera2 = useCamera2;
        this.captureToTexture = captureToTexture;
    }

    /**
     *  @brief 视频源释放.
     */
    public void dispose() {
        super.dispose();
    }

    private boolean useCamera2() {
        return Camera2Enumerator.isSupported(appContext) && this.useCamera2;
    }

    private VideoCapturer createCameraCapturer(CameraEnumerator enumerator) {
        final String[] deviceNames = enumerator.getDeviceNames();

        // First, try to find front facing camera
        Logging.d(TAG, "Looking for front facing cameras.");
        for (String deviceName : deviceNames) {
            if (enumerator.isFrontFacing(deviceName)) {
                Logging.d(TAG, "Creating front facing camera capturer.");
                VideoCapturer videoCapturer = enumerator.createCapturer(deviceName, this);

                if (videoCapturer != null) {
                    return videoCapturer;
                }
            }
        }

        // Front facing camera not found, try something else
        Logging.d(TAG, "Looking for other cameras.");
        for (String deviceName : deviceNames) {
            if (!enumerator.isFrontFacing(deviceName)) {
                Logging.d(TAG, "Creating other camera capturer.");
                VideoCapturer videoCapturer = enumerator.createCapturer(deviceName, this);

                if (videoCapturer != null) {
                    return videoCapturer;
                }
            }
        }

        return null;
    }

    protected boolean openInput(long nativeRtcVideoSource) {
        boolean ret = true;
        do {
            if (useCamera2()) {
                if (!captureToTexture) {
                    Logging.e(TAG, "Camera2 must use captureToTexture.");
                    ret = false;
                    break;
                }
                internalVideoCapturer = createCameraCapturer(new Camera2Enumerator(appContext));
            } else {
                internalVideoCapturer = createCameraCapturer(new Camera1Enumerator(captureToTexture));
            }

            if (internalVideoCapturer == null) {
                ret = false;
                break;
            }

            videoTrackSourceObserver = new VideoCapturer.AndroidVideoTrackSourceObserver(nativeRtcVideoSource, this);
        } while (false);
        return ret;
    }

    @Override
    public void onCapturerStarted(boolean success) {
        if (beeVideoSourceOpenSink != null) {
            beeVideoSourceOpenSink.onOpen(success);
        }
    }

    @Override
    public void onCapturerStopped() {
    }

    @Override
    public void onCameraError(String errorDescription) {
    }

    @Override
    public void onCameraDisconnected() {
    }

    @Override
    public void onCameraFreezed(String errorDescription) {
    }

    @Override
    public void onCameraOpening(String cameraName) {
    }

    @Override
    public void onFirstFrameAvailable() {
    }

    @Override
    public void onCameraClosed() {
        if (beeVideoSourceOpenSink != null) {
            beeVideoSourceOpenSink.onClosed();
        }
    }
}
