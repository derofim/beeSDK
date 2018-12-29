#include "lua_webrtc_stats_log_observer.h"

namespace bee {

LuaWebRTCStatsLogObserver::LuaWebRTCStatsLogObserver():logger_("LuaWebRTCStatsLogObserver") {

}

LuaWebRTCStatsLogObserver::~LuaWebRTCStatsLogObserver() {

}

void LuaWebRTCStatsLogObserver::OnComplete(const webrtc::StatsReports& reports) {
    logger_.Debug("\n[STATS]  ====================BEGIN====================\n");
    for (auto report_iter = reports.begin(); report_iter != reports.end(); ++report_iter) {
        const webrtc::StatsReport *report = (*report_iter);
        const char *type_name = report->TypeToString();
        if (type_name == NULL) {
            continue;
        }
        logger_.Debug("\n[STATS]  + %s  %s\n", type_name, report->id()->ToString().c_str());
        const webrtc::StatsReport::Values &values = report->values();
        for (auto value_iter = values.begin(); value_iter != values.end(); ++value_iter) {
            webrtc::StatsReport::Value *value = value_iter->second.get();
            std::ostringstream os;
            os << "\n[STATS]    - ";
            os << value->display_name();
            os << ":";
            os << value->ToString();
            logger_.Debug("%s\n", os.str().c_str());
        }
    }
    logger_.Debug("\n[STATS]  ====================END====================\n");
}

} // namespace bee
