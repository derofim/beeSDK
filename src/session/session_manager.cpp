#include "session_manager.h"

namespace bee {

//////////////////////////////////SessionManager////////////////////////////////////////
SessionManager::SessionManager():
    initialized_(false),
    count_(0) {

}

SessionManager::~SessionManager() {

}

bool SessionManager::init(size_t count) {
    bool ret = true;
    do {
        if (initialized_) {
            break;
        }

        count_ = (count==0)?kDefaultSessionCount:count;
        
        all_sessions_.resize(count_);
        for (size_t i = 0;i < count_;++i) {
            BeeSession::Ptr session(new BeeSession(i));
            session->init_state_machine();
            all_sessions_[i] = session;
            idle_sessions_[i] = session;
        }

        initialized_ = true;
    } while (0);
    return ret;
}

bool SessionManager::uninit() {
    bool ret = true;
    do {
        if (!initialized_) {
            break;
        }

        SessionTable::iterator iter = busy_sessions_.begin();
        for (;iter != busy_sessions_.end();++iter) {
            BeeSession::Ptr session = iter->second;
            if (session != NULL) {
                session->close();
            }
        }

        all_sessions_.clear();
        idle_sessions_.clear();
        busy_sessions_.clear();

        initialized_ = false;
    } while (0);
    return ret;
}

int32_t SessionManager::open_idle_session(IOSPtr ios, BeeErrorCode &ec, const SystemParam &param, const std::string &lua_name, const std::string &lua, std::string &live_session_id, std::vector<BeeCapability> &capability) {
    int32_t session_id = -1;
    do {
        SessionTable::iterator iter = idle_sessions_.begin();
        if (iter == idle_sessions_.end()) {
            ec = kBeeErrorCode_Insufficient_Session;
            break;
        }

        BeeSession::Ptr session = iter->second;
        if (session == NULL) {
            ec = kBeeErrorCode_Null_Pointer;
            break;
        }

        ec = session->open(ios, param, lua_name, lua, capability);
        if (ec != kBeeErrorCode_Success) {
            break;
        }

        session_id = session->get_session_id();
        live_session_id = session->get_live_session_id();
        busy_sessions_[session_id] = session;
        idle_sessions_.erase(iter);
    } while (0);
    return session_id;
}

bool SessionManager::close_busy_session(int32_t session_id) {
    bool ret = true;
    do {
        SessionTable::iterator iter = busy_sessions_.find(session_id);
        if (iter == busy_sessions_.end()) {
            ret = false;
            break;
        }

        BeeSession::Ptr session = iter->second;
        if (session == NULL) {
            ret = false;
            break;
        }

        if (session->close() != kBeeErrorCode_Success) {
            ret = false;
            break;
        }

        idle_sessions_[session_id] = iter->second;
        busy_sessions_.erase(iter);        
    } while (0);
    return ret;
}

BeeSession::Ptr SessionManager::get_busy_session(int32_t session_id) {
    BeeSession::Ptr session;
    do {
        SessionTable::iterator iter = busy_sessions_.find(session_id);
        if (iter == busy_sessions_.end()) {
            break;
        }

        session = iter->second;
    } while (0);
    return session;
}

} // namespace bee
