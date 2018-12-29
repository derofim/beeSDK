#ifndef __HTTP_SESSION_EX_H__
#define __HTTP_SESSION_EX_H__

#include "bee/base/bee_define.h"
#include "utility/common.h"
#include "utility/timer.h"
#include "http/http_response.h"
#include "network/http_state_machine.h"

namespace bee {

typedef enum HttpStateId {
    eHttpState_Resolve = 0,
    eHttpState_Connect,
    eHttpState_Request,
    eHttpState_RcvHeader,
    eHttpState_RcvBody
}HttpStateId;

class HttpResponse;
///////////////////////////////////Callbacks///////////////////////////////////////
typedef std::function<bool(BeeErrorCode, const boost::system::error_code&, const std::vector<tcp::endpoint>&)> HttpResolveCallback;
typedef std::function<bool(BeeErrorCode, const boost::system::error_code&, const tcp::endpoint&)> HttpConnectCallback;
typedef std::function<bool(BeeErrorCode, const boost::system::error_code&, size_t)> HttpSendCallback;
typedef std::function<bool(BeeErrorCode, const boost::system::error_code&, std::shared_ptr<HttpResponse>)> HttpRcvHeaderCallback;
typedef std::function<bool(BeeErrorCode, const boost::system::error_code&, IOBuffer&)> HttpRcvContentCallback;

typedef struct HttpCallbacks {
    HttpResolveCallback     resolve_callback;
    HttpConnectCallback     connect_callback;
    HttpSendCallback        send_callback;
    HttpRcvHeaderCallback   rcv_header_callback;
    HttpRcvContentCallback  rcv_content_callback;
}HttpCallbacks;

typedef std::map<std::string, std::string, lower_comp> HttpHeaderTable;

////////////////////////////////////HttpHandler//////////////////////////////////////
class HttpHandler {
public:
    typedef std::shared_ptr<HttpHandler> SPtr;
    typedef std::weak_ptr<HttpHandler> WPtr;
    HttpHandler(){}
    virtual ~HttpHandler(){}

public:
    virtual bool handle_resolve(BeeErrorCode ec1, const boost::system::error_code &ec2, const std::vector<tcp::endpoint> &resolved_addr){return true;}
    virtual bool handle_connect(BeeErrorCode ec1, const boost::system::error_code &ec2, const tcp::endpoint &connected_endpoint){return true;}
    virtual bool handle_send(BeeErrorCode ec1, const boost::system::error_code &ec2, size_t bytes_send){return true;}
    virtual bool handle_rcv_header(BeeErrorCode ec1, const boost::system::error_code &ec2, std::shared_ptr<HttpResponse> header){return true;}
    virtual bool handle_rcv_content(BeeErrorCode ec1, const boost::system::error_code &ec2, IOBuffer &data){return true;}
};

//////////////////////////////////HttpSession////////////////////////////////////////
class HttpSession : public HttpStateMachine {
public:
    typedef std::shared_ptr<HttpSession> Ptr;
    HttpSession(IOSPtr ios);
    virtual ~HttpSession();

public:
    DECLARE_STATE_MACHINE
    virtual bool request(const std::string& url, const std::string& ref_url = "", int64_t range_start = -1, int64_t range_end = -1);
    virtual void output(StateEvent::Ptr ev);   
    int32_t get_total_time(){return (int32_t)timer_.elapsed();}
    int32_t on_resolve(StateEvent::Ptr ev);
    int32_t on_connect(StateEvent::Ptr ev);
    int32_t on_request(StateEvent::Ptr ev);
    int32_t on_rcv_header(StateEvent::Ptr ev);
    int32_t on_rcv_body(StateEvent::Ptr ev);

    bool transform_resolve_to_connect(StateEvent::Ptr input, StateEvent::Ptr output);
    bool transform_connect_to_request(StateEvent::Ptr input, StateEvent::Ptr output);
    bool transform_send_request_to_rcv_header(StateEvent::Ptr input, StateEvent::Ptr output);
    bool transform_rcv_header_to_redirection(StateEvent::Ptr input, StateEvent::Ptr output);
    bool transform_rcv_header_to_rcv_body(StateEvent::Ptr input, StateEvent::Ptr output);

    void set_handler(HttpHandler::SPtr handler){handler_ = handler;}
    void set_callbacks(const HttpCallbacks callback){http_callbacks = callback;}
    void set_wait_content_complete(bool wait){wait_content_complete_ = wait;}

    virtual BeeErrorCode handle_resolve(BeeErrorCode ec1, const boost::system::error_code &ec2, const std::vector<tcp::endpoint> &resolved_addr){return kBeeErrorCode_Success;}
    virtual BeeErrorCode handle_connect(BeeErrorCode ec1, const boost::system::error_code &ec2, const tcp::endpoint &connected_endpoint){return kBeeErrorCode_Success;}
    virtual BeeErrorCode handle_send(BeeErrorCode ec1, const boost::system::error_code &ec2, size_t bytes_send){return kBeeErrorCode_Success;}
    virtual BeeErrorCode handle_rcv_header(BeeErrorCode ec1, const boost::system::error_code &ec2, std::shared_ptr<HttpResponse> header){return kBeeErrorCode_Success;}
    virtual BeeErrorCode handle_redirect(int32_t status_code, const std::string &location) { return kBeeErrorCode_Success; }
    virtual BeeErrorCode handle_rcv_content(BeeErrorCode ec1, const boost::system::error_code &ec2, IOBuffer &data){return kBeeErrorCode_Success;}

protected:
    HttpHandler::WPtr handler_;
    HttpCallbacks     http_callbacks;
    MillisecTimer     timer_;
    bool              wait_content_complete_;
};

} // namespace bee

#endif
