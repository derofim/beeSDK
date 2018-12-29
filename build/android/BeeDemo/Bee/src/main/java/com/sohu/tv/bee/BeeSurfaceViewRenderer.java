/**
 *  @file        BeeSurfaceViewRenderer.java
 *  @brief       BeeSDK渲染器view声明文件.
 *  @author      fangdali
 *  @copyright   sohu-inc
 *  @version     2.0.0.1
 */
package com.sohu.tv.bee;

import android.content.Context;
import android.content.res.Resources.NotFoundException;
import android.graphics.Point;
import android.util.AttributeSet;
import android.view.SurfaceHolder;
import android.view.SurfaceView;

import org.webrtc.EglBase;
import org.webrtc.EglRenderer;
import org.webrtc.GlRectDrawer;
import org.webrtc.Logging;
import org.webrtc.RendererCommon;
import org.webrtc.ThreadUtils;
import org.webrtc.VideoRenderer;

import java.util.concurrent.CountDownLatch;

/// android渲染器view类.
public class BeeSurfaceViewRenderer
    extends SurfaceView implements SurfaceHolder.Callback, VideoRenderer.Callbacks {
  private static final String TAG = "BeeSurfaceViewRenderer";

  /// 缓存的资源名称.
  private final String resourceName;
  private final RendererCommon.VideoLayoutMeasure videoLayoutMeasure =
      new RendererCommon.VideoLayoutMeasure();
  private final EglRenderer eglRenderer;

  // Callback for reporting renderer events. Read-only after initilization so no lock required.
  private RendererCommon.RendererEvents rendererEvents;

  private final Object layoutLock = new Object();
  private boolean isFirstFrameRendered;
  private int rotatedFrameWidth;
  private int rotatedFrameHeight;
  private int frameRotation;

  // Accessed only on the main thread.
  private boolean enableFixedSize;
  private int surfaceWidth;
  private int surfaceHeight;

  private boolean initialize = false;

  /**
   * @brief  标准视图构造函数
   * @param  context  android上下文
   * @note   为了呈现某些内容，您必须首先调用init().
   */
  public BeeSurfaceViewRenderer(Context context) {
    super(context);
    this.resourceName = getResourceName();
    eglRenderer = new EglRenderer(resourceName);
    getHolder().addCallback(this);
  }

  /**
   * @brief  标准视图构造函数
   * @param  context  android上下文
   * @param  attrs    自定义属性
   * @note   为了呈现某些内容，您必须首先调用init().
   */
  public BeeSurfaceViewRenderer(Context context, AttributeSet attrs) {
    super(context, attrs);
    this.resourceName = getResourceName();
    eglRenderer = new EglRenderer(resourceName);
    getHolder().addCallback(this);
  }

  /**
   * @brief   是否初始化
   * @return  返回是否初始化
   */
  public boolean isInitialized() {
    return initialize;
  }

  /**
   * @brief  标准视图初始化
   * @param  sharedContext  共享egl上下文
   * @param  rendererEvents 渲染器辅助类(默认null)
   * @note   允许release()后，重新初始化渲染器
   */
  public void init(EglBase.Context sharedContext, RendererCommon.RendererEvents rendererEvents) {
    init(sharedContext, rendererEvents, EglBase.CONFIG_PLAIN, new GlRectDrawer());
  }

  public void init(final EglBase.Context sharedContext,
      RendererCommon.RendererEvents rendererEvents, final int[] configAttributes,
      RendererCommon.GlDrawer drawer) {
    if (true == initialize) {
      return;
    }

    ThreadUtils.checkIsOnMainThread();
    this.rendererEvents = rendererEvents;
    synchronized (layoutLock) {
      rotatedFrameWidth = 0;
      rotatedFrameHeight = 0;
      frameRotation = 0;
    }
    eglRenderer.init(sharedContext, configAttributes, drawer);
    setEnableHardwareScaler(false);
    setMirror(true);
    initialize = true;
  }

  /**
   * @brief  标准视图资源释放
   * @note  如果不调用此函数，GL资源可能会泄漏。
   */
  public void release() {
    eglRenderer.release();
  }

  /**
   * @brief 注册在收到新视频帧时要调用的回调。
   * @param listener 要调用的回调.
   * @param scale    传递给回调的Bitmap的比例，如果不需要Bitmap，则为0.
   * @param drawer   用于视频帧回调的自定义画板.
   */
  public void addFrameListener(
      EglRenderer.FrameListener listener, float scale, final RendererCommon.GlDrawer drawer) {
    eglRenderer.addFrameListener(listener, scale, drawer);
  }

  /**
   * Register a callback to be invoked when a new video frame has been received. This version uses
   * the drawer of the EglRenderer that was passed in init.
   *
   * @param listener The callback to be invoked.
   * @param scale    The scale of the Bitmap passed to the callback, or 0 if no Bitmap is
   *                 required.
   */
  public void addFrameListener(EglRenderer.FrameListener listener, float scale) {
    eglRenderer.addFrameListener(listener, scale);
  }

  public void removeFrameListener(EglRenderer.FrameListener listener) {
    eglRenderer.removeFrameListener(listener);
  }

  /**
   * @brief 设置为表面启用固定大小。
   * @param enabled  是否启用
   * @note  但在某些设备上可能有问题。默认情况下，此功能已关闭。
   */
  public void setEnableHardwareScaler(boolean enabled) {
    ThreadUtils.checkIsOnMainThread();
    enableFixedSize = enabled;
    updateSurfaceSize();
  }

  /**
   * @brief 设置是否应镜像视频流。
   * @param mirror 是否开启
   */
  public void setMirror(final boolean mirror) {
    eglRenderer.setMirror(mirror);
  }

  /**
   * @brief 设置视频将如何填充允许的布局区域。
   * @param scalingType  填充类型(SCALE_ASPECT_FIT, SCALE_ASPECT_FILL, SCALE_ASPECT_BALANCED).
   */
  public void setScalingType(RendererCommon.ScalingType scalingType) {
    ThreadUtils.checkIsOnMainThread();
    videoLayoutMeasure.setScalingType(scalingType);
  }

  public void setScalingType(RendererCommon.ScalingType scalingTypeMatchOrientation,
      RendererCommon.ScalingType scalingTypeMismatchOrientation) {
    ThreadUtils.checkIsOnMainThread();
    videoLayoutMeasure.setScalingType(scalingTypeMatchOrientation, scalingTypeMismatchOrientation);
  }

  /**
   * @brief 限制渲染帧率
   * @param fps 限制渲染帧率到此值，或使用Float.POSITIVE_INFINITY禁用fps缩减.
   */
  public void setFpsReduction(float fps) {
    eglRenderer.setFpsReduction(fps);
  }

  /**
   * @brief 禁止降低渲染帧率
   */
  public void disableFpsReduction() {
    eglRenderer.disableFpsReduction();
  }

  /**
   * @brief 暂停视频
   */
  public void pauseVideo() {
    eglRenderer.pauseVideo();
  }

  // VideoRenderer.Callbacks interface.
  @Override
  public void renderFrame(VideoRenderer.I420Frame frame) {
    updateFrameDimensionsAndReportEvents(frame);
    eglRenderer.renderFrame(frame);
  }

  // View layout interface.
  @Override
  protected void onMeasure(int widthSpec, int heightSpec) {
    ThreadUtils.checkIsOnMainThread();
    final Point size;
    synchronized (layoutLock) {
      size =
          videoLayoutMeasure.measure(widthSpec, heightSpec, rotatedFrameWidth, rotatedFrameHeight);
    }
    setMeasuredDimension(size.x, size.y);
    logD("onMeasure(). New size: " + size.x + "x" + size.y);
  }

  @Override
  protected void onLayout(boolean changed, int left, int top, int right, int bottom) {
    ThreadUtils.checkIsOnMainThread();
    eglRenderer.setLayoutAspectRatio((right - left) / (float) (bottom - top));
    updateSurfaceSize();
  }

  private void updateSurfaceSize() {
    ThreadUtils.checkIsOnMainThread();
    synchronized (layoutLock) {
      if (enableFixedSize && rotatedFrameWidth != 0 && rotatedFrameHeight != 0 && getWidth() != 0
          && getHeight() != 0) {
        final float layoutAspectRatio = getWidth() / (float) getHeight();
        final float frameAspectRatio = rotatedFrameWidth / (float) rotatedFrameHeight;
        final int drawnFrameWidth;
        final int drawnFrameHeight;
        if (frameAspectRatio > layoutAspectRatio) {
          drawnFrameWidth = (int) (rotatedFrameHeight * layoutAspectRatio);
          drawnFrameHeight = rotatedFrameHeight;
        } else {
          drawnFrameWidth = rotatedFrameWidth;
          drawnFrameHeight = (int) (rotatedFrameWidth / layoutAspectRatio);
        }
        // Aspect ratio of the drawn frame and the view is the same.
        final int width = Math.min(getWidth(), drawnFrameWidth);
        final int height = Math.min(getHeight(), drawnFrameHeight);
        logD("updateSurfaceSize. Layout size: " + getWidth() + "x" + getHeight() + ", frame size: "
            + rotatedFrameWidth + "x" + rotatedFrameHeight + ", requested surface size: " + width
            + "x" + height + ", old surface size: " + surfaceWidth + "x" + surfaceHeight);
        if (width != surfaceWidth || height != surfaceHeight) {
          surfaceWidth = width;
          surfaceHeight = height;
          getHolder().setFixedSize(width, height);
        }
      } else {
        surfaceWidth = surfaceHeight = 0;
        getHolder().setSizeFromLayout();
      }
    }
  }

  // SurfaceHolder.Callback interface.
  @Override
  public void surfaceCreated(final SurfaceHolder holder) {
    ThreadUtils.checkIsOnMainThread();
    eglRenderer.createEglSurface(holder.getSurface());
    surfaceWidth = surfaceHeight = 0;
    updateSurfaceSize();
  }

  @Override
  public void surfaceDestroyed(SurfaceHolder holder) {
    ThreadUtils.checkIsOnMainThread();
    final CountDownLatch completionLatch = new CountDownLatch(1);
    eglRenderer.releaseEglSurface(new Runnable() {
      @Override
      public void run() {
        completionLatch.countDown();
      }
    });
    ThreadUtils.awaitUninterruptibly(completionLatch);
  }

  @Override
  public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {
    ThreadUtils.checkIsOnMainThread();
    logD("surfaceChanged: format: " + format + " size: " + width + "x" + height);
  }

  private String getResourceName() {
    try {
      return getResources().getResourceEntryName(getId()) + ": ";
    } catch (NotFoundException e) {
      return "";
    }
  }

  // Update frame dimensions and report any changes to |rendererEvents|.
  private void updateFrameDimensionsAndReportEvents(VideoRenderer.I420Frame frame) {
    synchronized (layoutLock) {
      if (!isFirstFrameRendered) {
        isFirstFrameRendered = true;
        logD("Reporting first rendered frame.");
        if (rendererEvents != null) {
          rendererEvents.onFirstFrameRendered();
        }
      }
      if (rotatedFrameWidth != frame.rotatedWidth() || rotatedFrameHeight != frame.rotatedHeight()
          || frameRotation != frame.rotationDegree) {
        logD("Reporting frame resolution changed to " + frame.width + "x" + frame.height
            + " with rotation " + frame.rotationDegree);
        if (rendererEvents != null) {
          rendererEvents.onFrameResolutionChanged(frame.width, frame.height, frame.rotationDegree);
        }
        rotatedFrameWidth = frame.rotatedWidth();
        rotatedFrameHeight = frame.rotatedHeight();
        frameRotation = frame.rotationDegree;
        post(new Runnable() {
          @Override
          public void run() {
            updateSurfaceSize();
            requestLayout();
          }
        });
      }
    }
  }

  private void logD(String string) {
    Logging.d(TAG, resourceName + string);
  }
}
