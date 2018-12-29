#include "bee_service.h"
#include "service/bee_entrance.h"

namespace bee {

BeeService::BeeService(bee_int32_t svc_code) : svc_code_(svc_code) {

}

BeeService::~BeeService() {

}

BeeErrorCode BeeService::reg(bee_handle handle) {
    BeeErrorCode ret = kBeeErrorCode_Success;
    do {
        if (handle < 0) {
            ret = kBeeErrorCode_Invalid_Param;
            break;
        }

        handle_ = handle;
        ret = BeeEntrance::instance()->reg_svc(shared_from_this());
    } while (0);
    return ret;
}

BeeErrorCode BeeService::unreg() {
    return BeeEntrance::instance()->unreg_svc(shared_from_this());
}

BeeErrorCode BeeService::execute(const std::string &command, const std::string &args, bee_int32_t timeout) {
    return BeeEntrance::instance()->sync_execute(handle_, svc_code_, command, args, timeout);
}

} //namespace bee
