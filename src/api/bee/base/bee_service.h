/**
 *  @file        bee_service.h
 *  @brief       BeeSDK底层基本业务类声明文件.
 *  @author      HeZhen
 *  @copyright   sohu-inc
 *  @version     2.0.0.1
 */

#ifndef __BEE_SERVICE_H__
#define __BEE_SERVICE_H__

#include "bee/base/bee_define.h"
#include <string>
#include <memory>

namespace bee {

/// BeeSDK基本业务类.
class BeeService : public std::enable_shared_from_this<BeeService> {
public:
    /**
     *  @brief  BeeService类构造函数.
     *  @param  svc_code    业务码.
     */
    BeeService(bee_int32_t svc_code);
    
    /**
     *  @brief  BeeService类析构函数.
     */
    virtual ~BeeService();

public:
    /**
     *  @brief  注册业务到一个会话上，所有业务都必须绑定到一个会话中才能接收到业务数据.
     *  @param  handle      业务码.
     *  @return 错误码.
     *  @note   一个会话上同时只能注册一种同业务码的业务，如果需要多个相同业务，需要打开多个会话.
     */
    BeeErrorCode reg(bee_handle handle);

    /**
     *  @brief  取消业务到一个会话的注册，反注册后不会再收到业务数据.
     *  @return 错误码.
     *  @note   通常反注册是不需要的，会话关闭后注册信息会自动清除.
     */
    BeeErrorCode unreg();
    
    /**
     *  @brief  执行一个业务命令.
     *  @param  command     命令名.
     *  @param  args        命令参数.
     *  @param  timeout     执行命令的超时.
     *  @return 错误码.
     *  @note   该命令为异步，返回数据在handle_data中.
     *  @see    handle_data.
     */
    BeeErrorCode execute(const std::string &command, const std::string &args, bee_int32_t timeout);

    /**
     *  @brief  业务数据回调，派生类必须实现该方法以获得业务数据.
     *  @param  data        数据.
     */
    virtual void handle_data(const std::string &data) = 0;

    /**
     *  @brief  获取会话句柄.
     *  @return 会话句柄.
     */
    bee_handle get_handle() { return handle_; }

    /**
     *  @brief  获取业务码.
     *  @return 业务码.
     */
    bee_int32_t get_svc_code() { return svc_code_; }

protected:
    bee_handle handle_ = -1;
    bee_int32_t svc_code_ = -1;
};

} // namespace bee

#endif // #ifndef __BEE_SERVICE_H__
