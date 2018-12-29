#ifndef __STATUSD_CLIENT_H__
#define __STATUSD_CLIENT_H__

#include "statusd_def.h"
#include "network/tcp_state_machine.h"

typedef enum StatusdLogLevel{
    eStatusdLogLevel_Debug = 0,
    eStatusdLogLevel_Trace,
    eStatusdLogLevel_Error,
    eStatusdLogLevel_Close,
    eStatusdLogLevel_Count
}StatusdLogLevel;

namespace bee {

//////////////////////////////////StatusdHandler////////////////////////////////////////
class StatusdHandler {
public:
    typedef std::shared_ptr<StatusdHandler> SPtr;
    typedef std::weak_ptr<StatusdHandler> WPtr;
    StatusdHandler() {}
    virtual ~StatusdHandler() {}

public:
    virtual void handle_statusd_error(BeeErrorCode ec1, const boost::system::error_code& ec2) = 0;
};

///////////////////////////////////StatusdClient///////////////////////////////////////
class StatusdClient : public TcpStateMachine {
public:
    typedef std::shared_ptr<StatusdClient> Ptr;
    StatusdClient(IOSPtr ios);
	virtual ~StatusdClient();

public:
    DECLARE_STATE_MACHINE
    BeeErrorCode connect(const std::string &uid, StatusdHandler::SPtr handler);
    BeeErrorCode set_sid(const std::string &sid, bool sync);
    BeeErrorCode configure_log(int32_t level, TimeType time_base);
    BeeErrorCode write_log(const std::string &log);
    bool is_logined() { return logined_; }

    //Routers
    int32_t handle_Resolve(StateEvent::Ptr ev);
    int32_t handle_Connect(StateEvent::Ptr ev);
    int32_t handle_Login(StateEvent::Ptr ev);

    //Transforms
    bool Transform_Resolve_To_Connect(StateEvent::Ptr input, StateEvent::Ptr output);
    bool Transform_Connect_To_Login(StateEvent::Ptr input, StateEvent::Ptr output);
    
    virtual void stop();

protected:
    void inner_connect(const std::string &uid);
    void inner_set_sid(const std::string &sid, bool sync);
    void inner_configure_log(int32_t level, TimeType time_base);
    void inner_write_log(const std::string &log);
    void output(StateEvent::Ptr ev); public:
    void write(const IOBuffer &data);
    void handle_write(const boost::system::error_code& ec2, size_t bytes_transferred);
    void handle_error(BeeErrorCode ec1, const boost::system::error_code& ec2);
    void sync_sid();
    void start_heartbeat_timer();
    void handle_heartbeart_timeout();

protected:
    IOBuffer dummy_buffer_;

private:
    StatusdHandler::WPtr handler_;
    bool logined_ = false;
    std::string uid_;
    std::string sid_;
    TimeType time_base_ = 0;
    TimeType start_time_ = MillisecTimer::get_tickcount();
    bool sync_sid_pending_ = false;
    StatusdLogLevel log_level_ = eStatusdLogLevel_Debug;
    AsyncWaitTimer::Ptr heartbeat_timer_;
};

} // namespace bee

#endif
