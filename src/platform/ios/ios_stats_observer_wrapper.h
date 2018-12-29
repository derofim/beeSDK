#ifndef __IOS_STATS_OBSERVER_WRAPPER_H__
#define __IOS_STATS_OBSERVER_WRAPPER_H__

#import "WebRTC/RTCLegacyStatsReport.h"

#include "webrtc/api/peerconnectioninterface.h"

namespace bee {
    
class BeeStatsObserverAdapter : public webrtc::StatsObserver {
public:
    BeeStatsObserverAdapter(void (^completionHandler)(NSArray<RTCLegacyStatsReport *> *stats));
    ~BeeStatsObserverAdapter();
    void OnComplete(const webrtc::StatsReports& reports) override;
    
private:
    void (^completion_handler_)(NSArray<RTCLegacyStatsReport *> *stats);
};

}  // namespace bee

#endif // #ifndef __IOS_STATS_OBSERVER_WRAPPER_H__
