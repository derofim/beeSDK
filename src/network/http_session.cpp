#include "http_session.h"
#include "http/http_response.h"

namespace bee {

///////////////////////////////////AsyncStateMachine Definition///////////////////////////////////////
BEGIN_STATE_MACHINE(HttpSession)
    BEGIN_STATE(eHttpState_Resolve, TcpResolveState, &HttpSession::on_resolve, true)
        SWITCH_TO(eHttpState_Connect, &HttpSession::transform_resolve_to_connect)
    END_STATE

    BEGIN_STATE(eHttpState_Connect, TcpConnectState, &HttpSession::on_connect, false)
        SWITCH_TO(eHttpState_Request, &HttpSession::transform_connect_to_request)
    END_STATE

    BEGIN_STATE(eHttpState_Request, HttpRequestState, &HttpSession::on_request, false)
        SWITCH_TO(eHttpState_RcvHeader, &HttpSession::transform_send_request_to_rcv_header)
    END_STATE

    BEGIN_STATE(eHttpState_RcvHeader, HttpRcvHeaderState, &HttpSession::on_rcv_header, false)
        SWITCH_TO(eHttpState_Resolve, &HttpSession::transform_rcv_header_to_redirection)
        SWITCH_TO(eHttpState_RcvBody, &HttpSession::transform_rcv_header_to_rcv_body)
    END_STATE

    BEGIN_STATE(eHttpState_RcvBody, HttpRcvBodyState, &HttpSession::on_rcv_body, false)
    END_STATE
END_STATE_MACHINE

HttpSession::HttpSession(IOSPtr ios):HttpStateMachine(ios), wait_content_complete_(true) {

}

HttpSession::~HttpSession() {

}

int32_t HttpSession::on_resolve(StateEvent::Ptr ev) {
    int32_t next_state = kInvalidState;
    do {
        if (ev->ec1 != kBeeErrorCode_Success || ev->ec2) {
            break;
        }

        TcpResolveOutputEvent *output_ev = (TcpResolveOutputEvent*)ev.get();
        HttpHandler::SPtr handler = handler_.lock();
        if (handler != NULL) {
            if (!handler->handle_resolve(output_ev->ec1, output_ev->ec2, output_ev->resolved_endpoints)) {
                break;
            }
        }

        if (http_callbacks.resolve_callback != NULL) {
            if (!http_callbacks.resolve_callback(output_ev->ec1, output_ev->ec2, output_ev->resolved_endpoints)) {
                break;
            }
        }

        ev->ec1 = handle_resolve(output_ev->ec1, output_ev->ec2, output_ev->resolved_endpoints);
        if (ev->ec1 != kBeeErrorCode_Success) {            
            break;
        }

        next_state = eHttpState_Connect;
    } while (0);
    return next_state;
}

int32_t HttpSession::on_connect(StateEvent::Ptr ev) {
    int32_t next_state = kInvalidState;
    do {
        if (ev->ec1 != kBeeErrorCode_Success || ev->ec2) {
            break;
        }

        TcpConnectOutputEvent *output_ev = (TcpConnectOutputEvent*)ev.get();
        HttpHandler::SPtr handler = handler_.lock();
        if (handler != NULL) {
            if (!handler->handle_connect(output_ev->ec1, output_ev->ec2, output_ev->connected_endpoint)) {
                break;
            }
        }

        if (http_callbacks.connect_callback != NULL) {
            if (!http_callbacks.connect_callback(output_ev->ec1, output_ev->ec2, output_ev->connected_endpoint)) {
                break;
            }
        }

        ev->ec1 = handle_connect(output_ev->ec1, output_ev->ec2, output_ev->connected_endpoint);
        if (ev->ec1 != kBeeErrorCode_Success) {            
            break;
        }

        next_state = eHttpState_Request;
    } while (0);
    return next_state;
}

int32_t HttpSession::on_request(StateEvent::Ptr ev) {
    int32_t next_state = kInvalidState;
    do {
        if (ev->ec1 != kBeeErrorCode_Success || ev->ec2) {
            break;
        }

        TcpSendOutputEvent *output_ev = (TcpSendOutputEvent*)ev.get();
        HttpHandler::SPtr handler = handler_.lock();
        if (handler != NULL) {
            if (!handler->handle_send(output_ev->ec1, output_ev->ec2, output_ev->send_bytes)) {
                break;
            }
        }

        if (http_callbacks.send_callback != NULL) {
            if (!http_callbacks.send_callback(output_ev->ec1, output_ev->ec2, output_ev->send_bytes)) {
                break;
            }
        }

        ev->ec1 = handle_send(output_ev->ec1, output_ev->ec2, output_ev->send_bytes);
        if (ev->ec1 != kBeeErrorCode_Success) {
            break;
        }

        next_state = eHttpState_RcvHeader;
    } while (0);
    return next_state;
}

int32_t HttpSession::on_rcv_header(StateEvent::Ptr ev) {
    int32_t next_state = kInvalidState;
    do {
        if (ev->ec1 != kBeeErrorCode_Success || ev->ec2) {
            break;
        }        

        std::shared_ptr<HttpResponse> response = get_response();
        if (response == NULL) {
            break;
        }

        HttpHandler::SPtr handler = handler_.lock();
        if (handler != NULL) {
            if (!handler->handle_rcv_header(ev->ec1, ev->ec2, response)) {
                break;
            }
        }

        if (http_callbacks.rcv_header_callback != NULL) {
            if (!http_callbacks.rcv_header_callback(ev->ec1, ev->ec2, response)) {
                break;
            }
        }

        ev->ec1 = handle_rcv_header(ev->ec1, ev->ec2, response);
        if (ev->ec1 != kBeeErrorCode_Success) {
            break;
        }

        //Check for redirection.
        unsigned int status_code = response->get_status_code();
        if (status_code >= HttpResponse::SH_HTTP_STATUS_AMBIGUOUS && 
            status_code <= HttpResponse::SH_HTTP_STATUS_REDIRECT_KEEP_VERB) {
            if (status_code == HttpResponse::SH_HTTP_STATUS_MOVED ||              //301, permanent.
                status_code == HttpResponse::SH_HTTP_STATUS_REDIRECT ||           //302, temporarily.
                status_code == HttpResponse::SH_HTTP_STATUS_REDIRECT_METHOD ||    //303, HTTP1.1, same as 302.
                status_code == HttpResponse::SH_HTTP_STATUS_REDIRECT_KEEP_VERB) { //307, HTTP1.1, post redirect.
                std::string location = response->get_header("Location");
                ev->ec1 = handle_redirect(status_code, location);
                if (ev->ec1 != kBeeErrorCode_Success) {
                    break;
                }
                next_state = eHttpState_Resolve;
            }
            break;
        }

        next_state = eHttpState_RcvBody;
    } while (0);
    return next_state;
}

int32_t HttpSession::on_rcv_body(StateEvent::Ptr ev) {
    int32_t next_state = kInvalidState;
    do {
        if (ev->ec1 != kBeeErrorCode_Success || ev->ec2) {
            break;
        }

        HttpHandler::SPtr handler = handler_.lock();
        if (handler != NULL) {
            if (!handler->handle_rcv_content(ev->ec1, ev->ec2, ev->data)) {
                break;
            }
        }

        if (http_callbacks.rcv_content_callback != NULL) {
            if (!http_callbacks.rcv_content_callback(ev->ec1, ev->ec2, ev->data)) {
                break;
            }
        }

        ev->ec1 = handle_rcv_content(ev->ec1, ev->ec2, ev->data);
        if (ev->ec1 != kBeeErrorCode_Success) {
            break;
        }

        next_state = kFinishedState;
    } while (0);
    return next_state;
}

bool HttpSession::request(const std::string& url, const std::string& ref_url, int64_t range_start, int64_t range_end) {
    bool ret = HttpStateMachine::request(url, ref_url, range_start, range_end);
    if (ret) {
        timer_.restart();
    }
    return ret;
}

void HttpSession::output(StateEvent::Ptr ev) {
    do {
        if (ev == NULL) {
            break;
        }

        HttpHandler::SPtr handler = handler_.lock();
        if (handler != NULL) {
            if (!handler->handle_rcv_content(ev->ec1, ev->ec2, ev->data)) {
                break;
            }
        }

        if (http_callbacks.rcv_content_callback != NULL) {
            http_callbacks.rcv_content_callback(ev->ec1, ev->ec2, ev->data);
        }

        if (!handle_rcv_content(ev->ec1, ev->ec2, ev->data)) {
            break;
        }
    } while (0);
}

bool HttpSession::transform_resolve_to_connect(StateEvent::Ptr input, StateEvent::Ptr output) {
    bool ret = true;
    do {
        TcpResolveOutputEvent *resolve_ev = (TcpResolveOutputEvent*)input.get();        
        if (resolve_ev->resolved_endpoints.empty()) {
            input->ec1 = kBeeErrorCode_Resolve_Fail;
            ret = false;
            break;
        }

        TcpConnectInputEvent *connect_ev = (TcpConnectInputEvent*)output.get();
        connect_ev->endpoints = resolve_ev->resolved_endpoints; 
        connect_ev->setup = true;
    } while (0);
    return ret;
}

bool HttpSession::transform_connect_to_request(StateEvent::Ptr input, StateEvent::Ptr output) {
    bool ret = true;
    do {
        output->setup = true;
    } while (0);
    return ret;
}

bool HttpSession::transform_send_request_to_rcv_header(StateEvent::Ptr input, StateEvent::Ptr output) {
    bool ret = true;
    do {
        output->setup = true;
    } while (0);
    return ret;
}

bool HttpSession::transform_rcv_header_to_redirection(StateEvent::Ptr input, StateEvent::Ptr output) {
    bool ret = true;
    do {
        std::shared_ptr<HttpResponse> response = get_response();
        if (response == NULL) {
            ret = false;
            break;
        }

        std::string real_url = response->get_header("Location");
        if (real_url.empty()) {
            ret = false;
            break;
        }

        std::string host, service;
        ret = redirect(real_url, host, service);
        if (!ret) {
            break;
        }

        TcpResolveInputEvent *resolve_input = (TcpResolveInputEvent*)output.get();
        resolve_input->host = host;
        resolve_input->service = service;
        resolve_input->setup = true;
    } while (0);
    return ret;
}

bool HttpSession::transform_rcv_header_to_rcv_body(StateEvent::Ptr input, StateEvent::Ptr output) {
    bool ret = true;
    do {
        HttpRcvBodyInputEvent *output_ev = (HttpRcvBodyInputEvent*)output.get();
        if (output_ev == NULL) {
            ret = false;
            break;
        }

        output_ev->wait_complete = wait_content_complete_;
        output_ev->setup = true;
    } while (0);
    return ret;
}

} // namespace bee
