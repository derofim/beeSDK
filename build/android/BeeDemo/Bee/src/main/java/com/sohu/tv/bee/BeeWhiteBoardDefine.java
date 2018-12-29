/**
 *  @file        BeeWhiteBoardDefine.java
 *  @brief       BeeSDK白板公用声明文件.
 *  @author      fangdali
 *  @copyright   sohu-inc
 *  @version     2.0.0.1
 */
package com.sohu.tv.bee;

/// 白板业务定义
public class BeeWhiteBoardDefine {
    /// 白板业务码.
    public static final int kBeeSvcType_WhiteBoard = 2;

    /// 白板数据类型.
    public enum WhiteBoardMsgType {
        eWhiteBoardMsgType_Local_Join,          //!< 本地用户加入.
        eWhiteBoardMsgType_Remote_Join,         //!< 远端用户加入.
        eWhiteBoardMsgType_Local_Leave,         //!< 本地用户离开.
        eWhiteBoardMsgType_Remote_Leave,        //!< 远端用户离开.
        eWhiteBoardMsgType_Closed,              //!< 已经关闭.
        eWhiteBoardMsgType_Message,             //!< 白板消息.
        eWhiteBoardMsgType_Count                //!< 白板数据类型数量.
    }

    /// 白板绘制模式.
    public enum BeeDrawingMode {
        eBeeDrawingMode_Pen,                    //!< 画笔.
        eBeeDrawingMode_Eraser,                 //!< 橡皮擦.
        eBeeDrawingMode_Laser,                  //!< 激光笔.
        eBeeDrawingMode_Text,                   //!< 文本.
        eBeeDrawingMode_None                    //!< 无.
    }

    /// 白板的撤销、重做栈类型.
    public enum BeeWhiteBoardStack {
        eBeeWhiteBoardStack_Undo,               //!< 撤销栈.
        eBeeWhiteBoardStack_Redo                //!< 重做栈.
    }

    /// 白板角色类型.
    public enum BeeWhiteBoardRole {
        eBeeWhiteBoardRole_None,                //!< 无.
        eBeeWhiteBoardRole_Teacher,             //!< 老师.
        eBeeWhiteBoardRole_Student              //!< 学生.
    }
}
