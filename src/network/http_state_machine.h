#ifndef __HTTP_STATE_MACHINE_H__
#define __HTTP_STATE_MACHINE_H__

#include "network/tcp_state_machine.h"
namespace bee {

//////////////////////////////////////Enums////////////////////////////////////
typedef enum HttpStateType {
    kHttpStateType_Resolve = kTcpStateType_Count,
    kHttpStateType_Connect,
    kHttpStateType_Request,
    kHttpStateType_ReadHeader,
    kHttpStateType_ReadBody,
    kHttpStateType_Count
}HttpStateType;

///////////////////////////////////HttpRequestState///////////////////////////////////////
class HttpRequestState : public State {
public:
    HttpRequestState(int32_t id);
    virtual ~HttpRequestState();

public:
    STATE_MACHINE_OUTPUT(TcpSendOutputEvent)
    virtual BeeErrorCode execute();
    virtual void handle_write(const boost::system::error_code& ec2, size_t trans_bytes);
};

///////////////////////////////////HttpRcvHeaderState///////////////////////////////////////
class HttpRcvHeaderState : public State {
public:
    HttpRcvHeaderState(int32_t id);
    virtual ~HttpRcvHeaderState();

public:
    virtual BeeErrorCode execute();
    virtual void handle_read_header(const boost::system::error_code& ec2, size_t trans_bytes);
};

////////////////////////////////////HttpRcvBodyState//////////////////////////////////////
struct HttpRcvBodyInputEvent : public StateEvent {
    bool wait_complete;

    HttpRcvBodyInputEvent() {
        wait_complete = true;
    }
};

class HttpRcvBodyState : public State {
public:
    HttpRcvBodyState(int32_t id);
    virtual ~HttpRcvBodyState();

public:
    STATE_MACHINE_INPUT(HttpRcvBodyInputEvent)
    virtual BeeErrorCode execute(); 

protected:
    BeeErrorCode read_content(std::size_t len);
    BeeErrorCode read_chunk();
    BeeErrorCode read_chunk_size();
    BeeErrorCode read_chunk_body();
    void handle_read_chunk_size(const boost::system::error_code& ec2, size_t trans_bytes);
    void handle_read_chunk_body(const boost::system::error_code& ec2, size_t trans_bytes);
    void handle_read_content(const boost::system::error_code& ec2, size_t trans_bytes, size_t need_len, int64_t data_file_off, int64_t data_content_off);
    void report_cache_content(BeeErrorCode ec1, const boost::system::error_code& ec2);

protected:
    int64_t  content_length_;
    int64_t  content_offset_;
    int64_t  file_offset_;
    bool     is_chunk_;
    uint32_t last_chunk_size_;
    IOBuffer content_buff_;
};

////////////////////////////////HttpStateMachine//////////////////////////////////////////
class HttpRequest;
class HttpResponse;
class HttpStateMachine : public TcpStateMachine {
public:
    typedef std::shared_ptr<HttpStateMachine> Ptr;
    HttpStateMachine(IOSPtr ios);
    virtual ~HttpStateMachine();

public:
    virtual bool request(
        const std::string& url,
        const std::string& ref_url = "", 
        int64_t range_start = -1, 
        int64_t range_end = -1);

    virtual bool redirect(const std::string &url, std::string &host, std::string &service);

    std::shared_ptr<HttpResponse> get_response(){return response_;}

protected:
    std::shared_ptr<HttpRequest> setup_request(
        const std::string& url,
        const std::string& ref_url = "", 
        int64_t range_start = -1, 
        int64_t range_end = -1);

protected:
    std::shared_ptr<HttpRequest> request_;
    std::shared_ptr<HttpResponse> response_;
    std::string cookie_;
    std::string user_agent_;
    std::string proxy_host_;
    uint32_t    proxy_port_;

    ////////////////////////////////////Friends//////////////////////////////////////
    friend class HttpConnectState;
    friend class HttpRequestState;
    friend class HttpRcvHeaderState;
    friend class HttpRcvBodyState;
};

} // namespace bee

#endif
