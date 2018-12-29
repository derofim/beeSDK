/**
 *  @file        bee_sdk_sink.h
 *  @brief       BeeSDK获取通用通知的协议文件.
 *  @author      HeZhen
 *  @copyright   sohu-inc
 *  @version     2.0.0.1
 */

#ifndef __BEE_SDK_SINK_H__
#define __BEE_SDK_SINK_H__

#include "bee/base/bee_define.h"
#import <Foundation/Foundation.h>

/// BeeSDK通用通知协议.
@protocol BeeSDKSink <NSObject>

/**
 *  @brief  日志回调.
 *  @param  log     日志行.
 */
- (void)onLog:(NSString*)log;

/**
 *  @brief  系统通知.
 *  @param  ec1     BeeSDK错误码.
 *  @param  ec2     平台相关错误码.
 */
- (void)onNotify:(BeeErrorCode)ec1 ec2:(bee_int32_t) ec2;

@end

#endif // #ifndef __BEE_SDK_SINK_H__
