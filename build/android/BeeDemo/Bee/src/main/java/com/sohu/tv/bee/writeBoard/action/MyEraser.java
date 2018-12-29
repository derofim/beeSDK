package com.sohu.tv.bee.writeBoard.action;

import android.graphics.Canvas;
import android.graphics.Paint;
import android.graphics.Path;
import android.graphics.PorterDuff;
import android.graphics.PorterDuffXfermode;

/**
 * 橡皮擦（与画布背景色相同的Path）
 * <p/>
 * Created by Administrator on 2015/6/24.
 */
public class MyEraser extends Action {
    private Path path;
    private Paint mEraserPaint;
    private Float x;
    private Float y;

    public MyEraser(Float x, Float y, Integer color, Integer size) {
        super(x, y, color, size);
        this.x = x;
        this.y = y;
        path = new Path();
        path.moveTo(x, y);
        path.lineTo(x, y);
    }

    @Override
    public boolean isSequentialAction() {
        return true;
    }

    public void onDraw(Canvas canvas) {
        if (canvas == null) {
            return;
        }

        if (mEraserPaint == null) {
            mEraserPaint = new Paint();
            mEraserPaint.setColor(color);
            mEraserPaint.setAlpha(0);
            mEraserPaint.setXfermode(new PorterDuffXfermode(PorterDuff.Mode.DST_IN));
            mEraserPaint.setAntiAlias(true);
            mEraserPaint.setDither(true);
            mEraserPaint.setStyle(Paint.Style.STROKE);
            mEraserPaint.setStrokeJoin(Paint.Join.ROUND);
            mEraserPaint.setStrokeWidth(size);
        }

        canvas.drawPath(path, mEraserPaint);
        canvas.drawPoint(x, y, mEraserPaint);
    }

    public void onMove(float mx, float my) {
        path.lineTo(mx, my);
    }
}
