#ifndef __BEE_SESSION_H__
#define __BEE_SESSION_H__

#include "utility/common.h"
#include "bee/base/bee.h"
#include "state/simple_state_machine.h"
#include <random>
#include <set>

extern "C" {
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
}

namespace bee {

//////////////////////////////////State Machine////////////////////////////////////////
typedef enum BeeSessionInputEvent {
    kBeeSessionInput_Open = 0,
    kBeeSessionInput_Opened,
    kBeeSessionInput_Execute,
    kBeeSessionInput_Close,
    kBeeSessionInput_Closed,
    kBeeSessionInput_Count
}BeeSessionInputEvent;

typedef enum BeeSessionState {
    kBeeSessionState_Idle = 0,
    kBeeSessionState_Opening,
    kBeeSessionState_Ready,
    kBeeSessionState_Closing,
    kBeeSessionState_Count,
    kBeeSessionState_Invalid
}BeeSessionState;

const int32_t kLiveSessionIdLen = 32;

///////////////////////////////////BeeSession///////////////////////////////////////
class LuaEngine;
class BeeService;

class BeeSession : public std::enable_shared_from_this<BeeSession>, public SimpleStateMachine {
public:
    typedef std::shared_ptr<BeeSession> Ptr;
    BeeSession(int32_t session_id);
    ~BeeSession();

public:
    BeeErrorCode open(IOSPtr ios, const SystemParam &param, const std::string &lua_name, const std::string &lua, std::vector<BeeCapability> &capability);
    BeeErrorCode close();
    BeeErrorCode execute(bee_int32_t svc_code, const std::string &cmd, const std::string &args);
    void         on_svc_notify(int32_t svc_code, const char *data, size_t size);
    int32_t      get_session_id(){return session_id_;}
    IOSPtr       get_ios(){return ios_;}
    void         init_state_machine();
    std::string& get_live_session_id() { return live_session_id_; }
    BeeErrorCode reg_svc(std::shared_ptr<BeeService> svc);
    BeeErrorCode unreg_svc(std::shared_ptr<BeeService> svc);

protected:
    void         attach_ios(IOSPtr ios){ios_ = ios;}
    void         detach_ios() { ios_.reset(); }
    void         generate_live_session_id();
    void         reset_live_session_id();

protected:
    IOSPtr ios_;
    int32_t session_id_ = -1;
    std::shared_ptr<LuaEngine> lua_engine_;
    std::string live_session_id_;
    std::random_device rd_;
    std::unordered_map<bee_int32_t, std::shared_ptr<BeeService> > services_;
    std::set<bee_int32_t> capabilities_;
};

} // namespace bee

#endif
