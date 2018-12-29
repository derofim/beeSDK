#include "ios_stats_observer_wrapper.h"
#import "platform/ios/RTCLegacyStatsReport+Private.h"

namespace bee {

BeeStatsObserverAdapter::BeeStatsObserverAdapter(void (^completionHandler)(NSArray<RTCLegacyStatsReport *> *stats)) {
    completion_handler_ = completionHandler;
}

BeeStatsObserverAdapter::~BeeStatsObserverAdapter() {
    completion_handler_ = nil;
}

void BeeStatsObserverAdapter::OnComplete(const webrtc::StatsReports& reports) {
    RTC_DCHECK(completion_handler_);
    NSMutableArray *stats = [NSMutableArray arrayWithCapacity:reports.size()];
    for (const auto* report : reports) {
        RTCLegacyStatsReport *statsReport =
        [[RTCLegacyStatsReport alloc] initWithNativeReport:*report];
        [stats addObject:statsReport];
    }
    completion_handler_(stats);
    completion_handler_ = nil;
}
    
}  // namespace bee
