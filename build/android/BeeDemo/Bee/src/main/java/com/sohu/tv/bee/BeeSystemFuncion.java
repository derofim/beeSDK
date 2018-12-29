/**
 *  @file        BeeSystemFuncion.java
 *  @brief       BeeSDK全局函数声明文件.
 *  @author      fangdali
 *  @copyright   sohu-inc
 *  @version     2.0.0.1
 */
package com.sohu.tv.bee;

import android.content.Context;

import com.sohu.tv.bee.BeeSystemDefine.BeeErrorCode;

/// BeeSDK全局函数类
public class BeeSystemFuncion {

    /**
     * @brief   获取错误码
     * @param   ret  错误码int类型
     * @return  错误码
     */
    public static BeeErrorCode toBeeErrorCode(int ret) {
        if (ret >= BeeErrorCode.kBeeErrorCode_Success.ordinal() && ret < BeeErrorCode.kBeeErrorCode_Count.ordinal()) {
            return BeeErrorCode.values()[ret];
        } else {
            return BeeErrorCode.kBeeErrorCode_Not_Compatible;
        }
    }

    /**
     * @brief   颜色rgb格式转argb格式
     * @param   rgb 颜色rgb格式
     * @return  argb颜色格式
     */
    public static int convertRGBToARGB(int rgb) {
        int r = (rgb >> 16) & 0xFF;
        int g = (rgb >> 8) & 0xFF;
        int b = (rgb >> 0) & 0xFF;

        return 0xff000000 | (r << 16) | (g << 8) | b;
    }

    /**
     * @brief   颜色rgb格式转argb格式
     * @param   argb 颜色argb格式
     * @return  rgb颜色格式
     */
    public static String convertARGBToRGB(int argb) {
        int rgb = 0x00ffffff & argb;
        String st = Integer.toHexString(rgb).toUpperCase();
        st = String.format("#%6s", st);
        st = st.replaceAll(" ", "0");
        return st;
    }

    /**
     * @brief   dp转px
     * @param   context android上下文
     * @param   dp       dp数据
     * @return  px数据
     */
    public static int dp2px(Context context, float dp) {
        float scale = context.getResources().getDisplayMetrics().density;
        return (int) (dp * scale + 0.5f);
    }

    /**
     * @brief   px转dp
     * @param   context android上下文
     * @param   px       px数据
     * @return  dp数据
     */
    public static int px2dp(Context context, float px) {
        float scale = context.getResources().getDisplayMetrics().density;
        return (int) (px / scale + 0.5f);
    }

    /**
     * @brief   px转sp
     * @param   context android上下文
     * @param   px      px数据
     * @return  sp数据
     */
    public static int px2sp(Context context,float px) {
        float fontScale=context.getResources().getDisplayMetrics().scaledDensity;
        return (int) (px/fontScale+0.5f);
    }
}
