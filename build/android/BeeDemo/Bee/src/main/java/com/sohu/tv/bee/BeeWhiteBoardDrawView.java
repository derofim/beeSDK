/**
 *  @file        BeeWhiteBoardView.java
 *  @brief       BeeSDK白板View声明文件.
 *  @author      fangdali
 *  @copyright   sohu-inc
 *  @version     2.0.0.1
 */
package com.sohu.tv.bee;

import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.DrawFilter;
import android.graphics.Matrix;
import android.graphics.Paint;
import android.graphics.PorterDuff;
import android.graphics.drawable.BitmapDrawable;
import android.os.Handler;
import android.util.AttributeSet;
import android.util.Log;
import android.view.MotionEvent;
import android.view.View;
import android.widget.ImageView;
import android.widget.VideoView;

import com.sohu.tv.bee.writeBoard.ActionTypeEnum;
import com.sohu.tv.bee.writeBoard.DrawChannel;
import com.sohu.tv.bee.writeBoard.action.MyEraser;
import com.sohu.tv.bee.writeBoard.action.MyPath;
import com.sohu.tv.bee.writeBoard.action.Action;
import com.sohu.tv.bee.BeeWhiteBoardView.EventMode;

import java.lang.ref.WeakReference;
import java.util.HashMap;
import java.util.Iterator;
import java.util.Map;
import com.sohu.tv.bee.writeBoard.action.MyText;

import org.webrtc.ThreadUtils;

import static com.sohu.tv.bee.BeeSystemFuncion.convertRGBToARGB;
import static com.sohu.tv.bee.BeeWhiteBoardDefine.BeeDrawingMode.eBeeDrawingMode_Eraser;
import static com.sohu.tv.bee.BeeWhiteBoardDefine.BeeDrawingMode.eBeeDrawingMode_Laser;
import static com.sohu.tv.bee.BeeWhiteBoardDefine.BeeDrawingMode.eBeeDrawingMode_Pen;
import static com.sohu.tv.bee.BeeWhiteBoardDefine.BeeDrawingMode.eBeeDrawingMode_Text;

/// BeeSDK白板View类.
public class BeeWhiteBoardDrawView extends View {
    private static final String TAG = BeeWhiteBoardDrawView.class.getSimpleName();
    private Context context = null;
    private int bgColor = Color.WHITE; // 背景颜色
    private int paintColor = Color.BLACK; // 默认画笔颜色
    private int paintSize = 5;
    private Map<String, DrawChannel> otherDrawChannelMap = new HashMap<>();
    private boolean isDoc = false;
    private int boardPageCount = 0;
    private HashMap<Integer, BeeWhiteBoardPage> boardPageMap = new HashMap<>();
    private int currentPage = 0;
    private EventMode mode = EventMode.EVENT_MODE_NONE; // 1:draw 2:scale and translation
    private float lastX = 0.0f; //移动的上一次位置
    private float lastY = 0.0f; //移动的上一次位置
    private float originBoardWidth = 0.0f;
    private float originBoardHeight = 0.0f;
    private int localBoardWidth = 0;
    private int localBoardHeight = 0;
    private float localImgOfferX = 0.0f;
    private float localImgOfferY = 0.0f;
    private float originImgWidth = 0.0f;
    private float originImgHeight = 0.0f;
    private float originImgOfferX = 0.0f;
    private float originImgOfferY = 0.0f;

    private float xZoom = 1.0f;     // 收发数据时缩放倍数（归一化）
    private float yZoom = 1.0f;
    private float laserX = 0.0f;    //激光笔位置
    private float laserY = 0.0f;    //激光笔位置

    private int paintType = -1;
    private Canvas canvas = null;
    private Bitmap pptBitmap = null;
    private Bitmap mBitmap = null;
    private Bitmap laserBitmap = null;
    private DrawChannel drawChannel;
    private boolean laserExist = false;
    private boolean isDrawLock = false; //未锁定，可以绘制

    private WeakReference<BeeWhiteBoardViewDelegate> beeWhiteBoardViewDelegate = null;

    public BeeWhiteBoardDrawView(Context context) {
        super(context);
        init(context);
    }

    public BeeWhiteBoardDrawView(Context context, AttributeSet attrs, int defStyle) {
        super(context, attrs, defStyle);
        init(context);
    }

    public BeeWhiteBoardDrawView(Context context, AttributeSet atts) {
        super(context, atts);
        init(context);
    }

    private void init(Context context) {
        this.context = context;
        this.drawChannel = new DrawChannel();
        this.drawChannel.setColor(paintColor);
        initLaserPen();
        setBackgroundColor(bgColor);
        clearAll();
    }

    private void initLaserPen() {
        laserBitmap = Bitmap.createBitmap(20, 20, Bitmap.Config.ARGB_8888);
        Canvas canvas = new Canvas(laserBitmap);
        Paint paint = new Paint();
        paint.setStyle(Paint.Style.FILL);
        paint.setAntiAlias(true);//去除抗锯齿
        paint.setColor(Color.RED);
        canvas.drawCircle(10, 10, 10, paint);
    }

    @Override
    protected void onSizeChanged(int w, int h, int oldw, int oldh) {
        super.onSizeChanged(w, h, oldw, oldh);
        localBoardWidth = getMeasuredWidth();
        localBoardHeight = getMeasuredHeight();
        if (localBoardWidth !=0 && localBoardHeight != 0) {
            mBitmap = Bitmap.createBitmap(localBoardWidth, localBoardHeight, Bitmap.Config.ARGB_8888);
            canvas = new Canvas(mBitmap);

            setZoom(originBoardWidth, originBoardHeight);
        }
    }

    public void onResume() {
        new Handler(context.getMainLooper()).postDelayed(new Runnable() {
            @Override
            public void run() {
                loadWhiteBoard();
            }
        }, 50);
    }

    public void setWhiteBoard(BeeWhiteBoardViewDelegate delegate) {
        this.beeWhiteBoardViewDelegate = new WeakReference<>(delegate);
    }

    public void setLineColor(final int color) {
        this.post(new Runnable() {
            @Override
            public void run() {
                drawChannel.paintColor = color;
            }
        });
    }

    public void setLineWidth(final int size) {
        this.post(new Runnable() {
            @Override
            public void run() {
                if (size > 0) {
                    paintSize = size;
                    drawChannel.setSize(size);
                }
            }
        });
    }

    public void setDrawingMode(final BeeWhiteBoardDefine.BeeDrawingMode mode) {
        this.post(new Runnable() {
            @Override
            public void run() {
                drawChannel.drawingMode = mode;
            }
        });
    }

    public void setDrawLock(final boolean b) {
        this.post(new Runnable() {
            @Override
            public void run() {
                isDrawLock = b;
            }
        });
    }

    @Override
    protected void onDraw(Canvas canvas) {
        if (isDoc) {
            if (null != pptBitmap) {
                canvas.drawBitmap(pptBitmap, originImgOfferX * xZoom, originImgOfferY * yZoom, null);
            }
        }

        if (null != mBitmap) {
            canvas.drawBitmap(mBitmap, 0, 0, null);
        }

        if (laserExist) {
            if (null != laserBitmap) {
                canvas.drawBitmap(laserBitmap, laserX, laserY, null);
            }
        }
        super.onDraw(canvas);
    }

    public void onTouchBegin(float x, float y) {
        if (true == isDrawLock || drawChannel == null || (Math.abs(x - lastX) <= 0.1f && Math.abs(y - lastY) <= 0.1f)) {
            return;
        }

        if (drawChannel.drawingMode == eBeeDrawingMode_Laser) {
            laserX = x;
            laserY = y;
            laserExist = true;
            return;
        }

        onDrawLineBegin(x, y, drawChannel.paintColor, drawChannel.paintSize, drawChannel.drawingMode, true, null);
        BeeWhiteBoardViewDelegate delegate = beeWhiteBoardViewDelegate.get();
        if (delegate != null) {
            //delegate.onDrawingLine(lastX / xZoom, lastY / yZoom, x / xZoom, y / yZoom, drawChannel.paintColor, drawChannel.paintSize, drawChannel.drawingMode);
        }

        lastX = x;
        lastY = y;
        laserExist = false;
    }

    public void onTouchMove(float x, float y) {
        if (true == isDrawLock || drawChannel == null || drawChannel.action == null) {
            return;
        }

        if (Math.abs(x - lastX) <= 0.1f && Math.abs(y - lastY) <= 0.1f) {
            return;
        }

        if (drawChannel.drawingMode == eBeeDrawingMode_Laser) {
            laserX = x;
            laserY = y;
            laserExist = true;
            return;
        }

        onDrawLineMove(x, y, drawChannel.drawingMode, true, null);
        BeeWhiteBoardViewDelegate delegate = beeWhiteBoardViewDelegate.get();
        if (delegate != null) {
            if (isDoc) {
                delegate.onDrawingLine((lastX - originImgOfferX * xZoom) / xZoom, (lastY - originImgOfferY * yZoom) / yZoom, (x - originImgOfferX * xZoom) / xZoom, (y - originImgOfferY * yZoom) / yZoom, drawChannel.paintColor, drawChannel.paintSize, drawChannel.drawingMode);
            } else {
                delegate.onDrawingLine(lastX / xZoom, lastY / yZoom, x / xZoom, y / yZoom, drawChannel.paintColor, drawChannel.paintSize, drawChannel.drawingMode);
            }
        }
        lastX = x;
        lastY = y;
        laserExist = false;
    }

    public void onTouchEnd() {
        if (true == isDrawLock || drawChannel == null || drawChannel.action == null || drawChannel.drawingMode == eBeeDrawingMode_Laser) {
            return;
        }

        onDrawLineEnd(true, null);
        BeeWhiteBoardViewDelegate delegate = beeWhiteBoardViewDelegate.get();
        if (delegate != null) {
            delegate.onDrawLineEnd();
        }
    }

    //滑动不向上传递
    @Override
    public boolean dispatchTouchEvent(MotionEvent event) {
        getParent().requestDisallowInterceptTouchEvent(true);
        return super.dispatchTouchEvent(event);
    }

    private DrawChannel getOtherChannel(String account) {
        DrawChannel otherChannel = null;
        if (otherDrawChannelMap.get(account) == null) {
            otherChannel = new DrawChannel();
            otherChannel.paintSize = this.paintSize;
            otherChannel.setType(this.paintType);
            otherDrawChannelMap.put(account, otherChannel);
        } else {
            otherChannel = otherDrawChannelMap.get(account);
        }

        return otherChannel;
    }

    private void loadWhiteBoard() {
        ThreadUtils.checkIsOnMainThread();
        if (false == isDoc) {
            setBackgroundColor(bgColor);
            drawHistoryActions(canvas);
            invalidate();
        } else {
            loadFromPage(currentPage);
        }
    }

    private BeeWhiteBoardPage getBoardPage(int nPage) {
        return boardPageMap.get(nPage);
    }


    private void drawHistoryActions(Canvas canvas) {
        ThreadUtils.checkIsOnMainThread();
        if (canvas == null) {
            return;
        }
        //清空画板
        canvas.drawColor(Color.TRANSPARENT, PorterDuff.Mode.CLEAR);

        // 绘制所有历史Action
        BeeWhiteBoardPage beeWhiteBoardPage = getBoardPage(currentPage);
        if (beeWhiteBoardPage != null) {
            if (beeWhiteBoardPage.getUndo_actions() != null) {
                for (Action a : beeWhiteBoardPage.getUndo_actions()) {
                    a.onDraw(canvas);
                }
            }
        }

        Map<String, DrawChannel> tempMap = new HashMap<>();
        tempMap.putAll(otherDrawChannelMap);
        Iterator<Map.Entry<String, DrawChannel>> entries = tempMap.entrySet().iterator();
        while (entries.hasNext()) {
            Map.Entry<String, DrawChannel> entry = entries.next();
            DrawChannel otherChannel = entry.getValue();
            if (otherChannel != null && otherChannel.action != null) {
                otherChannel.action.onDraw(canvas);
            }
        }

        // 绘制本地当前操作
        if (drawChannel != null && drawChannel.action != null) {
            drawChannel.action.onDraw(canvas);
        }
    }

    private void onDrawLineBegin(float x, float y, int strokeColor, int strokeWidth, BeeWhiteBoardDefine.BeeDrawingMode mode, boolean local, String session) {
        ThreadUtils.checkIsOnMainThread();
        BeeWhiteBoardPage beeWhiteBoardPage = getBoardPage(currentPage);
        if (beeWhiteBoardPage == null)
            return;

        DrawChannel channel = null;
        if (local) {
            channel = drawChannel;
        } else {
            channel = getOtherChannel(session);
        }

        laserExist = false;
        if (mode == BeeWhiteBoardDefine.BeeDrawingMode.eBeeDrawingMode_Laser) {
            laserExist = true;
        }

        if (mode == eBeeDrawingMode_Laser) {
            laserX = x;
            laserY = y;
        } else if (mode == eBeeDrawingMode_Pen) {
            channel.setType(ActionTypeEnum.Path.getValue());
            channel.setSize(strokeWidth);
            channel.setColor(convertRGBToARGB(strokeColor));

            if (channel.action != null) {
                beeWhiteBoardPage.pushUndo(channel.action);
                channel.action = null;
            }

            channel.action = new MyPath(x, y, channel.paintColor, channel.paintSize);
            channel.drawingMode = eBeeDrawingMode_Pen;
            channel.action.onStart(canvas);
        } else if (mode == eBeeDrawingMode_Eraser) {
            channel.setEraseType(this.bgColor, strokeWidth);

            if (channel.action != null) {
                beeWhiteBoardPage.pushUndo(channel.action);
                channel.action = null;
            }

            channel.action = new MyEraser(x, y, channel.paintColor, channel.paintSize);
            channel.drawingMode = eBeeDrawingMode_Eraser;
            channel.action.onStart(canvas);
        }

        invalidate();
    }

    private void onDrawLineMove(float x, float y, BeeWhiteBoardDefine.BeeDrawingMode mode, boolean local, String session) {
        ThreadUtils.checkIsOnMainThread();
        BeeWhiteBoardPage beeWhiteBoardPage = getBoardPage(currentPage);
        if (beeWhiteBoardPage == null)
            return;

        DrawChannel channel = null;
        if (local) {
            channel = drawChannel;
        } else {
            channel = getOtherChannel(session);
        }

        laserExist = false;
        if (mode == BeeWhiteBoardDefine.BeeDrawingMode.eBeeDrawingMode_Laser) {
            laserExist = true;
        }

        if (mode == eBeeDrawingMode_Laser) {
            laserX = x;
            laserY = y;
        } else if (mode == eBeeDrawingMode_Pen) {
            channel.drawingMode = eBeeDrawingMode_Pen;
            if (null != channel.action) {
                channel.action.onMove(x, y);
                channel.action.onDraw(canvas);
            }
        } else if (mode == eBeeDrawingMode_Eraser) {
            channel.drawingMode = eBeeDrawingMode_Eraser;
            if (null != channel.action) {
                channel.action.onMove(x, y);
                channel.action.onDraw(canvas);
            }
        }

        invalidate();
    }

    private void onDrawLineEnd(boolean local, String session) {
        ThreadUtils.checkIsOnMainThread();
        BeeWhiteBoardPage beeWhiteBoardPage = getBoardPage(currentPage);
        if (beeWhiteBoardPage == null) {
            return;
        }

        DrawChannel channel = null;
        if (local) {
            channel = drawChannel;
        } else {
            channel = getOtherChannel(session);
        }

        if (channel.action == null) {
           return;
        }

        beeWhiteBoardPage.pushUndo(channel.action);
        beeWhiteBoardPage.clearRedoStack();
        channel.action = null;
    }

    private void onDrawText(float x, float y, int strokeColor, int strokeWidth, BeeWhiteBoardDefine.BeeDrawingMode mode, String data, boolean local, String session) {
        ThreadUtils.checkIsOnMainThread();
        BeeWhiteBoardPage beeWhiteBoardPage = getBoardPage(currentPage);
        if (beeWhiteBoardPage == null)
            return;

        DrawChannel channel = null;
        if (local) {
            channel = drawChannel;
        } else {
            channel = getOtherChannel(session);
        }

        channel.setSize(strokeWidth);
        channel.setColor(convertRGBToARGB(strokeColor));

        if (channel.action != null) {
            beeWhiteBoardPage.pushUndo(channel.action);
            channel.action = null;
        }
        channel.action = new MyText(x, y, channel.paintColor, channel.paintSize, data);
        channel.drawingMode = eBeeDrawingMode_Text;
        channel.action.onStart(canvas);
        channel.action.onDraw(canvas);

        invalidate();
    }

    private void loadFromPage(final int pageIndex) {
        ThreadUtils.checkIsOnMainThread();
        if (boardPageMap != null && boardPageMap.size() > 0 && pageIndex >=0 && pageIndex < boardPageMap.size()) {
            BeeWhiteBoardPage beeWhiteBoardPage = getBoardPage(pageIndex);
            beeWhiteBoardPage.getDocFromUrl(context, new BeeAsyncHandler() {
                @Override
                public void LoadDocHandler(BeeSystemDefine.BeeErrorCode code, Bitmap bitmap, String err) {
                    if (code != BeeSystemDefine.BeeErrorCode.kBeeErrorCode_Success) {
                        Log.e(TAG, err);
                        return;
                    }

                    int width = bitmap.getWidth();
                    int height = bitmap.getHeight();
                    float scaleW = localBoardWidth / (width * 1.0f);
                    float scaleH = localBoardHeight / (height * 1.0f);
                    float scale = Math.min(scaleW, scaleH);

                    Matrix matrix = new Matrix();
                    matrix.postScale(scale, scale);
                    if (pptBitmap != null && pptBitmap.isRecycled()) {
                        pptBitmap.recycle();
                        pptBitmap = null;
                    }
                    pptBitmap = Bitmap.createBitmap(bitmap, 0, 0, width,
                            height, matrix, true);

                    /*BitmapDrawable bd = new BitmapDrawable(getResources(), bitmap);
                    setBackground(bd);*/

                    drawHistoryActions(canvas);
                    postInvalidate();
                }
            });
        }
    }

    private void clear(DrawChannel otherChannel, boolean isPaintView) {
        DrawChannel channel = isPaintView ? drawChannel : otherChannel;
        if (channel == null) {
            return;
        }

        BeeWhiteBoardPage beeWhiteBoardPage = getBoardPage(currentPage);
        if (beeWhiteBoardPage != null) {
            beeWhiteBoardPage.clearUndoStack();
            beeWhiteBoardPage.clearRedoStack();
        }
        channel.action = null;
        drawHistoryActions(canvas);
        invalidate();
    }

    public void remove(final String uid) {
        this.post(new Runnable() {
            @Override
            public void run() {
                if (otherDrawChannelMap.get(uid) != null) {
                    otherDrawChannelMap.remove(uid);
                }
            }
        });
    }

    public void setOriginBoardSize(float width, float height) {
        originBoardWidth = width;
        originBoardHeight = height;
        setZoom(originBoardWidth, originBoardHeight);
    }

    public void setZoom(float width, float height) {
        xZoom = (localBoardWidth*1.0f)/width;
        yZoom = (localBoardHeight*1.0f)/height;
    }

    public float getxZoom() {
        return xZoom;
    }

    public float getyZoom() {
        return yZoom;
    }

    public void setImgOfferValue(float offerX, float offerY, float width, float height) {
        originImgWidth = width;
        originImgHeight = height;
        originImgOfferX = offerX;
        originImgOfferY = offerY;

        localImgOfferX = originImgOfferX * xZoom;
        localImgOfferY = originImgOfferY * yZoom;
    }

    public void clearAll() {
        this.post(new Runnable() {
            @Override
            public void run() {
                for (Map.Entry<String, DrawChannel> entry : otherDrawChannelMap.entrySet()) {
                    clear(entry.getValue(), false);
                }

                clear(drawChannel, true);
            }
        });
    }

    public void undo() {
        this.post(new Runnable() {
            @Override
            public void run() {
                BeeWhiteBoardPage beeWhiteBoardPage = getBoardPage(currentPage);
                if (beeWhiteBoardPage != null) {
                    beeWhiteBoardPage.pushRedo(beeWhiteBoardPage.popUndo());
                    drawHistoryActions(canvas);
                    invalidate();
                }
            }
        });
    }

    public void redo() {
        this.post(new Runnable() {
            @Override
            public void run() {
                BeeWhiteBoardPage beeWhiteBoardPage = getBoardPage(currentPage);
                if (beeWhiteBoardPage != null) {
                    beeWhiteBoardPage.pushUndo(beeWhiteBoardPage.popRedo());
                    drawHistoryActions(canvas);
                    invalidate();
                }
            }
        });
    }

    public void prepage() {
        this.post(new Runnable() {
            @Override
            public void run() {
                if (isDoc && currentPage > 0) {
                    currentPage--;
                    loadFromPage(currentPage);
                }
            }
        });
    }

    public void nextpage() {
        this.post(new Runnable() {
            @Override
            public void run() {
                if (isDoc && currentPage < boardPageCount - 1) {
                    currentPage++;
                    loadFromPage(currentPage);
                }
            }
        });
    }

    public int getBoardPageNow() {
        return currentPage;
    }

    public boolean getisDoc() {
        return isDoc;
    }

    public void lineBegin(final float x, final float y, final int strokeColor, final int strokeWidth, final BeeWhiteBoardDefine.BeeDrawingMode mode, final String from) {
        this.post(new Runnable() {
            @Override
            public void run() {
                if (isDoc) {
                    onDrawLineBegin((x + originImgOfferX) * xZoom, (y + originImgOfferY) * yZoom, strokeColor, strokeWidth, mode, false, from);
                } else {
                    onDrawLineBegin(x * xZoom, y * yZoom, strokeColor, strokeWidth, mode, false, from);
                }
            }
        });
    }

    public void lineMove(final float x, final float y, final BeeWhiteBoardDefine.BeeDrawingMode mode, final String from) {
        this.post(new Runnable() {
            @Override
            public void run() {
                if (isDoc) {
                    onDrawLineMove((x + originImgOfferX) * xZoom, (y + originImgOfferY) * yZoom, mode, false, from);
                } else {
                    onDrawLineMove(x * xZoom, y * yZoom, mode, false, from);
                }
            }
        });
    }

    public void lineEnd(final String from) {
        this.post(new Runnable() {
            @Override
            public void run() {
                onDrawLineEnd(false, from);
            }
        });
    }

    public void drawText(final float x, final float y, final int strokeColor, final int strokeWidth, final BeeWhiteBoardDefine.BeeDrawingMode mode, final String data, final String from) {
        this.post(new Runnable() {
            @Override
            public void run() {
                if (isDoc) {
                    onDrawText((x + originImgOfferX) * xZoom, (y + originImgOfferY) * yZoom, strokeColor, strokeWidth, mode, data, false, from);
                } else {
                    onDrawText(x * xZoom, y * yZoom, strokeColor, strokeWidth, mode, data, false, from);
                }
            }
        });
    }

    public void setBoardInfo(final boolean doc, final HashMap<Integer, BeeWhiteBoardPage> pageMap, final int PageNow) {
        this.post(new Runnable() {
            @Override
            public void run() {
                isDoc = doc;
                currentPage = PageNow;
                boardPageMap = pageMap;
                boardPageCount = pageMap.size();

                loadWhiteBoard();
            }
        });
    }
}
