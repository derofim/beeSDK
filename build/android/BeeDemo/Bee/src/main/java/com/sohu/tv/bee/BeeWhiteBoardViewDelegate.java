/**
 *  @file        BeeWhiteBoardViewDelegate.java
 *  @brief       BeeSDK白板View委托接口
 *  @author      fangdali
 *  @copyright   sohu-inc
 *  @version     2.0.0.1
 */
package com.sohu.tv.bee;

/// 白板View委托协议，可以获得本地绘制的轨迹信息.
public interface BeeWhiteBoardViewDelegate {
    /**
     *  @brief  本地绘制一条线段的通知.
     *  @param  x0          线段起始点x坐标.
     *  @param  y0          线段起始点y坐标.
     *  @param  x1          线段终点x坐标.
     *  @param  y1          线段终点y坐标.
     *  @param  color       线段的颜色.
     *  @param  width       线段的宽度.
     *  @param  mode        绘制模式.
     *  @note   一条曲线由若干连续的线段组成.
     */
    public void onDrawingLine(float x0, float y0, float x1, float y1, int color, int width, BeeWhiteBoardDefine.BeeDrawingMode mode);
    
    /**
     *  @brief  绘制当前曲线结束的通知.
     */
    public void onDrawLineEnd();
}
