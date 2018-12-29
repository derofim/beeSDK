/**
 *  @file        BeeWhiteBoardPage.java
 *  @brief       BeeSDK白板页声明文件.
 *  @author      fangdali
 *  @copyright   sohu-inc
 *  @version     2.0.0.1
 */
package com.sohu.tv.bee;

import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.drawable.BitmapDrawable;
import android.graphics.drawable.Drawable;
import android.widget.Toast;

import com.bumptech.glide.Glide;
import com.bumptech.glide.request.animation.GlideAnimation;
import com.bumptech.glide.request.target.SimpleTarget;
import com.sohu.tv.bee.writeBoard.action.Action;

import java.util.Collections;
import java.util.concurrent.CopyOnWriteArrayList;

/// BeeSDK白板页类.
public class BeeWhiteBoardPage {
    /// 白板页的所有者/绘制者.
    private String src;
    /// 白板页的文档url.
    private String url;
    /// 白板页原始宽.
    private float width;
    /// 白板页原始高.
    private float height;

    /// 撤销操作栈.
    private CopyOnWriteArrayList<Action> undo_actions = new CopyOnWriteArrayList<>();

    /// 重做操作栈.
    private CopyOnWriteArrayList<Action> redo_actions = new CopyOnWriteArrayList<>();

    /**
     *  @brief  白板页类构造函数.
     *  @param  src         白板页的所有者/绘制者.
     *  @param  url         白板页的文档url.
     *  @param  width       白板页原始宽.
     *  @param  height      白板页原始高.
     */
    public BeeWhiteBoardPage(String src, String url, float width, float height) {
        this.src = src;
        this.url = url;
        this.width = width;
        this.height = height;
    }

    /**
     *  @brief  缓存一个撤销操作.
     *  @param  action       撤销操作.
     */
    public void pushUndo(Action action) {
        if (null != action) {
            undo_actions.add(action);
        }
    }

    /**
     *  @brief  取出最近的一个撤销操作.
     *  @return 最近的一个撤销操作.
     */
    public Action popUndo() {
        if (undo_actions.size() == 0) {
            return null;
        }

        int lastIndex = undo_actions.size() - 1;
        Action action = undo_actions.get(lastIndex);
        undo_actions.remove(lastIndex);
        return action;
    }

    /**
     *  @brief  缓存一个重做操作.
     *  @param  action       重做操作.
     */
    public void pushRedo(Action action) {
        if (null != action) {
           redo_actions.add(action);
        }
    }

    /**
     *  @brief  取出最近的一个重做的操作.
     *  @return 最近的一个重做操作.
     */
    public Action popRedo() {
        if (redo_actions.size() == 0) {
            return null;
        }

        int lastIndex = redo_actions.size() - 1;
        Action action = redo_actions.get(lastIndex);
        redo_actions.remove(lastIndex);
        return action;
    }

    /**
     *  @brief  清除撤销操作栈.
     */
    public void clearUndoStack() {
        undo_actions.clear();
    }

    /**
     *  @brief  清除重做操作栈.
     */
    public void clearRedoStack() {
        redo_actions.clear();
    }

    /**
     *  @brief  重做操作栈反向排列.
     */
    public void reverseRedoStack() {
        Collections.reverse(redo_actions);
    }

    /**
     *  @brief  获取撤销操作.
     * @return  返回撤销操作.
     */
    public CopyOnWriteArrayList<Action> getUndo_actions() {
        return undo_actions;
    }

    /**
     *  @brief  根据url获取doc文档
     */
    public void getDocFromUrl(Context context, final BeeAsyncHandler handler) {
        Glide.with(context).load(url).asBitmap().into(new SimpleTarget<Bitmap>() {
            @Override
            public void onResourceReady(Bitmap resource, GlideAnimation<? super Bitmap> glideAnimation) {
                handler.LoadDocHandler(BeeSystemDefine.BeeErrorCode.kBeeErrorCode_Success, resource, "");
            }

            @Override
            public void onLoadFailed(Exception e, Drawable errorDrawable) {
                handler.LoadDocHandler(BeeSystemDefine.BeeErrorCode.kBeeErrorCode_Not_Found, null, e.getMessage());
            }
        });
    }
}
