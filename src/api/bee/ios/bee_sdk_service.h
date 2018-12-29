/**
 *  @file        bee_sdk_service.h
 *  @brief       BeeSDK通用业务类声明文件.
 *  @author      HeZhen
 *  @copyright   sohu-inc
 *  @version     2.0.0.1
 */

#ifndef __BEE_SDK_SERVICE_H__
#define __BEE_SDK_SERVICE_H__

#import <Foundation/Foundation.h>
#include "bee/base/bee_define.h"
#include <string>

#pragma mark - BeeSDKServiceDataProtocol

/// BeeSDK通用业务数据通知协议.
@protocol BeeSDKServiceDataProtocol <NSObject>

/**
 *  @brief  通知业务数据.
 *  @param  data    业务数据.
 */
- (void)handleData:(NSString*)data;

@end

#pragma mark - BeeSDKService

/// BeeSDK通用业务类，作为所有业务类的基类.
@interface BeeSDKService : NSObject <BeeSDKServiceDataProtocol>

/**
 *  @brief  通用业务类构造函数.
 *  @param  svcCode     业务码.
 *  @note   必须在派生类调用.
 *  @return 通用业务类对象.
 */
- (instancetype)initWithSvcCode:(bee_int32_t)svcCode;

/**
 *  @brief  注册业务到一个会话.
 *  @param  handle      会话句柄.
 *  @note   每个业务都必须注册到一个会话中才能获取业务数据.
 *  @return 错误码.
 */
- (BeeErrorCode)Register:(bee_handle)handle;

/**
 *  @brief  解除业务到会话的注册.
 *  @return 错误码.
 */
- (BeeErrorCode)unRegister;

/**
 *  @brief  返回是否已经注册.
 *  @return 是否已经注册.
 */
- (BOOL)isRegistered;

/**
 *  @brief  执行一个命令.
 *  @param  command     命令名称.
 *  @param  args        命令参数.
 *  @param  timeout     执行命令的超时时间，单位ms.
 *  @note   目前该方法是异步方法，通过BeeSDKServiceDataProtocol返回数据. 
 *  @return 错误码.
 *  @see    BeeSDKServiceDataProtocol.
 */
- (BeeErrorCode)execute:(NSString*)command args:(NSString*)args timeout:(bee_int32_t)timeout;

@end

#endif // #ifndef __BEE_SDK_SERVICE_H__
