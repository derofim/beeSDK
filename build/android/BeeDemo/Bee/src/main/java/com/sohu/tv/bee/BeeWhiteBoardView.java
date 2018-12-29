package com.sohu.tv.bee;

import android.app.Service;
import android.content.Context;
import android.graphics.Color;
import android.media.AudioManager;
import android.os.Build;
import android.support.annotation.NonNull;
import android.support.annotation.Nullable;
import android.util.AttributeSet;
import android.view.Gravity;
import android.view.KeyEvent;
import android.view.MotionEvent;
import android.widget.FrameLayout;
import android.widget.LinearLayout;
import android.widget.RelativeLayout;

import java.util.HashMap;

import static android.view.ViewGroup.LayoutParams.MATCH_PARENT;
import static android.view.ViewGroup.LayoutParams.WRAP_CONTENT;

public class BeeWhiteBoardView extends FrameLayout {
    private final static String TAG = BeeWhiteBoardView.class.getSimpleName();
    private Context context = null;
    private BeeWhiteBoardDrawView beeWhiteBoardDrawView = null;
    private BeeWhiteBoardVideoView beeWhiteBoardVideoView = null;

    private boolean isDrawLock = false;
    private EventMode mode = EventMode.EVENT_MODE_NONE; // 1:draw 2:scale and translation
    private long firstDown = System.currentTimeMillis();
    private float downInstance = 0.0f;
    private float prePointX = 0.0f;
    private float prePointY = 0.0f;
    private float scale = 1.0f;


    public enum EventMode {
        EVENT_MODE_NONE,
        EVENT_MODE_DRAW,
        EVENT_MODE_SCALE_AND_TRANSLATION
    }

    public BeeWhiteBoardView(@NonNull Context context) {
        super(context);
        layoutInit(context);
    }

    public BeeWhiteBoardView(@NonNull Context context, @Nullable AttributeSet attrs) {
        super(context, attrs);
        layoutInit(context);
    }

    public BeeWhiteBoardView(@NonNull Context context, @Nullable AttributeSet attrs, int defStyleAttr) {
        super(context, attrs, defStyleAttr);
        layoutInit(context);
    }

    private void layoutInit(Context context) {
        this.context = context;
        RelativeLayout.LayoutParams layoutParams = new RelativeLayout.LayoutParams(MATCH_PARENT, MATCH_PARENT);
        beeWhiteBoardDrawView = new BeeWhiteBoardDrawView(context);
        beeWhiteBoardDrawView.setLayoutParams(layoutParams);
        this.addView(beeWhiteBoardDrawView);
        // test
        //String url = "http://ivi.bupt.edu.cn/hls/cctv6hd.m3u8";
        //playVideo(url, 50, 50, 100, 100);
    }

    @Override
    protected void onMeasure(int widthMeasureSpec, int heightMeasureSpec) {
        super.onMeasure(widthMeasureSpec, heightMeasureSpec);
    }

    @Override
    protected void onSizeChanged(int w, int h, int oldw, int oldh) {
        super.onSizeChanged(w, h, oldw, oldh);
    }

    @Override
    public boolean onTouchEvent(MotionEvent event) {
        int action = event.getActionMasked();
        if (action == MotionEvent.ACTION_CANCEL) {
            return false;
        }

        float x = event.getX();
        float y = event.getY();
        switch (action) {
            case MotionEvent.ACTION_DOWN:
                mode = EventMode.EVENT_MODE_NONE;
                firstDown = System.currentTimeMillis();
                if (beeWhiteBoardDrawView != null) {
                    beeWhiteBoardDrawView.onTouchBegin(x, y);
                }
                break;
            case MotionEvent.ACTION_POINTER_DOWN:
                mode = EventMode.EVENT_MODE_SCALE_AND_TRANSLATION;
                if (event.getPointerCount() > 1) {
                    downInstance = getDistance(event);
                    prePointX = event.getX(0);
                    prePointY = event.getY(0);
                }
                break;
            case MotionEvent.ACTION_MOVE:
                if ((mode != EventMode.EVENT_MODE_SCALE_AND_TRANSLATION && System.currentTimeMillis() - firstDown > 60)) {
                    mode = EventMode.EVENT_MODE_DRAW;
                }

                if (EventMode.EVENT_MODE_DRAW == mode) {
                    if (beeWhiteBoardDrawView != null) {
                        beeWhiteBoardDrawView.onTouchMove(x, y);
                    }
                } else if (EventMode.EVENT_MODE_SCALE_AND_TRANSLATION == mode) {
                    if (event.getPointerCount() > 1) {
                        //scale
                        float moveInstance = getDistance(event);
                        float mp = moveInstance / downInstance;
                        scale *= mp;
                        if (scale < 0.75f) {
                            scale = 0.75f;
                        } else if (scale > 10f) {
                            scale = 10f;
                        }
                        setScaleX(scale);
                        setScaleY(scale);

                        //move
                        float movePointX = event.getX(0);
                        float movePointY = event.getY(0);
                        int offerx = (int) (movePointX - prePointX);
                        int offery = (int) (movePointY - prePointY);

                        float translationx = getTranslationX() + offerx;
                        float translationy = getTranslationY() + offery;
                        if (translationx > getWidth() * scale * 1 / 2) {
                            translationx = getWidth() * scale * 1 / 2;
                        } else if (translationx < getWidth() * scale * -1 / 2) {
                            translationx = getWidth() * scale * -1 / 2;
                        }
                        if (translationy > getHeight() * scale * 1 / 2) {
                            translationy = getHeight() * scale * 1 / 2;
                        } else if (translationy < getHeight() * scale * -1 / 2) {
                            translationy = getHeight() * scale * -1 / 2;
                        }
                        setTranslationX(translationx);
                        setTranslationY(translationy);

                        /*if (Build.VERSION.SDK_INT <= Build.VERSION_CODES.N) {
                            if (beeWhiteBoardVideoView != null) {
                                beeWhiteBoardVideoView.setScale(scale);
                                beeWhiteBoardVideoView.requestLayout();
                            }
                        }*/
                    }
                }
                break;
            case MotionEvent.ACTION_UP:
                if (event.getPointerCount() == 1) {
                    if (beeWhiteBoardDrawView != null) {
                        beeWhiteBoardDrawView.onTouchEnd();
                    }
                }
                break;
            default:
                break;
        }

        return true;
    }

    /*获取两指之间的距离*/
    private float getDistance(MotionEvent event) {
        float x = event.getX(1) - event.getX(0);
        float y = event.getY(1) - event.getY(0);
        float distance = (float) Math.sqrt(x * x + y * y);//两点间的距离
        return distance;
    }

    public BeeWhiteBoardDrawView getBeeWhiteBoardDrawView() {
        return beeWhiteBoardDrawView;
    }

    public void setWhiteBoard(BeeWhiteBoardViewDelegate delegate) {
        if (beeWhiteBoardDrawView != null) {
            beeWhiteBoardDrawView.setWhiteBoard(delegate);
        }
    }

    /**
     * @brief   设置白板本地绘制线条颜色.
     * @param   rgbColor    线条颜色.
     */
    public void setLineColor(int rgbColor) {
        if (beeWhiteBoardDrawView != null) {
            beeWhiteBoardDrawView.setLineColor(rgbColor);
        }
    }

    /**
     * @brief   设置白板本地绘制线条宽度.
     * @param   width       线条宽度.
     */
    public void setLineWidth(int width) {
        if (beeWhiteBoardDrawView != null) {
            beeWhiteBoardDrawView.setLineWidth(width);
        }
    }

    /**
     * @brief   设置白板本地绘制模式.
     * @param   mode        绘制模式.
     */
    public void setDrawingMode(final BeeWhiteBoardDefine.BeeDrawingMode mode) {
        if (beeWhiteBoardDrawView != null) {
            beeWhiteBoardDrawView.setDrawingMode(mode);
        }
    }

    /**
     *  @brief  清除白板当前页屏幕.
     */
    public void clearAll() {
        if (beeWhiteBoardDrawView != null) {
            beeWhiteBoardDrawView.clearAll();
        }
    }

    /**
     * @brief   取消白板上一次绘制.
     */
    public void undo() {
        if (beeWhiteBoardDrawView != null) {
            beeWhiteBoardDrawView.undo();
        }
    }

    /**
     * @brief   重做白板上一次取消的绘制.
     */
    public void redo() {
        if (beeWhiteBoardDrawView != null) {
            beeWhiteBoardDrawView.redo();
        }
    }

    public void prepage() {
        if (beeWhiteBoardDrawView != null) {
            beeWhiteBoardDrawView.prepage();
        }
    }

    public void nextpage() {
        if (beeWhiteBoardDrawView != null) {
            beeWhiteBoardDrawView.nextpage();
        }
    }

    public void drawText(float x, float y, int strokeColor, int strokeWidth, BeeWhiteBoardDefine.BeeDrawingMode mode, String data, String from) {
        if (beeWhiteBoardDrawView != null) {
            beeWhiteBoardDrawView.drawText(x, y, strokeColor, strokeWidth, mode, data, from);
        }
    }

    public void lineBegin(float x, float y, int strokeColor, int strokeWidth, BeeWhiteBoardDefine.BeeDrawingMode mode, String from) {
        if (beeWhiteBoardDrawView != null) {
            beeWhiteBoardDrawView.lineBegin(x, y, strokeColor, strokeWidth, mode, from);
        }
    }

    public void lineMove(float x, float y, BeeWhiteBoardDefine.BeeDrawingMode mode, String from) {
        if (beeWhiteBoardDrawView != null) {
            beeWhiteBoardDrawView.lineMove(x, y, mode, from);
        }
    }

    public void lineEnd(String from) {
        if (beeWhiteBoardDrawView != null) {
            beeWhiteBoardDrawView.lineEnd(from);
        }
    }

    public void setOriginBoardSize(float width, float height) {
        if (beeWhiteBoardDrawView != null) {
            beeWhiteBoardDrawView.setOriginBoardSize(width, height);
        }
    }

    public float getxZoom() {
        if (beeWhiteBoardDrawView != null) {
            return beeWhiteBoardDrawView.getxZoom();
        } else {
            return 0.0f;
        }
    }

    public float getyZoom() {
        if (beeWhiteBoardDrawView != null) {
            return beeWhiteBoardDrawView.getyZoom();
        } else {
            return 0.0f;
        }
    }

    public void setImgOfferValue(float offerX, float offerY, float width, float height) {
        if (beeWhiteBoardDrawView != null) {
            beeWhiteBoardDrawView.setImgOfferValue(offerX, offerY, width, height);
        }
    }

    public void setBoardInfo(boolean doc, HashMap<Integer, BeeWhiteBoardPage> pageMap, int pageNow) {
        if (beeWhiteBoardDrawView != null) {
            beeWhiteBoardDrawView.setBoardInfo(doc, pageMap, pageNow);
        }
    }

    public void remove(String uid) {
        if (beeWhiteBoardDrawView != null) {
            beeWhiteBoardDrawView.remove(uid);
        }
    }

    /**
     * @brief   关闭绘制，默认为打开.
     */
    public void lockDrawing() {
        this.post(new Runnable() {
            @Override
            public void run() {
                isDrawLock = true;
            }
        });

        if (beeWhiteBoardDrawView != null) {
            beeWhiteBoardDrawView.setDrawLock(true);
        }
    }

    /**
     * @brief   打开绘制，默认为打开.
     */
    public void unlockDrawing() {
        if (beeWhiteBoardDrawView != null) {
            beeWhiteBoardDrawView.setDrawLock(false);
        }
    }

    /**
     * @brief   重置白板View的显示位置为初始位置.
     */
    public void resetFrame() {
        this.post(new Runnable() {
            @Override
            public void run() {
                setScaleX(1.0f);
                setScaleY(1.0f);
                setTranslationX(0);
                setTranslationY(0);
                requestLayout();
            }
        });
    }

    /**
     * @brief   播放一个视频.
     * @param   url         视频url.
     * @param   x           视频显示位置x.
     * @param   y           视频显示位置y.
     * @param   width       视频显示宽度.
     * @param   height      视频显示高度.
     */
    public void playVideo(final String url, final int x, final int y, final int width, final int height) {
        this.post(new Runnable() {
            @Override
            public void run() {
                if (beeWhiteBoardVideoView == null) {
                    beeWhiteBoardVideoView = new BeeWhiteBoardVideoView(context);

                    FrameLayout layout = new FrameLayout(context);
                    FrameLayout.LayoutParams layoutParams = new FrameLayout.LayoutParams(WRAP_CONTENT, WRAP_CONTENT);
                    layoutParams.width = BeeSystemFuncion.dp2px(context, width);
                    layoutParams.height = BeeSystemFuncion.dp2px(context, height);
                    layout.setBackgroundColor(Color.BLACK);
                    layout.setLayoutParams(layoutParams);
                    layout.setX(BeeSystemFuncion.dp2px(context, x));
                    layout.setY(BeeSystemFuncion.dp2px(context, y));
                    layout.addView(beeWhiteBoardVideoView);
                    addView(layout);

                    FrameLayout.LayoutParams params = (FrameLayout.LayoutParams) beeWhiteBoardVideoView.getLayoutParams();
                    params.gravity = Gravity.CENTER;
                    beeWhiteBoardVideoView.setLayoutParams(params);
                }

                beeWhiteBoardVideoView.playVideo(url);
            }
        });

    }

    /**
     * @brief   停止播放当前视频.
     */
    public void stopVideo() {
        if (beeWhiteBoardVideoView != null) {
            beeWhiteBoardVideoView.stopVideo();
            this.removeView(beeWhiteBoardVideoView);
            beeWhiteBoardVideoView = null;
        }
    }

    /**
     * @brief   暂停播放当前视频.
     */
    public void pauseVideo() {
        if (beeWhiteBoardVideoView != null) {
            beeWhiteBoardVideoView.pauseVideo();
        }
    }

    /**
     * @brief   继续播放当前视频.
     */
    public void resumeVideo() {
        if (beeWhiteBoardVideoView != null) {
            beeWhiteBoardVideoView.resumeVideo();
        }
    }
}
