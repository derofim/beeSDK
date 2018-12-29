#include "statusd_client.h"

namespace bee {

///////////////////////////////////AsyncStateMachine Definition///////////////////////////////////////
BEGIN_STATE_MACHINE(StatusdClient)
    BEGIN_STATE(eStatusdState_Resolve, TcpResolveState, &StatusdClient::handle_Resolve, true)
        SWITCH_TO(eStatusdState_Connect, &StatusdClient::Transform_Resolve_To_Connect)
    END_STATE

    BEGIN_STATE(eStatusdState_Connect, TcpConnectState, &StatusdClient::handle_Connect, false)
        SWITCH_TO(eStatusdState_Login, &StatusdClient::Transform_Connect_To_Login)
    END_STATE

    BEGIN_STATE(eStatusdState_Login, TcpSendState, &StatusdClient::handle_Login, false)
    END_STATE
END_STATE_MACHINE

StatusdClient::StatusdClient(IOSPtr ios) : TcpStateMachine(ios) {

}

StatusdClient::~StatusdClient() {

}

BeeErrorCode StatusdClient::connect(const std::string &uid, StatusdHandler::SPtr handler) {
    BeeErrorCode ret = kBeeErrorCode_Success;
    do {
        if (ios_ == NULL) {
            ret = kBeeErrorCode_Service_Not_Started;
            break;
        }

        handler_ = handler;
        ios_->post(boost::bind(&StatusdClient::inner_connect, shared_from_base<StatusdClient>(), uid));
    } while (0);
    return ret;
}

BeeErrorCode StatusdClient::set_sid(const std::string &sid, bool sync) {
    BeeErrorCode ret = kBeeErrorCode_Success;
    do {
        if (ios_ == NULL) {
            ret = kBeeErrorCode_Service_Not_Started;
            break;
        }

        ios_->post(boost::bind(&StatusdClient::inner_set_sid, shared_from_base<StatusdClient>(), sid, sync));
    } while (0);
    return ret;
}

BeeErrorCode StatusdClient::configure_log(int32_t level, TimeType time_base) {
    BeeErrorCode ret = kBeeErrorCode_Success;
    do {
        if (ios_ == NULL) {
            ret = kBeeErrorCode_Service_Not_Started;
            break;
        }

        ios_->post(boost::bind(&StatusdClient::inner_configure_log, shared_from_base<StatusdClient>(), level, time_base));
    } while (0);
    return ret;
}

BeeErrorCode StatusdClient::write_log(const std::string &log) {
    BeeErrorCode ret = kBeeErrorCode_Success;
    do {
        if (ios_ == NULL) {
            ret = kBeeErrorCode_Service_Not_Started;
            break;
        }

        if (!logined_) {
            ret = kBeeErrorCode_Invalid_State;
            break;
        }

        ios_->post(boost::bind(&StatusdClient::inner_write_log, shared_from_base<StatusdClient>(), log));
    } while (0);
    return ret;
}

int32_t StatusdClient::handle_Resolve(StateEvent::Ptr ev) {
    int32_t next_state = kInvalidState;
    do {
        if (ev->ec1 != kBeeErrorCode_Success || ev->ec2) {
            break;
        }

        next_state = eStatusdState_Connect;
    } while (0);
    return next_state;
}

int32_t StatusdClient::handle_Connect(StateEvent::Ptr ev) {
    int32_t next_state = kInvalidState;
    do {
        TcpConnectOutputEvent *output_ev = (TcpConnectOutputEvent*)ev.get();
        if (output_ev->ec1 != kBeeErrorCode_Success || output_ev->ec2) {
            break;
        }

        next_state = eStatusdState_Login;
    } while (0);
    return next_state;
}

int32_t StatusdClient::handle_Login(StateEvent::Ptr ev) {
    int32_t next_state = kInvalidState;
    do {
        TcpConnectOutputEvent *output_ev = (TcpConnectOutputEvent*)ev.get();
        if (output_ev->ec1 != kBeeErrorCode_Success || output_ev->ec2) {
            break;
        }

        next_state = kFinishedState;
        logined_ = true;

        if (sync_sid_pending_) {
            sync_sid();
            sync_sid_pending_ = false;
        }
        
        start_heartbeat_timer();
    } while (0);
    return next_state;
}

bool StatusdClient::Transform_Resolve_To_Connect(StateEvent::Ptr input, StateEvent::Ptr output) {
    bool ret = true;
    do {
        TcpResolveOutputEvent *resolve_ev = (TcpResolveOutputEvent*)input.get();        
        if (resolve_ev->resolved_endpoints.empty()) {
            ret = false;
            break;
        }

        TcpConnectInputEvent *connect_ev = (TcpConnectInputEvent*)output.get();
        connect_ev->endpoints = resolve_ev->resolved_endpoints;
        connect_ev->connect_timeout = kStatusdConnectTimeout;
        connect_ev->setup = true;
    } while (0);
    return ret;
}

bool StatusdClient::Transform_Connect_To_Login(StateEvent::Ptr input, StateEvent::Ptr output) {
    bool ret = true;
    do {
        if (uid_.empty()) {
            ret = false;
            break;
        }

        output->data = IOBuffer(kPkgAppLoginLen);

        PkgAppLogin *pkg = (PkgAppLogin *)output->data.data();
        memset(pkg, 0, kPkgAppLoginLen);
        SETUP_PROTOCOL_HEADER(pkg, kPkgAppLoginLen, STATUSD_LOGIN);        
        memcpy(pkg->uid, uid_.c_str(), uid_.size());
        memcpy(pkg->sid, sid_.c_str(), sid_.size());

        output->setup = true;
    } while (0);
    return ret;
}
    
void StatusdClient::stop() {
    if (heartbeat_timer_ != NULL) {
        heartbeat_timer_->cancel();
        heartbeat_timer_.reset();
    }
    
    TcpStateMachine::stop();
}

void StatusdClient::inner_connect(const std::string &uid) {
    uid_ = uid;
    start(kStatusHost, kStatusPort);
}

void StatusdClient::inner_set_sid(const std::string &sid, bool sync) {
    do {
        if (sid == sid_) {
            break;
        }

        sid_ = sid;

        if (!logined_ && sync) {
            sync_sid_pending_ = true;
            break;
        }

        if (!sync) {
            break;
        }

        sync_sid();
    } while (0);
}

void StatusdClient::inner_configure_log(int32_t level, TimeType time_base) {
    if (level >= eStatusdLogLevel_Debug && level < eStatusdLogLevel_Count) {
        log_level_ = static_cast<StatusdLogLevel>(level);
    }

    time_base_ = time_base;
}

void StatusdClient::inner_write_log(const std::string &log) {
    auto current_time_ = MillisecTimer::get_tickcount();
    auto passed_time_ = current_time_ - start_time_;
    auto relative_time = time_base_ + passed_time_;

    size_t pure_len = strlen(log.c_str());
    uint32_t len = kPkgAppLogLen + pure_len + 1; 
    IOBuffer buffer(len);
    PkgAppLog *pkg = (PkgAppLog *)buffer.data();
    memset(pkg, 0, len);
    SETUP_PROTOCOL_HEADER(pkg, len, STATUSD_REPORT_LOG);
    pkg->log_type = STATUSD_LOG_TYPE;
    pkg->log_time = relative_time;
    memcpy(pkg->log_msg, log.c_str(), pure_len);
    pkg->log_msg[pure_len] = '\0'; // '\0' terminate

    write(buffer);
}

void StatusdClient::output(StateEvent::Ptr ev) {
    if ((ev->ec1 != kBeeErrorCode_Success &&  
        ev->ec1 != kBeeErrorCode_State_Machine_Finished) || 
        ev->ec2) {
        handle_error(ev->ec1, ev->ec2);
    }
}

void StatusdClient::write(const IOBuffer &data) {
    async_write(
        data,
        boost::bind(&StatusdClient::handle_write,
            shared_from_base<StatusdClient>(),
            boost::asio::placeholders::error,
            boost::asio::placeholders::bytes_transferred));
}

void StatusdClient::handle_write(const boost::system::error_code& ec2, size_t bytes_transferred) {
    do {
        if (ec2) {
            handle_error(kBeeErrorCode_Write_Fail, ec2);
            break;
        }

        if (check_send_buffer(bytes_transferred)) {
            async_write(
                dummy_buffer_,
                boost::bind(&StatusdClient::handle_write,
                    shared_from_base<StatusdClient>(),
                    boost::asio::placeholders::error,
                    boost::asio::placeholders::bytes_transferred));
        }
    } while (0);
}

void StatusdClient::handle_error(BeeErrorCode ec1, const boost::system::error_code& ec2) {
    logined_ = false;
    StatusdHandler::SPtr handler = handler_.lock();
    if (handler != NULL) {
        handler->handle_statusd_error(ec1, ec2);
    }
}

void StatusdClient::sync_sid() {
    IOBuffer buffer(kPkgResetSIDLen);
    PkgResetSID *pkg = (PkgResetSID *)buffer.data();
    memset(pkg, 0, kPkgResetSIDLen);
    SETUP_PROTOCOL_HEADER(pkg, kPkgResetSIDLen, STATUSD_RESET_SID);
    memcpy(pkg->sid, sid_.c_str(), sid_.size());

    write(buffer);
}
    
void StatusdClient::start_heartbeat_timer() {
    if (heartbeat_timer_ != NULL) {
        heartbeat_timer_->cancel();
        heartbeat_timer_.reset();
    }    
    
    heartbeat_timer_ = AsyncWaitTimer::create(*(ios()));
    heartbeat_timer_->set_wait_millseconds(kStatusHeartbeatInterval);
    heartbeat_timer_->set_wait_times(-1);
    heartbeat_timer_->set_immediately(false);
    heartbeat_timer_->async_wait(boost::bind(&StatusdClient::handle_heartbeart_timeout, shared_from_base<StatusdClient>()));
}
    
void StatusdClient::handle_heartbeart_timeout() {
    inner_write_log("heartbeat");
}

} //namespace bee
