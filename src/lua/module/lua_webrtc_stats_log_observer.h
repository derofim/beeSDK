#ifndef __LUA_WEBRTC_STATS_LOG_OBSERVER_H__
#define __LUA_WEBRTC_STATS_LOG_OBSERVER_H__

#include "log/logger.h"
#include "webrtc/api/peerconnectioninterface.h"

namespace bee {

class LuaWebRTCStatsLogObserver : public webrtc::StatsObserver {
public:
    LuaWebRTCStatsLogObserver();
    ~LuaWebRTCStatsLogObserver();

public:
    virtual void OnComplete(const webrtc::StatsReports& reports);
    
private:
    Logger logger_;
};

} // namespace bee

#endif // #ifndef __LUA_WEBRTC_STATS_LOG_OBSERVER_H__
