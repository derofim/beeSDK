/**
 *  @file        bee_service.h
 *  @brief       BeeSDK底层通用业务无关接口回调类声明文件.
 *  @author      HeZhen
 *  @copyright   sohu-inc
 *  @version     2.0.0.1
 */

#ifndef __BEE_SINK_H__
#define __BEE_SINK_H__

#include "bee/base/bee_define.h"

namespace bee {

/// BeeSDK通用接口回调类.
class BeeSink {
public:
    /**
     *  @brief  BeeSink类构造函数.
     */
    BeeSink() {}
    /**
     *  @brief  BeeSink类析构函数.
     */
    virtual ~BeeSink() {}

public:
    /**
     *  @brief  日志回调.
     *  @param  log     日志行.
     */
	virtual void on_log(const char *log) = 0;

    /**
     *  @brief  系统通知回调(保留).
     *  @param  ec1     系统错误码.
     *  @param  ec2     平台相关错误码.
     */
	virtual void on_notify(BeeErrorCode ec1, bee_int32_t ec2) = 0;
};

} // namespace bee

#endif // #ifndef __BEE_SINK_H__
