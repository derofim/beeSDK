package com.sohu.tv.bee.writeBoard.action;

import android.graphics.Canvas;
import android.graphics.Paint;
import android.graphics.Typeface;

public class MyText extends Action {
    private String data = null;
    private float scale = 1.0f;

    public MyText(float startX, float startY, int color, int size, String data) {
        super(startX, startY, color, size);
        this.data = data;
    }

    @Override
    public void onMove(float mx, float my) {

    }

    @Override
    public void onDraw(Canvas canvas) {
        Paint paint = new Paint();
        paint.setAntiAlias(true);
        paint.setStyle(Paint.Style.FILL);
        paint.setColor(color);
        paint.setTextSize(size);
        paint.setTextAlign(Paint.Align.LEFT);
        paint.setTypeface(Typeface.SANS_SERIF);
        Paint.FontMetrics fontMetrics = paint.getFontMetrics();
        float fontHeight = fontMetrics.descent - fontMetrics.ascent;
        canvas.drawText(data, startX, stopY + fontHeight, paint);
    }
}
