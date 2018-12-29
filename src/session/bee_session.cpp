#include "bee_session.h"
#include "session_manager.h"
#include "bee/base/bee_service.h"
#include "service/bee_promise.h"
#include "lua/lua_engine.h"

namespace bee {

////////////////////////////////////BeeSession//////////////////////////////////////
BeeSession::BeeSession(int32_t session_id)
    : SimpleStateMachine(kBeeSessionState_Count, kBeeSessionInput_Count),
      session_id_(session_id) {

}

BeeSession::~BeeSession() {

}

BeeErrorCode BeeSession::open(IOSPtr ios, const SystemParam &param, const std::string &lua_name, const std::string &lua, std::vector<BeeCapability> &capability) {
    BeeErrorCode ret = kBeeErrorCode_Success;
    do {
        if (!switch_state(kBeeSessionInput_Open)) {
            ret = kBeeErrorCode_Invalid_State;
            break;
        }

        generate_live_session_id();
        lua_engine_.reset(new LuaEngine(this));        
        ret = lua_engine_->open(lua_name, lua, param, live_session_id_, capability);
        if (ret != kBeeErrorCode_Success) {
            break;
        }

        if (!switch_state(kBeeSessionInput_Opened)) {
            ret = kBeeErrorCode_Invalid_State;
            break;
        }

        attach_ios(ios);
        
        capabilities_.clear();
        for (BeeCapability cap : capability) {
            capabilities_.insert(cap.svc_code);
        }
    } while (0);

    if (ret != kBeeErrorCode_Success) {
        reset_state();
    }
    return ret;
}

BeeErrorCode BeeSession::close() {
    BeeErrorCode ret = kBeeErrorCode_Success;
    do {
        if (!switch_state(kBeeSessionInput_Close)) {
            ret = kBeeErrorCode_Invalid_State;
            break;
        }

        detach_ios();

        if (lua_engine_ != NULL) {
            lua_engine_->close();
            lua_engine_.reset();
        }

        switch_state(kBeeSessionInput_Closed);
        reset_live_session_id();
        services_.clear();
        capabilities_.clear();
    } while (0);
    return ret;
}

BeeErrorCode BeeSession::execute(bee_int32_t svc_code, const std::string &cmd, const std::string &args) {
    BeeErrorCode ret = kBeeErrorCode_Success;
    do {
        if (lua_engine_ == NULL) {
            ret = kBeeErrorCode_Session_Not_Opened;
            break;
        }

        if (services_.find(svc_code) == services_.end()) {
            ret = kBeeErrorCode_Service_Not_Registered;
            break;
        }

        if (!switch_state(kBeeSessionInput_Execute)) {
            ret = kBeeErrorCode_Invalid_State;
            break;
        }

        ret = lua_engine_->execute(svc_code, cmd, args);
        if (ret != kBeeErrorCode_Success) {
            break;
        }
    } while (0);
    return ret;
}

void BeeSession::on_svc_notify(int32_t svc_code, const char *data, size_t size) {
    if (svc_code < 0 || data == NULL || size == 0) {
        return;
    }

    auto iter = services_.find(svc_code);
    if (iter != services_.end() && iter->second != NULL) {
        iter->second->handle_data(std::string(data, size));
    }
}

void BeeSession::init_state_machine() {
    for (int32_t i = 0;i < kBeeSessionState_Count;++i) {
        for (int32_t j = 0;j< kBeeSessionInput_Count;++j) {
            state_machine_[i][j] = kBeeSessionState_Invalid;
        }
    }

    state_machine_[kBeeSessionState_Idle][kBeeSessionInput_Open]        = kBeeSessionState_Opening;
    state_machine_[kBeeSessionState_Opening][kBeeSessionInput_Opened]   = kBeeSessionState_Ready;
    state_machine_[kBeeSessionState_Opening][kBeeSessionInput_Close]    = kBeeSessionState_Closing;
    state_machine_[kBeeSessionState_Ready][kBeeSessionInput_Execute]    = kBeeSessionState_Ready;
    state_machine_[kBeeSessionState_Ready][kBeeSessionInput_Close]      = kBeeSessionState_Closing;
    state_machine_[kBeeSessionState_Closing][kBeeSessionInput_Closed]   = kBeeSessionState_Idle;
}

void BeeSession::generate_live_session_id() {
    const char char_set[63] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
    live_session_id_.resize(kLiveSessionIdLen);
    char *p = (char*)live_session_id_.data();
    for (size_t i = 0; i < kLiveSessionIdLen; ++i) {
        p[i] = char_set[rd_() % 62];
    }
}

void BeeSession::reset_live_session_id() {
    live_session_id_.clear();
}

BeeErrorCode BeeSession::reg_svc(std::shared_ptr<BeeService> svc) {
    BeeErrorCode ret = kBeeErrorCode_Success;
    do {
        if (svc == NULL) {
            ret = kBeeErrorCode_Null_Pointer;
            break;
        }

        if (svc->get_svc_code() < 0) {
            ret = kBeeErrorCode_Invalid_Param;
            break;
        }
        
        if (capabilities_.find(svc->get_svc_code()) == capabilities_.end()) {
            ret = kBeeErrorCode_Service_Not_Supported;
            break;
        }

        services_.emplace(svc->get_svc_code(), svc);
    } while (0);
    return ret;
}

BeeErrorCode BeeSession::unreg_svc(std::shared_ptr<BeeService> svc) {
    BeeErrorCode ret = kBeeErrorCode_Success;
    do {
        if (svc == NULL) {
            ret = kBeeErrorCode_Invalid_Param;
            break;
        }

        if (svc->get_svc_code() < 0) {
            ret = kBeeErrorCode_Invalid_Param;
            break;
        }

        auto iter = services_.find(svc->get_svc_code());
        if (iter == services_.end()) {
            ret = kBeeErrorCode_Not_Found;
            break;
        }

        services_.erase(iter);
    } while (0);
    return ret;
}

} // namespace bee
