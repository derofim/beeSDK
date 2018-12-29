/**
 *  @file        bee_white_board.h
 *  @brief       BeeSDK白板类声明文件.
 *  @author      HeZhen
 *  @copyright   sohu-inc
 *  @version     2.0.0.1
 */

#ifndef __BEE_WHITE_BOARD_H__
#define __BEE_WHITE_BOARD_H__

#import <Foundation/Foundation.h>
#import "bee/ios/bee_sdk_service.h"
#import "bee/ios/bee_white_board_sink.h"
#import "bee/ios/bee_white_board_view.h"

using namespace bee;

#pragma mark - BeeWhiteBoard

/// BeeSDK白板类.
@interface BeeWhiteBoard : BeeSDKService <BeeWhiteBoardViewDelegate>

/// 白板View.
@property(nonatomic, strong) BeeWhiteBoardView *view;

/**
 *  @brief  白板类构造函数.
 *  @param  view        白板View，用于绘制白板元素.
 *  @param  handle      会话句柄.
 *  @param  token       APP令牌，每个APP必须绑定一个令牌用于鉴权.
 *  @param  timeout     白板接口超时时间，单位ms.
 *  @param  sink        白板类接口回调对象.
 *  @note   白板类对象.
 */
- (instancetype)initWithParam:(BeeWhiteBoardView*)view
                       handle:(bee_handle)handle
                        token:(NSString*)token
                      timeout:(bee_int32_t)timeout
                         sink:(id<BeeWhiteBoardSink>)sink;

/**
 *  @brief  加入白板房间.
 *  @param  roomName    白板房间名.
 *  @param  uid         本地用户唯一标识.
 *  @param  nickName    本地用户昵称.
 *  @param  creator     是否是白板创建者.
 *  @param  role        白板角色.
 *  @note   结果通过[BeeWhiteBoardSink onJoin:msg:]返回.
 *  @see    [BeeWhiteBoardSink onJoin:msg:].
 */
- (void)join:(NSString*)roomName
         uid:(NSString*)uid
    nickName:(NSString*)nickName
     creator:(BOOL)creator
        role:(BeeWhiteBoardRole)role;

/**
 *  @brief  离开白板房间.
 *  @note   结果通过[BeeWhiteBoardSink onLeave:msg:]返回.
 *  @see    [BeeWhiteBoardSink onLeave:msg:].
 */
- (void)leave;

/**
 *  @brief  清除白板当前页屏幕.
 */
- (void)clearAll;

/**
 * @brief   取消白板上一次绘制.
 */
- (void)undo;

/**
 * @brief   重做白板上一次取消的绘制.
 */
- (void)redo;

/**
 * @brief   设置白板本地绘制模式.
 * @param   mode        绘制模式.
 */
- (void)setDrawingMode:(BeeDrawingMode)mode;

/**
 * @brief   设置白板本地绘制线条颜色.
 * @param   rgbColor    线条颜色.
 */
- (void)setLineColor:(int)rgbColor;

/**
 * @brief   设置白板本地绘制线条宽度.
 * @param   width       线条宽度.
 */
- (void)seLineWidth:(CGFloat)width;

/**
 * @brief   关闭白板绘制，默认打开.
 */
- (void)lockDrawing;

/**
 * @brief   打开白板绘制，默认打开.
 */
- (void)unlockDrawing;

/**
 * @brief   重置白板的显示位置为初始位置.
 */
- (void)resetFrame;

@end

#endif // #ifndef __BEE_WHITE_BOARD_H__
