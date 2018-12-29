/**
 *  @file        bee_white_board_sink.h
 *  @brief       BeeSDK白板回调声明文件.
 *  @author      HeZhen
 *  @copyright   sohu-inc
 *  @version     2.0.0.1
 */

#ifndef __BEE_WHITE_BOARD_SINK_H__
#define __BEE_WHITE_BOARD_SINK_H__

#import <Foundation/Foundation.h>
#include "bee/base/bee_define.h"

#pragma mark - BeeWhiteBoardSink

/// BeeSDK白板回调协议.
@protocol BeeWhiteBoardSink <NSObject>

/**
 *  @brief  加入白板结果回调.
 *  @param  error       错误码.
 *  @param  msg         错误描述.
 */
- (void)onJoin:(BeeErrorCode)error
           msg:(NSString*)msg;

/**
 *  @brief  离开白板结果回调.
 *  @param  error       错误码.
 *  @param  msg         错误描述.
 */
- (void)onLeave:(BeeErrorCode)error
            msg:(NSString*)msg;

@end

#endif // #ifndef __BEE_WHITE_BOARD_SINK_H__
