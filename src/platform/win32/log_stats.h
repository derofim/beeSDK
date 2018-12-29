#ifndef __LOG_STATS_H__
#define __LOG_STATS_H__

#include "log/logger.h"
#include "webrtc/api/peerconnectioninterface.h"

namespace bee {

class WinStatsObserver : public webrtc::StatsObserver {
public:
    WinStatsObserver();
    ~WinStatsObserver();

public:
    virtual void OnComplete(const webrtc::StatsReports& reports);
    
private:
    Logger logger_;
};

} // namespace bee

#endif // #ifndef __LOG_STATS_H__
