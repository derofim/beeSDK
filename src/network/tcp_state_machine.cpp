#include "tcp_state_machine.h"
#include <queue>

namespace bee {

////////////////////////////////////TcpResolveState//////////////////////////////////////
TcpResolveState::TcpResolveState(int32_t id) : State(id) {
    type_ = kTcpStateType_Resolve;
}

TcpResolveState::~TcpResolveState() {

}

BeeErrorCode TcpResolveState::execute() {
    BeeErrorCode ret = kBeeErrorCode_Success;
    do {
        if (!check_event()) {
            ret = kBeeErrorCode_Invalid_Param;
            break;
        }

        TcpResolveInputEvent *input_ev = (TcpResolveInputEvent*)input_event_.get(); //Must be exactly right.
        TcpResolveOutputEvent *output_ev = (TcpResolveOutputEvent*)output_event_.get(); 

        boost::asio::ip::address addr;
        if (check_host_addr(input_ev->host, addr)) {
            //Check if host is ip address,not domain name,if so,switch to next state.
            tcp::endpoint ep(addr, atoi(input_ev->service.c_str()));
            output_ev->resolved_endpoints.push_back(ep);
            output_ev->setup = true;
            done();
            break;
        }

        query_.reset(new TcpResolverQuery(input_ev->host, input_ev->service));
        std::shared_ptr<AsyncStateMachine> fsm = state_machine_.lock();
        if (fsm == NULL) {
            ret = kBeeErrorCode_Error_State_Machine;
            break;
        }

        resolver_.reset(new tcp::resolver(*(fsm->ios())));
        resolver_->async_resolve(
            *(query_),
            boost::bind(&TcpResolveState::handle_resolve,
            shared_from_base<TcpResolveState>(),
            boost::asio::placeholders::error,
            boost::asio::placeholders::iterator));
    } while (0);
    return ret;
}

void TcpResolveState::handle_resolve(const boost::system::error_code& ec2, tcp::resolver::iterator endpoint_iter) {
    BeeErrorCode ec1 = kBeeErrorCode_Success;
    do {
        TcpResolveOutputEvent *ev = (TcpResolveOutputEvent*)output_event_.get(); //Must be exactly right.
        tcp::resolver::iterator end = tcp::resolver::iterator();
        if (endpoint_iter == end) {
            ec1 = kBeeErrorCode_Resolve_Fail;
        }

        for (;endpoint_iter != end;++endpoint_iter) {
            tcp::endpoint endpoint = *endpoint_iter;
            ev->resolved_endpoints.push_back(endpoint);
        }

        ev->ec1 = ec1;
        ev->ec2 = ec2;
        ev->setup = true;
    } while (0);
    done(ec1, ec2); 
}

bool TcpResolveState::setup_input_event(const std::string &host,const std::string &service) {
    bool ret = true;
    do {
        if (input_event_ == NULL ||
            host.empty() ||
            service.empty()) {
            ret = false;
            break;
        }

        TcpResolveInputEvent *ev = (TcpResolveInputEvent*)input_event_.get(); //Must be exactly right.
        if (ev == NULL) {
            ret = false;
            break;
        }

        ev->host = host;
        ev->service = service;
        ev->setup = true;
    } while (0);
    return ret;
}

bool TcpResolveState::check_host_addr(const std::string &host, boost::asio::ip::address &addr) {
    boost::system::error_code err;
    addr = boost::asio::ip::address::from_string(host, err);
    return (!err);
}

////////////////////////////////////TcpConnectState//////////////////////////////////////
TcpConnectState::TcpConnectState(int32_t id):State(id), index_(0), logger_("TcpConnectState") {
    type_ = kTcpStateType_Connect;
}

TcpConnectState::~TcpConnectState() {
}

BeeErrorCode TcpConnectState::execute() {
    BeeErrorCode ret = kBeeErrorCode_Success;
    do {
        if (!check_event()) {
            ret = kBeeErrorCode_Invalid_Param;
            break;
        }

        TcpConnectInputEvent *ev = (TcpConnectInputEvent*)input_event_.get(); //Must be exactly right.
        if (ev->endpoints.empty()) {
            ret = kBeeErrorCode_Invalid_Param;
            break;
        }

        std::shared_ptr<AsyncStateMachine> fsm = state_machine_.lock();
        if (fsm == NULL) {
            ret = kBeeErrorCode_Error_State_Machine;
            break;
        }

        total_count_ = (int32_t)ev->endpoints.size();
        if (ev->start_index >= 0 && ev->start_index < total_count_) {
            index_ = ev->start_index;
        }
        int32_t target_index = index_ % total_count_;
        logger_.Debug("Connecting %dth %s:%u.\n", target_index, ev->endpoints[target_index].address().to_string().c_str(), ev->endpoints[target_index].port());

        TcpStateMachine *tcp_fsm = (TcpStateMachine*)fsm.get();
        tcp_fsm->async_connect(
            ev->endpoints[target_index],
            boost::bind(&TcpConnectState::handle_connect,
            shared_from_base<TcpConnectState>(),
            boost::asio::placeholders::error,
            tcp::resolver::iterator(),
            index_));

        tried_count_++;
        total_timeout_ = ev->connect_timeout;

        check_timer_ = AsyncWaitTimer::create(*(tcp_fsm->ios()));
        check_timer_->set_wait_millseconds(kTcpCheckConnectPeriod);
        check_timer_->set_wait_times(-1);
        check_timer_->set_immediately(false);
        check_timer_->async_wait(boost::bind(&TcpConnectState::handle_period_timeout, shared_from_base<TcpConnectState>()));
    } while (0);
    return ret;
}

void TcpConnectState::handle_connect(const boost::system::error_code& ec2, tcp::resolver::iterator endpoint_iter, int32_t index) {
    BeeErrorCode ec1 = kBeeErrorCode_Connect_Fail;
    bool connect_next = false;
    bool tls_handshake = false;
    do {
        std::shared_ptr<AsyncStateMachine> fsm = state_machine_.lock();
        TcpStateMachine *tcp_fsm = (TcpStateMachine*)fsm.get();
        if (tcp_fsm == NULL) {
            ec1 = kBeeErrorCode_Error_State_Machine;
            break;
        }

        //Event of last endpoint, just ignore it.
        if (index != index_) {
            connect_next = true;
            break;
        }

        TcpConnectInputEvent *input_ev = (TcpConnectInputEvent*)input_event_.get();
        TcpConnectOutputEvent *output_ev = (TcpConnectOutputEvent*)output_event_.get();
        //If connect current endpoint failed.
        if (ec2) {
            //If there're more candidate endpoints to connect, try connecting next.
            if (tried_count_ < total_count_) {
                index_++;
            } else {
                //If timed-out event reported and total timeout not reached yet, just re-connect to the last address until total timeouted.
                if (ec2 != boost::asio::error::timed_out) {
                    break;
                }
            }
            
            int32_t target_index = index_ % total_count_;
            logger_.Debug("Connecting %dth %s:%u.\n", target_index, input_ev->endpoints[target_index].address().to_string().c_str(), input_ev->endpoints[target_index].port());

            //Reset current endpoint connecting spend time.
            current_endpoint_spent_time_ = 0;

            //Close old socket & open new socket.
            tcp_fsm->reset_socket();

            //Try connecting now.
            tcp_fsm->async_connect(
                input_ev->endpoints[target_index],
                boost::bind(&TcpConnectState::handle_connect,
                shared_from_base<TcpConnectState>(),                
                boost::asio::placeholders::error,
                tcp::resolver::iterator(),
                index_));

            tried_count_++;
            connect_next = true;
            break;
        }

        //If tls connection connected, not completed yet, need to handshake.
        if (tcp_fsm->is_tls()) {
            //Do handshaking.
            tcp_fsm->async_handshake(
                boost::asio::ssl::stream_base::client,
                boost::bind(
                &TcpConnectState::handle_handshake,
                shared_from_base<TcpConnectState>(),
                boost::asio::placeholders::error));
            tls_handshake = true;
            break;
        }

        //If run to here, everything must be success.
        int32_t target_index = index_ % total_count_;
        output_ev->index = target_index;
        output_ev->connected_endpoint = input_ev->endpoints[target_index];
        output_ev->setup = true;
        tcp_fsm->connected_ = true;
        ec1 = kBeeErrorCode_Success;
        logger_.Debug("Connected to %dth %s:%u, non tls.\n", target_index, input_ev->endpoints[target_index].address().to_string().c_str(), input_ev->endpoints[target_index].port());

        if (check_timer_ != NULL) {
            check_timer_->cancel();
            check_timer_.reset();
        }
    } while (0);
    
    if (!connect_next && !tls_handshake) {
        done(ec1, ec2);
    }
}

void TcpConnectState::handle_period_timeout() {
    current_spent_time_ += kTcpCheckConnectPeriod;
    current_endpoint_spent_time_ += kTcpCheckConnectPeriod;
    if (current_spent_time_ >= total_timeout_) {
        check_timer_->cancel();
        check_timer_.reset();
        done(kBeeErrorCode_Timeout, boost::system::error_code());        
    } else if (current_endpoint_spent_time_ >= endpoint_timeout_) {
        TcpConnectInputEvent *ev = (TcpConnectInputEvent*)input_event_.get();
        int32_t target_index = index_ % total_count_;
        logger_.Debug("Connecting %dth %s:%u timeout.\n", target_index, ev->endpoints[target_index].address().to_string().c_str(), ev->endpoints[target_index].port());
        handle_connect(boost::asio::error::timed_out, tcp::resolver::iterator(), index_);
    }
}

void TcpConnectState::handle_handshake(const boost::system::error_code& ec) {
    BeeErrorCode ec1 = kBeeErrorCode_Tls_Shakehand_Fail;
    do {
        std::shared_ptr<AsyncStateMachine> fsm = state_machine_.lock();
        TcpStateMachine *tcp_fsm = (TcpStateMachine*)fsm.get();
        if (tcp_fsm == NULL) {
            ec1 = kBeeErrorCode_Error_State_Machine;
            break;
        }

        TcpConnectInputEvent *input_ev = (TcpConnectInputEvent*)input_event_.get();
        TcpConnectOutputEvent *output_ev = (TcpConnectOutputEvent*)output_event_.get();
        int32_t target_index = index_ % total_count_;
        output_ev->index = target_index;
        if (ec) {
            std::string msg = ec.message();
            break;
        }

        //Handshake success.
        output_ev->connected_endpoint = input_ev->endpoints[target_index];
        output_ev->setup = true;
        tcp_fsm->connected_ = true;
        ec1 = kBeeErrorCode_Success;
        logger_.Debug("Connected to %dth %s:%u, tls.\n", target_index, input_ev->endpoints[target_index].address().to_string().c_str(), input_ev->endpoints[target_index].port());

        if (check_timer_ != NULL) {
            check_timer_->cancel();
            check_timer_.reset();
        }
    } while (0);
    done(ec1, ec);
}

bool TcpConnectState::setup_input_event(const std::string &ip, unsigned short port) {
    bool ret = true;
    do {
        if (input_event_ == NULL || ip.empty()) {
            ret = false;
            break;
        }

        boost::system::error_code ec;
        boost::asio::ip::address addr = boost::asio::ip::address::from_string(ip, ec);
        if (ec) {
            ret = false;
            break;
        }

        tcp::endpoint endpoint(addr,port);
        TcpConnectInputEvent *ev = (TcpConnectInputEvent*)input_event_.get(); //Must be exactly right.
        ev->endpoints.push_back(endpoint);
        ev->setup = true;
    } while (0);
    return ret;
}

//////////////////////////////////TcpSendState////////////////////////////////////////
TcpSendState::TcpSendState(int32_t id):State(id), bytes_send_(0) {
    type_ = kTcpStateType_Send;
}

TcpSendState::~TcpSendState() {

}

BeeErrorCode TcpSendState::execute() {
    BeeErrorCode ret = kBeeErrorCode_Success;
    do {
        if (!check_event()) {
            ret = kBeeErrorCode_Invalid_Param;
            break;
        }

        std::shared_ptr<AsyncStateMachine> fsm = state_machine_.lock();
        if (fsm == NULL) {
            ret = kBeeErrorCode_Error_State_Machine;
            break;
        }

        TcpStateMachine *tcp_fsm = (TcpStateMachine*)fsm.get();
        if (!tcp_fsm->connected_) {
            ret = kBeeErrorCode_Not_Connected;
            break;
        }

        tcp_fsm->async_write(
            input_event_->data,
            boost::bind(&TcpSendState::handle_write,
            shared_from_base<TcpSendState>(),
            boost::asio::placeholders::error,
            boost::asio::placeholders::bytes_transferred));
    } while (0);
    return ret;
}

void TcpSendState::handle_write(const boost::system::error_code& ec2, size_t bytes_transferred) {
    BeeErrorCode ec1 = kBeeErrorCode_Write_Fail;
    bool write_more  = false;
    do {
        if (ec2) {
            break;
        }

        std::shared_ptr<AsyncStateMachine> fsm = state_machine_.lock();
        if (fsm == NULL) {
            ec1 = kBeeErrorCode_Error_State_Machine;
            break;
        }

        bytes_send_ += bytes_transferred;
        TcpStateMachine *tcp_fsm = (TcpStateMachine*)fsm.get();        
        if (tcp_fsm->check_send_buffer(bytes_transferred)) {
            tcp_fsm->async_write(
                dummy_buffer_,
                boost::bind(&TcpSendState::handle_write,
                shared_from_base<TcpSendState>(),
                boost::asio::placeholders::error,
                boost::asio::placeholders::bytes_transferred));
            write_more = true;
        } else {
            TcpSendOutputEvent *output_ev = (TcpSendOutputEvent *)output_event_.get();
            output_ev->send_bytes = bytes_send_;
            output_ev->setup = true;
            ec1 = kBeeErrorCode_Success;
        }
    } while (0);

    if (!write_more) {
        done(ec1, ec2);
    }
}

////////////////////////////////////TcpReceiveTlvState//////////////////////////////////////
TcpReceiveTlvState::TcpReceiveTlvState(int32_t id):State(id), first_read_len_(0), total_len_(0) {
    type_ = kTcpStateType_RcvTlv;
}

TcpReceiveTlvState::~TcpReceiveTlvState() {

}

BeeErrorCode TcpReceiveTlvState::execute() {
    BeeErrorCode ret = kBeeErrorCode_Success;
    do {
        if (!check_event()) {
            ret = kBeeErrorCode_Invalid_Param;
            break;
        }

        std::shared_ptr<AsyncStateMachine> fsm = state_machine_.lock();
        if (fsm == NULL) {
            ret = kBeeErrorCode_Error_State_Machine;
            break;
        }

        TcpStateMachine *tcp_fsm = (TcpStateMachine*)fsm.get();
        if (!tcp_fsm->connected_) {
            ret = kBeeErrorCode_Not_Connected;
            break;
        }

        //Not support 64bit.
        TcpRcvTlvInputEvent *ev = (TcpRcvTlvInputEvent*)input_event_.get(); //Must be exactly right.
        if (ev->length_size != 1 &&  /* uint8_t  */
            ev->length_size != 2 &&  /* uint16_t*/
            ev->length_size != 4) {  /* uint32_t*/ 
            ret = kBeeErrorCode_Error_Data;
            break;
        }

        first_read_len_ = ev->length_offset + ev->length_size;
        tcp_fsm->async_read(
            boost::asio::transfer_exactly(first_read_len_),
            boost::bind(
            &TcpReceiveTlvState::handle_read_length, 
            shared_from_base<TcpReceiveTlvState>(),
            boost::asio::placeholders::error,
            boost::asio::placeholders::bytes_transferred));
    } while (0);
    return ret;
}

void TcpReceiveTlvState::handle_read_length(const boost::system::error_code& ec2, size_t bytes_transferred) {
    BeeErrorCode ec1 = kBeeErrorCode_Read_Fail;
    do {
        std::shared_ptr<AsyncStateMachine> fsm = state_machine_.lock();
        if (fsm == NULL) {
            ec1 = kBeeErrorCode_Error_State_Machine;
            break;
        }

        if (ec2) {
            break;
        }

        if (bytes_transferred != first_read_len_) {
            ec1 = kBeeErrorCode_Error_Data;
            break;
        }

        ec1 = kBeeErrorCode_Success;
        TcpStateMachine *tcp_fsm = (TcpStateMachine*)fsm.get();
        std::istream is(&(tcp_fsm->rcv_streambuf_));
        is.read(tcp_fsm->criobuff_.data(), bytes_transferred);

        TcpRcvTlvInputEvent *input_ev = (TcpRcvTlvInputEvent*)input_event_.get();
        switch (input_ev->length_size) {
        case 1:
            total_len_ = (size_t)(*((uint8_t*)(tcp_fsm->criobuff_.data() + input_ev->length_offset)));
            break;
        case 2:
            total_len_ = ntohs((size_t)(*((uint16_t*)(tcp_fsm->criobuff_.data() + input_ev->length_offset))));
            break;
        case 4:
            total_len_ = ntohl((size_t)(*((uint32_t*)(tcp_fsm->criobuff_.data() + input_ev->length_offset))));
            break;
        default:
            ec1 = kBeeErrorCode_Invalid_Param;
            break;
        }

        if (ec1 != kBeeErrorCode_Success) {
            break;
        }

        if (total_len_ > tcp_fsm->criobuff_.capacity()) {
            IOBuffer buff(total_len_);
            memcpy(buff.data(),tcp_fsm->criobuff_.data(),bytes_transferred);
            tcp_fsm->criobuff_ = buff;
        }

        size_t left_bytes = total_len_ - bytes_transferred;
        tcp_fsm->async_read(
            boost::asio::transfer_exactly(left_bytes),
            boost::bind(
            &TcpReceiveTlvState::handle_read_data, 
            shared_from_base<TcpReceiveTlvState>(),
            boost::asio::placeholders::error,
            boost::asio::placeholders::bytes_transferred));
    } while (0);
    if (ec1 != kBeeErrorCode_Success) {
        done(ec1, ec2);
    }
}

void TcpReceiveTlvState::handle_read_data(const boost::system::error_code& ec2, size_t bytes_transferred) {
    BeeErrorCode ec1 = kBeeErrorCode_Read_Fail;
    do {
        std::shared_ptr<AsyncStateMachine> fsm = state_machine_.lock();
        if (fsm == NULL) {
            ec1 = kBeeErrorCode_Error_State_Machine;
            break;
        }

        if (ec2) {
            break;
        }

        if (total_len_ != first_read_len_ + bytes_transferred) {
            ec1 = kBeeErrorCode_Error_Data;
            break;
        }

        TcpStateMachine *tcp_fsm = (TcpStateMachine*)fsm.get();
        std::istream is(&(tcp_fsm->rcv_streambuf_));
        is.read(tcp_fsm->criobuff_.data() + first_read_len_, bytes_transferred);

        TcpRcvTlvInputEvent *input_ev = (TcpRcvTlvInputEvent*)input_event_.get(); 
        TcpRcvTlvOutputEvent *output_ev = (TcpRcvTlvOutputEvent*)output_event_.get(); 
        output_ev->rcv_buffer = (char *)tcp_fsm->criobuff_.data();
        output_ev->setup = true;
        ec1 = kBeeErrorCode_Success;
    } while (0);
    done(ec1, ec2);
}

/////////////////////////////////////TcpStateMachine/////////////////////////////////////
bool TcpStateMachine::verify_file_loaded_(false);
boost::asio::ssl::context TcpStateMachine::ssl_ctx_(boost::asio::ssl::context::sslv23);
TcpStateMachine::TcpStateMachine(IOSPtr ios):
    AsyncStateMachine(ios),
    connected_(false),
    is_tls_(false),
    criobuff_(kDefaultReadBuffLen),
    connected_endpoint_index_(-1) {       
}

TcpStateMachine::~TcpStateMachine() {

}

bool TcpStateMachine::start(const std::string &host, const std::string &service) {
    bool ret = true;
    do {
        ret = setup_state_machine();
        if (!ret) {
            break;
        }

        ret = setup_socket();
        if (!ret) {
            break;
        }

        ret = active(host, service);
    } while (0);
    return ret;
}

void TcpStateMachine::stop() {
    if (connected_) {
        boost::system::error_code error;

        if (!is_tls_) {
            sock_->lowest_layer().shutdown(boost::asio::ip::tcp::socket::shutdown_both, error);
            sock_->close(error);
            sock_.reset();
        } else {
            tls_sock_->lowest_layer().shutdown(boost::asio::ip::tcp::socket::shutdown_both, error);
            tls_sock_->lowest_layer().close(error);
            tls_sock_.reset();
        }

        std::queue<IOBuffer> empty;
        std::swap(qwiobuff_,empty);

        cwiobuff_.clear();
        criobuff_.clear();
        resolved_endpoints_.clear();

        connected_ = false;
        ios_.reset();
        uninstall_state_machine();
    }
}

bool TcpStateMachine::setup_socket(bool tls, const std::string &host) {
    bool ret = true;
    do {
        if (ios_ == NULL) {
            ret = false;
            break;
        }

        //Clear all data in write and read caches.
        reset_caches();

        boost::system::error_code ec;
        if (!tls) {
            if (sock_) {//Close old socket first.
                sock_->lowest_layer().shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
                sock_->close(ec);
            }
            sock_.reset(new tcp::socket(*ios_));
        } else {
            if (!verify_file_loaded_) {
                ssl_ctx_.set_default_verify_paths(ec);
                if (ec) {
                    ret = false;
                    break;
                }
                verify_file_loaded_ = true;
            }

            if (tls_sock_) { //Close old socket first.
                tls_sock_->lowest_layer().shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
                tls_sock_->lowest_layer().close(ec);
            }
            tls_sock_.reset(new boost::asio::ssl::stream<tcp::socket>(*ios_, ssl_ctx_));
            tls_sock_->set_verify_mode(boost::asio::ssl::verify_none);
            tls_sock_->set_verify_callback(make_verbose_verification(boost::asio::ssl::rfc2818_verification(host)));
        }

        is_tls_ = tls;
        host_ = host;
    } while (0);
    return ret;
}

void TcpStateMachine::reset_socket() {
    do {
        if (ios_ == NULL) {
            break;
        }

        //Clear all data in write and read caches.
        reset_caches();

        boost::system::error_code ec;
        if (!is_tls_) {
            if (sock_) {//Close old socket first.
                sock_->lowest_layer().shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
                sock_->close(ec);
            }
            sock_.reset(new tcp::socket(*ios_));
        } else {
            if (!verify_file_loaded_) {
                ssl_ctx_.set_default_verify_paths(ec);
                if (ec) {
                    break;
                }
                verify_file_loaded_ = true;
            }

            if (tls_sock_) { //Close old socket first.
                tls_sock_->lowest_layer().shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
                tls_sock_->lowest_layer().close(ec);
            }
            tls_sock_.reset(new boost::asio::ssl::stream<tcp::socket>(*ios_, ssl_ctx_));
            tls_sock_->set_verify_mode(boost::asio::ssl::verify_none);
            tls_sock_->set_verify_callback(make_verbose_verification(boost::asio::ssl::rfc2818_verification(host_)));
        }
    } while (0);
}

bool TcpStateMachine::active(const std::string &host, const std::string &service) {
    bool ret = true;
    do {
        if (activate_state_ == NULL) {
            ret = false;
            break;
        }

        if (activate_state_->get_type() != kTcpStateType_Resolve) {
            ret = false;
            break;
        }

        State::Ptr activate_state = activate_state_; //Count++
        TcpResolveState *tcp_resolve_state = (TcpResolveState*)activate_state.get();
        if (tcp_resolve_state == NULL) {
            ret = false;
            break;
        }

        ret = tcp_resolve_state->setup_input_event(host, service);
        if (!ret) {
            break;
        }

        ret = activate_state_machine();
    } while (0);
    return ret;
}

void TcpStateMachine::reset_caches() {
    cwiobuff_.clear();
    criobuff_.clear();
    std::queue<IOBuffer> empty;
    std::swap(qwiobuff_, empty);
    if (rcv_streambuf_.size()) {
        rcv_streambuf_.consume(rcv_streambuf_.size());
    }
}

bool TcpStateMachine::check_send_buffer(size_t bytes_transferred) {
    bool ret = true;
    do {
        if (bytes_transferred > 0) {
            cwiobuff_.consume(bytes_transferred);
        }

        if (cwiobuff_.empty()) {
            if (!qwiobuff_.empty()) {
                cwiobuff_ = qwiobuff_.front();
                qwiobuff_.pop();
            }
        }

        if (cwiobuff_.empty()) {
            ret = false;
            break;
        }
    } while (0);
    return ret;
}

} // namespace bee
