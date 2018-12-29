#ifndef __TCP_STATE_MACHINE_H__
#define __TCP_STATE_MACHINE_H__

#include "state/async_state_machine.h"
#include "utility/timer.h"
#include "log/logger.h"

namespace bee {

const size_t kDefaultReadBuffLen = 32 * 1024;
const int32_t kDefaultTcpConnectTimeout = 10000;
const int32_t kTcpCheckConnectPeriod = 200;

///////////////////////////////////VerboseVerification///////////////////////////////////////
template <typename Verifier>
class VerboseVerification {
public:
    VerboseVerification(Verifier verifier) : verifier_(verifier) {}
    bool operator()(bool preverified, boost::asio::ssl::verify_context &ctx) {
        char subject_name[256] = { 0 };
        X509 *cert = X509_STORE_CTX_get_current_cert(ctx.native_handle());
        if (cert != NULL) {
            X509_NAME_oneline(X509_get_subject_name(cert), subject_name, 256);
            bool verified = verifier_(preverified, ctx);
            std::string str(subject_name);
            return verified;
        } else {
            return false;
        }
    }

private:
    Verifier verifier_;
};

template <typename Verifier>
VerboseVerification<Verifier> make_verbose_verification(Verifier verifier) {
    return VerboseVerification<Verifier>(verifier);
}

///////////////////////////////////Enums///////////////////////////////////////
typedef enum TcpStateType {
    kTcpStateType_Resolve = 0,
    kTcpStateType_Connect,
    kTcpStateType_Send,
    kTcpStateType_RcvTlv,
    kTcpStateType_Count
}TcpStateType;

//////////////////////////////////TcpResolveState////////////////////////////////////////
struct TcpResolveInputEvent : public StateEvent {
    std::string host;
    std::string service;
};

struct TcpResolveOutputEvent : public StateEvent {
    std::vector<tcp::endpoint> resolved_endpoints;
};

class TcpResolveState : public State {
public:
    TcpResolveState(int32_t id);
    virtual ~TcpResolveState();

public:
    STATE_MACHINE_INPUT(TcpResolveInputEvent)
    STATE_MACHINE_OUTPUT(TcpResolveOutputEvent)
    virtual BeeErrorCode execute();
    void handle_resolve(const boost::system::error_code& ec2, tcp::resolver::iterator endpoint_iter);
    bool setup_input_event(const std::string &host, const std::string &service = "80");
    bool check_host_addr(const std::string &host, boost::asio::ip::address &addr);

protected:
    std::shared_ptr<tcp::resolver> resolver_;
    TcpResolverQueryPtr query_;
};

//////////////////////////////////TcpConnectState////////////////////////////////////////
struct TcpConnectInputEvent : public StateEvent {
    std::vector<tcp::endpoint> endpoints;
    int32_t start_index = 0;
    int32_t connect_timeout = kDefaultTcpConnectTimeout;
};

struct TcpConnectOutputEvent : public StateEvent {
    tcp::endpoint connected_endpoint;
    int32_t index;

    TcpConnectOutputEvent() {
        index = -1;
    }
};

class TcpConnectState : public State {
public:
    TcpConnectState(int32_t id);
    virtual ~TcpConnectState();

public:
    STATE_MACHINE_INPUT(TcpConnectInputEvent)
    STATE_MACHINE_OUTPUT(TcpConnectOutputEvent)
    virtual BeeErrorCode execute();
    void handle_connect(const boost::system::error_code& ec2, tcp::resolver::iterator endpoint_iter, int32_t index);
    void handle_period_timeout();
    void handle_handshake(const boost::system::error_code& ec);
    bool setup_input_event(const std::string &ip, unsigned short port);

protected:
    int32_t index_ = 0;
    int32_t total_count_ = 0;
    int32_t tried_count_ = 0;
    AsyncWaitTimer::Ptr check_timer_;
    int32_t current_spent_time_ = 0;
    int32_t current_endpoint_spent_time_ = 0;
    int32_t total_timeout_ = kDefaultTcpConnectTimeout;
    int32_t endpoint_timeout_ = kDefaultTcpConnectTimeout;
    Logger logger_;
};

////////////////////////////////////TcpSendOutputEvent//////////////////////////////////////
struct TcpSendOutputEvent : public StateEvent {
    size_t send_bytes;

    TcpSendOutputEvent() {
        send_bytes = 0;
    }
};

class TcpSendState : public State {
public:
    STATE_MACHINE_OUTPUT(TcpSendOutputEvent)
    TcpSendState(int32_t id);
    virtual ~TcpSendState();

public:
    virtual BeeErrorCode execute();
    void handle_write(const boost::system::error_code& ec2, size_t bytes_transferred);

protected:
    size_t bytes_send_;
    IOBuffer dummy_buffer_;
};

//////////////////////////////////TcpReceiveTlvState////////////////////////////////////////
struct TcpRcvTlvInputEvent : public StateEvent {
    size_t length_offset;
    size_t length_size;

    TcpRcvTlvInputEvent() {
        length_offset = 0;
        length_size = 0;
    }
};

struct TcpRcvTlvOutputEvent : public StateEvent {
    char *rcv_buffer;

    TcpRcvTlvOutputEvent() {
        rcv_buffer = NULL;
    }
};

class TcpReceiveTlvState : public State {
public:
    TcpReceiveTlvState(int32_t id);
    virtual ~TcpReceiveTlvState();

public:
    STATE_MACHINE_INPUT(TcpRcvTlvInputEvent)
    STATE_MACHINE_OUTPUT(TcpRcvTlvOutputEvent)
    virtual BeeErrorCode execute();
    void handle_read_length(const boost::system::error_code& ec2, size_t bytes_transferred);
    void handle_read_data(const boost::system::error_code& ec2, size_t bytes_transferred);

protected:
    size_t first_read_len_;
    size_t total_len_;
};

//////////////////////////////////TcpStateMachine////////////////////////////////////////
class TcpStateMachine : public AsyncStateMachine {
public:
    typedef std::shared_ptr<TcpStateMachine> Ptr;
    TcpStateMachine(IOSPtr ios);
    virtual ~TcpStateMachine();

public:
    //Start from domain name.
    virtual bool start(const std::string &host, const std::string &service = "80");

    //Close socket & uninstall state machine.
    virtual void stop();

public:
    template <typename ConnectHandler>
    BOOST_ASIO_INITFN_RESULT_TYPE(ConnectHandler, void(boost::system::error_code)) async_connect(const tcp::endpoint& peer_endpoint, BOOST_ASIO_MOVE_ARG(ConnectHandler) handler);

    template <typename WriteHandler>
    BOOST_ASIO_INITFN_RESULT_TYPE(WriteHandler, void(boost::system::error_code, std::size_t)) async_write(const IOBuffer &data, BOOST_ASIO_MOVE_ARG(WriteHandler) handler);

    template <typename ReadHandler>
    BOOST_ASIO_INITFN_RESULT_TYPE(ReadHandler, void(boost::system::error_code, std::size_t)) async_read_until(const std::string& delim, BOOST_ASIO_MOVE_ARG(ReadHandler) handler);

    template <typename CompletionCondition, typename ReadHandler>
    BOOST_ASIO_INITFN_RESULT_TYPE(ReadHandler, void(boost::system::error_code, std::size_t)) async_read(CompletionCondition completion_condition, BOOST_ASIO_MOVE_ARG(ReadHandler) handler);

    template <typename HandshakeHandler>
    BOOST_ASIO_INITFN_RESULT_TYPE(HandshakeHandler, void(boost::system::error_code)) async_handshake(boost::asio::ssl::stream_base::handshake_type type, BOOST_ASIO_MOVE_ARG(HandshakeHandler) handler);

    bool check_send_buffer(size_t bytes_transferred);
    bool is_tls() { return is_tls_; }

protected:
    bool setup_socket(bool tls = false, const std::string &host = "");
    void reset_socket();
    bool active(const std::string &host, const std::string &service);
    void reset_caches();

protected:
    bool connected_;
    IOBuffer cwiobuff_;
    IOBuffer criobuff_;
    std::queue<IOBuffer> qwiobuff_;
    boost::asio::streambuf rcv_streambuf_;
    bool is_tls_;
    std::string host_;
    std::shared_ptr<tcp::socket> sock_;
    std::shared_ptr<boost::asio::ssl::stream<tcp::socket> > tls_sock_;
    tcp::endpoint connected_endpoint_;
    int32_t connected_endpoint_index_;
    std::vector<tcp::endpoint> resolved_endpoints_;
    static bool verify_file_loaded_;
    static boost::asio::ssl::context ssl_ctx_;

    //////////////////////////////////friends////////////////////////////////////////
    friend class TcpResolveState;
    friend class TcpConnectState;
    friend class TcpSendState;
    friend class TcpReceiveTlvState;
};

template <typename ConnectHandler>
BOOST_ASIO_INITFN_RESULT_TYPE(ConnectHandler, void(boost::system::error_code)) TcpStateMachine::async_connect(const tcp::endpoint& peer_endpoint, BOOST_ASIO_MOVE_ARG(ConnectHandler) handler) {
    if (!is_tls_) {
        BOOST_ASSERT(sock_ != NULL);
        return sock_->async_connect(peer_endpoint, handler);
    } else {
        BOOST_ASSERT(tls_sock_ != NULL);
        return tls_sock_->lowest_layer().async_connect(peer_endpoint, handler);
    }
}

template <typename WriteHandler>
BOOST_ASIO_INITFN_RESULT_TYPE(WriteHandler, void(boost::system::error_code, std::size_t)) TcpStateMachine::async_write(const IOBuffer &data, BOOST_ASIO_MOVE_ARG(WriteHandler) handler) {
    if (!data.empty()) {
        if (cwiobuff_.empty()) {
            cwiobuff_ = data.clone();
        } else {
            qwiobuff_.push(data.clone());
            return; //Sending something now, just wait for write done event after cached send data.
        }
    }

    if (cwiobuff_.empty()) {
        if (!qwiobuff_.empty()) {
            cwiobuff_ = qwiobuff_.front();
            qwiobuff_.pop();
        } else {
            return;
        }
    }

    if (!is_tls_) {
        BOOST_ASSERT(sock_ != NULL);
        return boost::asio::async_write((*sock_), boost::asio::buffer(cwiobuff_.data(), cwiobuff_.size()), handler);
    } else {
        BOOST_ASSERT(tls_sock_ != NULL);
        return boost::asio::async_write((*tls_sock_), boost::asio::buffer(cwiobuff_.data(), cwiobuff_.size()), handler);
    }
}

template <typename ReadHandler>
BOOST_ASIO_INITFN_RESULT_TYPE(ReadHandler, void(boost::system::error_code, std::size_t)) TcpStateMachine::async_read_until(const std::string& delim, BOOST_ASIO_MOVE_ARG(ReadHandler) handler) {
    if (!is_tls_) {
        BOOST_ASSERT(sock_ != NULL);
        return boost::asio::async_read_until((*sock_), rcv_streambuf_, delim, handler);
    } else {
        BOOST_ASSERT(tls_sock_ != NULL);
        return boost::asio::async_read_until((*tls_sock_), rcv_streambuf_, delim, handler);
    }
}

template <typename CompletionCondition, typename ReadHandler>
BOOST_ASIO_INITFN_RESULT_TYPE(ReadHandler, void(boost::system::error_code, std::size_t)) TcpStateMachine::async_read(CompletionCondition completion_condition, BOOST_ASIO_MOVE_ARG(ReadHandler) handler) {
    if (!is_tls_) {
        BOOST_ASSERT(sock_ != NULL);
        return boost::asio::async_read((*sock_), rcv_streambuf_, completion_condition, handler);
    } else {
        BOOST_ASSERT(tls_sock_ != NULL);
        return boost::asio::async_read((*tls_sock_), rcv_streambuf_, completion_condition, handler);
    }
}

template <typename HandshakeHandler>
BOOST_ASIO_INITFN_RESULT_TYPE(HandshakeHandler, void(boost::system::error_code)) TcpStateMachine::async_handshake(boost::asio::ssl::stream_base::handshake_type type, BOOST_ASIO_MOVE_ARG(HandshakeHandler) handler) {
    BOOST_ASSERT(tls_sock_ != NULL);
    return tls_sock_->async_handshake(type, handler);
}

} // namespace bee

#endif
