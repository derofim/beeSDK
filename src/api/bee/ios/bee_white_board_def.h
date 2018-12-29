/**
 *  @file        bee_white_board_def.h
 *  @brief       BeeSDK白板公用声明文件.
 *  @author      HeZhen
 *  @copyright   sohu-inc
 *  @version     2.0.0.1
 */

#ifndef __BEE_WHITE_BOARD_DEF_H__
#define __BEE_WHITE_BOARD_DEF_H__

#include "bee/base/bee_define.h"

namespace bee {

/// 白板业务码.
const bee_int32_t kBeeSvcType_WhiteBoard = 2;

/// 白板数据类型.
typedef enum WhiteBoardMsgType {
    eWhiteBoardMsgType_Local_Join = 0,      //!< 本地用户加入.
    eWhiteBoardMsgType_Remote_Join,         //!< 远端用户加入.
    eWhiteBoardMsgType_Local_Leave,         //!< 本地用户离开.
    eWhiteBoardMsgType_Remote_Leave,        //!< 远端用户离开.
    eWhiteBoardMsgType_Closed,              //!< 已经关闭.
    eWhiteBoardMsgType_Message,             //!< 白板消息.
    eWhiteBoardMsgType_Count                //!< 白板数据类型数量.
}WhiteBoardMsgType;

/// 白板绘制模式.
typedef enum BeeDrawingMode {
    eBeeDrawingMode_Pen,                    //!< 画笔.
    eBeeDrawingMode_Eraser,                 //!< 橡皮擦.
    eBeeDrawingMode_Laser,                  //!< 激光笔.
    eBeeDrawingMode_Text,                   //!< 文本.
    eBeeDrawingMode_None                    //!< 无.
}BeeDrawingMode;

/// 白板的撤销、重做栈类型.
typedef enum BeeWhiteBoardStack {
    eBeeWhiteBoardStack_Undo,               //!< 撤销栈.
    eBeeWhiteBoardStack_Redo                //!< 重做栈.
}BeeWhiteBoardStack;

/// 白板角色类型.
typedef enum BeeWhiteBoardRole {
    eBeeWhiteBoardRole_None = 0,            //!< 无.
    eBeeWhiteBoardRole_Teacher,             //!< 老师.
    eBeeWhiteBoardRole_Student              //!< 学生.
}BeeWhiteBoardRole;

#define UIColorFromRGBA(rgbValue, alphaValue) [UIColor \
colorWithRed:((float)((rgbValue & 0xFF0000) >> 16))/255.0 \
green:((float)((rgbValue & 0x00FF00) >> 8))/255.0 \
blue:((float)(rgbValue & 0x0000FF))/255.0 \
alpha:alphaValue]

#define UIColorFromRGB(rgbValue) UIColorFromRGBA(rgbValue, 1.0)

} // namespace bee

#endif //__BEE_WHITE_BOARD_DEF_H__
