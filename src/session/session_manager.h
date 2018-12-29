#ifndef __SESSION_MANAGER_H__
#define __SESSION_MANAGER_H__

#include "bee_session.h"
#include "network/io_service.h"

namespace bee {

typedef std::unordered_map<int32_t, BeeSession::Ptr> SessionTable;

/////////////////////////////////////Constants/////////////////////////////////////
const int32_t kDefaultSessionCount = 16;

////////////////////////////////////SessionManager//////////////////////////////////////
class SessionManager 
{
public:
    typedef std::shared_ptr<SessionManager> Ptr;
    SessionManager();
    ~SessionManager();

public:
    bool init(size_t count);
    bool uninit();
    int32_t open_idle_session(IOSPtr ios, BeeErrorCode &ec, const SystemParam &param, const std::string &lua_name, const std::string &lua, std::string &live_session_id, std::vector<BeeCapability> &capability);
    bool    close_busy_session(int32_t session_id);
    BeeSession::Ptr get_busy_session(int32_t session_id);

private:
    std::vector<BeeSession::Ptr> all_sessions_;    
    SessionTable idle_sessions_;
    SessionTable busy_sessions_;
    bool initialized_;
    size_t count_;
};

} // namespace bee

#endif
