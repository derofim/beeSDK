package com.sohu.tv.bee;

import android.app.Service;
import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.Rect;
import android.media.AudioManager;
import android.media.MediaMetadataRetriever;
import android.os.Build;
import android.support.annotation.Nullable;
import android.util.AttributeSet;
import android.util.Log;
import android.view.KeyEvent;
import android.view.MotionEvent;
import android.view.ViewGroup;
import android.widget.VideoView;

import java.util.HashMap;

import static android.media.AudioManager.AUDIOFOCUS_GAIN;

public class BeeWhiteBoardVideoView extends VideoView{
    private static final int CURRENT_SDK_VERSION = android.os.Build.VERSION.SDK_INT;
    private Context context = null;
    private float scale = 1.0f;

    public BeeWhiteBoardVideoView(Context context) {
        super(context);
        init(context);
    }

    public BeeWhiteBoardVideoView(Context context, AttributeSet attrs) {
        super(context, attrs);
        init(context);
    }

    public BeeWhiteBoardVideoView(Context context, AttributeSet attrs, int defStyleAttr) {
        super(context, attrs, defStyleAttr);
        init(context);
    }

    private void init(Context context) {
        this.context = context;
        setFocusable(true);
        setFocusableInTouchMode(true);
    }

    @Override
    public boolean onKeyDown(int keyCode, KeyEvent event) {
        if (isPlaying()) {
            AudioManager audio = (AudioManager) context.getSystemService(Service.AUDIO_SERVICE);
            switch (keyCode) {
                case KeyEvent.KEYCODE_VOLUME_UP: {
                    audio.adjustStreamVolume(
                            AudioManager.STREAM_MUSIC,
                            AudioManager.ADJUST_RAISE,
                            AudioManager.FLAG_PLAY_SOUND | AudioManager.FLAG_SHOW_UI);
                }
                return true;
                case KeyEvent.KEYCODE_VOLUME_DOWN: {
                    audio.adjustStreamVolume(
                            AudioManager.STREAM_MUSIC,
                            AudioManager.ADJUST_LOWER,
                            AudioManager.FLAG_PLAY_SOUND | AudioManager.FLAG_SHOW_UI);
                }
                return true;
            }
        }

        return super.onKeyDown(keyCode, event);
    }

    public void playVideo(final String url) {
        this.post(new Runnable() {
            @Override
            public void run() {
                setVideoPath(url);
                requestFocus();
                start();
            }
        });
    }

    public void stopVideo() {
        this.post(new Runnable() {
            @Override
            public void run() {
                stopPlayback();
            }
        });
    }

    public void pauseVideo() {
        this.post(new Runnable() {
            @Override
            public void run() {
                pause();
            }
        });
    }

    public void resumeVideo() {
        this.post(new Runnable() {
            @Override
            public void run() {
                resume();
            }
        });
    }

    public void setScale(float scale) {
        this.scale = scale;
    }

    /*@Override
    protected void onMeasure(int widthMeasureSpec, int heightMeasureSpec) {
        if (Build.VERSION.SDK_INT <= Build.VERSION_CODES.N) {
            super.onMeasure(widthMeasureSpec, heightMeasureSpec);
            //设置宽高
            int defaultWidth = getDefaultSize(width, widthMeasureSpec);
            int defaultHeight = getDefaultSize(height, heightMeasureSpec);
            setMeasuredDimension(defaultWidth,defaultHeight);
        } else {
            super.onMeasure(widthMeasureSpec, heightMeasureSpec);
        }
    }*/


}
