#include "platform/win32/log_stats.h"

namespace bee {

WinStatsObserver::WinStatsObserver():logger_("WinStatsObserver") {

}

WinStatsObserver::~WinStatsObserver() {

}

void WinStatsObserver::OnComplete(const webrtc::StatsReports& reports) {
    logger_.Debug("<Stats> stats begin.\n");
    for (auto report_iter = reports.begin(); report_iter != reports.end(); ++report_iter) {
        const webrtc::StatsReport *report = (*report_iter);
        const char *type_name = report->TypeToString();
        if (type_name == NULL) {
            continue;
        }
        logger_.Debug("<Stats> type:%s.\n", type_name);
        const webrtc::StatsReport::Values &values = report->values();
        for (auto value_iter = values.begin(); value_iter != values.end(); ++value_iter) {
            webrtc::StatsReport::Value *value = value_iter->second.get();
            logger_.Debug("<Stats>     --------------------\n");
            logger_.Debug("<Stats>     name  : %s.\n", value->display_name());
            switch (value->type()) {
            case webrtc::StatsReport::Value::kInt:
                logger_.Debug("<Stats>     value : %d.\n", value->int_val());
                break;
            case webrtc::StatsReport::Value::kInt64:
                logger_.Debug("<Stats>     value : %I64d.\n", value->int64_val());
                break;
            case webrtc::StatsReport::Value::kFloat:
                logger_.Debug("<Stats>     value : %f.\n", value->float_val());
                break;
            case webrtc::StatsReport::Value::kString:
                logger_.Debug("<Stats>     value : %s.\n", value->string_val().c_str());
                break;
            case webrtc::StatsReport::Value::kStaticString:
                logger_.Debug("<Stats>     value : %s.\n", value->static_string_val());
                break;
            case webrtc::StatsReport::Value::kBool:
                logger_.Debug("<Stats>     value : %s.\n", value->bool_val()?"true":"false");
                break;
            case webrtc::StatsReport::Value::kId:
                logger_.Debug("<Stats>     value : %s.\n", value->ToString().c_str());
                break;
            }
        }
    }
    logger_.Debug("<Stats> stats end.\n");
}

} // namespace bee
