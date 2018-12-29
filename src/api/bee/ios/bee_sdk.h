/**
 *  @file        bee_sdk.h
 *  @brief       BeeSDK通用接口声明文件.
 *  @author      HeZhen
 *  @copyright   sohu-inc
 *  @version     2.0.0.1
 */

#ifndef __BEE_SDK_H__
#define __BEE_SDK_H__

#import "bee_sdk_sink.h"

#pragma mark - BeeSDKParam

/// BeeSDK初始化参数类.
@interface BeeSDKParam : NSObject

/// 平台类型.
@property(nonatomic, assign) BeePlatformType platformType;
/// 集成SDK的App名.
@property(nonatomic, copy)   NSString *appName;
/// 集成SDK的App版本.
@property(nonatomic, copy)   NSString *appVersion;
/// 操作系统版本等信息.
@property(nonatomic, copy)   NSString *systemInfo;
/// 机器码，作为设备唯一标识.
@property(nonatomic, copy)   NSString *machineCode;
/// 日志路径，当logVolumeCount=0时，日志名以日期为后缀，当前logVolumeCount>0时，日志名以卷号为后缀.
@property(nonatomic, copy)   NSString *logPath;
/// 日志级别.
@property(nonatomic, assign) BeeLogLevel logLevel;
/// 单个日志最大行数(logVolumeCount=0).
@property(nonatomic, assign) bee_int32_t logMaxLine;
/// 日志卷数.
@property(nonatomic, assign) bee_int32_t logVolumeCount;
/// 每卷日志的大小(logVolumeCount>0)，单位为KB.
@property(nonatomic, assign) bee_int32_t logVolumeSize;
/// 最大会话数.
@property(nonatomic, assign) bee_uint32_t sessionCount;
/// 是否使能实时监控.
@property(nonatomic, assign) bool enableStatusd;

/**
 *  @brief  初始化参数类构造函数.
 *  @param  platformType    平台类型.
 *  @param  appName         集成SDK的App名.
 *  @param  appVersion      集成SDK的App版本.
 *  @param  systemInfo      操作系统版本等信息.
 *  @param  machineCode     机器码，作为设备唯一标识.
 *  @param  logPath         日志路径.
 *  @param  logLevel        日志级别.
 *  @param  logMaxLine      单个日志最大行数.
 *  @param  logVolumeCount  日志卷数.
 *  @param  logVolumeSize   每卷日志的大小，单位KB.
 *  @param  sessionCount    最大会话数.
 *  @param  enableStatusd   是否使能实时监控.
 *  @return BeeSDKParam对象.
 */
- (instancetype)initWithParam:(BeePlatformType)platformType
                      appName:(NSString*)appName
                   appVersion:(NSString*)appVersion
                   systemInfo:(NSString*)systemInfo
                  machineCode:(NSString*)machineCode
                      logPath:(NSString*)logPath
                     logLevel:(BeeLogLevel)logLevel
                   logMaxLine:(bee_int32_t)logMaxLine
               logVolumeCount:(bee_int32_t)logVolumeCount
                logVolumeSize:(bee_int32_t)logVolumeSize
                 sessionCount:(bee_uint32_t)sessionCount
                enableStatusd:(bool)enableStatusd;
@end

#pragma mark - BeeSDKCapability

/// BeeSDK业务能力类.
@interface BeeSDKCapability : NSObject

/// 业务码.
@property(nonatomic, assign) bee_int32_t svcCode;

/// 业务描述.
@property(nonatomic, copy) NSString *description;

@end

/**
 *  @brief  BeeSDK.initialize回调Block类型.
 *  @param  ec  错误码.
 */
typedef void (^InitHandler)(BeeErrorCode ec);

/**
 *  @brief  BeeSDK.uninitialize回调Block类型.
 *  @param  ec  错误码.
 */
typedef void (^UnInitHandler)(BeeErrorCode ec);

/**
 *  @brief  BeeSDK.openSession回调Block类型.
 *  @param  ec              错误码.
 *  @param  handle          会话句柄.
 *  @param  capabilities    会话的业务能力数组.
 */
typedef void (^OpenSessionHandler)(BeeErrorCode ec, bee_handle handle, NSArray<BeeSDKCapability*>* capabilities);

/**
 *  @brief  BeeSDK.closeSession回调Block类型.
 *  @param  ec  错误码.
 */
typedef void (^CloseSessionHandler)(BeeErrorCode ec);

#pragma mark - BeeSDK

/// BeeSDK通用接口类.
@interface BeeSDK : NSObject

/**
 *  @brief  单例方法.
 *  @return BeeSDK全局唯一单例对象.
 */
+ (instancetype)sharedInstance;

/**
 *  @brief  初始化BeeSDK.
 *  @param  param       初始化参数.
 *  @param  timeout     初始化方法的超时时间，单位ms.
 *  @param  beeSDKSink  接收通知的对象.
 *  @param  handler     初始化结果异步回调Block.
 *  @note   必须在调用其他API前调用本方法，本方法为异步，必须传入handler才能获得异步调用结果.
 */
- (void)initialize:(BeeSDKParam*)param timeout:(int)timeout sink:(id<BeeSDKSink>)beeSDKSink handler:(InitHandler)handler;

/**
 *  @brief  反初始化BeeSDK，释放资源.
 *  @param  handler     反初始化结果异步回调Block.
 *  @note   本方法为异步，必须传入handler才能获得异步调用结果.
 */
- (void)uninitialize:(UnInitHandler)handler;

/**
 *  @brief  打开一个会话，每个业务都必须绑定到一个会话中.
 *  @param  handler     打开会话结果异步回调Block.
 *  @note   本方法为异步，必须传入handler才能获得异步调用结果.
 */
- (void)openSession:(OpenSessionHandler)handler;

/**
 *  @brief  关闭一个会话，关闭会话后，会话上绑定的所有业务接口不再可用.
 *  @param  handle      会话句柄.
 *  @param  handler     关闭会话结果异步回调Block.
 *  @note   本方法为异步，必须传入handler才能获得异步调用结果.
 */
- (void)closeSession:(bee_handle)handle handler:(CloseSessionHandler)handler;

/**
 *  @brief  错误码转换为可读字符串.
 *  @param  error       错误码.
 *  @return 错误码描述字符串.
 */
- (NSString*)errorToString:(BeeErrorCode)error;

/**
 *  @brief  获得BeeSDK的调用队列线程，所有来自APP的调用都会在该线程排队，避免阻塞主线程.
 *  @return BeeSDK的调用队列线程.
 */
- (dispatch_queue_t)getDispatchQueue;

@end

#endif // #ifndef __BEE_SDK_H__
