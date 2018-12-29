#include "http_state_machine.h"
#include "http/http_request.h"
#include "http/http_response.h"

namespace bee {

//////////////////////////////////Constants////////////////////////////////////////
const int32_t kMaxContentLength = 20*1024*1024;

///////////////////////////////////HttpRequestState///////////////////////////////////////
HttpRequestState::HttpRequestState(int32_t id) : State(id) {
    type_ = kHttpStateType_Request;
}

HttpRequestState::~HttpRequestState() {

}

BeeErrorCode HttpRequestState::execute() {
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

        HttpStateMachine *http_fsm = (HttpStateMachine*)fsm.get();
        if (!http_fsm->connected_) {
            ret = kBeeErrorCode_Not_Connected;
            break;
        }

        HttpRequest::Ptr request = http_fsm->request_;
        if (request == NULL) {
            ret = kBeeErrorCode_Invalid_Param;
            break;
        }
        request->set_body(request->serialize_to_string());

        http_fsm->async_write(
            IOBuffer(request->body()),
            boost::bind(
            &HttpRequestState::handle_write, 
            shared_from_base<HttpRequestState>(),
            boost::asio::placeholders::error,
            boost::asio::placeholders::bytes_transferred));
    } while (0);
    return ret;
}

void HttpRequestState::handle_write(const boost::system::error_code& ec2, size_t trans_bytes) {
    BeeErrorCode ec1 = kBeeErrorCode_Write_Fail;
    do {
        if (ec2) {
            break;
        }

        TcpSendOutputEvent *output_ev = (TcpSendOutputEvent*)output_event_.get();
        output_ev->send_bytes = trans_bytes;
        output_ev->setup = true;
        ec1 = kBeeErrorCode_Success;
    } while (0);
    done(ec1, ec2);
}

//////////////////////////////////HttpRcvHeaderState////////////////////////////////////////
HttpRcvHeaderState::HttpRcvHeaderState(int32_t id) : State(id) {
    type_ = kHttpStateType_ReadHeader;
}

HttpRcvHeaderState::~HttpRcvHeaderState() {

}

BeeErrorCode HttpRcvHeaderState::execute() {
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

        HttpStateMachine *http_fsm = (HttpStateMachine*)fsm.get();
        if (!http_fsm->connected_) {
            ret = kBeeErrorCode_Not_Connected;
            break;
        }

        std::string delim("\r\n\r\n");
        http_fsm->async_read_until(
            delim,
            boost::bind(
            &HttpRcvHeaderState::handle_read_header, 
            shared_from_base<HttpRcvHeaderState>(),
            boost::asio::placeholders::error,
            boost::asio::placeholders::bytes_transferred));
    } while (0);
    return ret;
}

void HttpRcvHeaderState::handle_read_header(const boost::system::error_code& ec2, size_t trans_bytes) {
    BeeErrorCode ec1 = kBeeErrorCode_Read_Http_Header_Fail;
    do {
        if (ec2) {
            break;
        }

        std::shared_ptr<AsyncStateMachine> fsm = state_machine_.lock();
        if (fsm == NULL) {
            ec1 = kBeeErrorCode_Error_State_Machine;
            break;
        }

        HttpStateMachine *http_fsm = (HttpStateMachine*)fsm.get();
        std::istream is(&http_fsm->rcv_streambuf_);
        std::string str;
        std::string header;

        while (std::getline(is, header)) {
            str += header + '\n';
            if (header == "\r") {
                break;
            }
        }

        IOBuffer io_buf(str);
        HttpResponse::Ptr response = HttpResponse::HttpResponsePtr(new HttpResponse(io_buf)); 
        if (response == NULL || !response->is_valid()) {
            ec1 = kBeeErrorCode_Error_Data;
            break;
        }

        http_fsm->response_ = response;
        output_event_->setup = true;
        ec1 = kBeeErrorCode_Success;
    } while (0);
    done(ec1, ec2);
}

///////////////////////////////////HttpRcvBodyState///////////////////////////////////////
HttpRcvBodyState::HttpRcvBodyState(int32_t id) : 
    State(id),
    content_length_(0),
    content_offset_(0),
    file_offset_(0),
    is_chunk_(false),
    last_chunk_size_(0) {
    type_ = kHttpStateType_ReadBody;
}

HttpRcvBodyState::~HttpRcvBodyState() {

}

BeeErrorCode HttpRcvBodyState::execute() {
    BeeErrorCode ret = kBeeErrorCode_Success;
    do {
        std::shared_ptr<AsyncStateMachine> fsm = state_machine_.lock();
        if (fsm == NULL) {
            ret = kBeeErrorCode_Error_State_Machine;
            break;
        }

        HttpStateMachine *http_fsm = (HttpStateMachine*)fsm.get();
        if (!http_fsm->connected_) {
            ret = kBeeErrorCode_Not_Connected;
            break;
        }

        HttpResponse::Ptr response = http_fsm->response_;
        if (response == NULL) {
            ret = kBeeErrorCode_Error_Data;
            break;
        }

        int64_t range_beg = 0;
        int64_t range_end = 0;
        response->get_range(range_beg, range_end);
        if (range_beg != -1) {
            file_offset_ = range_beg;
        } else {
            file_offset_ = 0;
        }

        content_length_ = response->get_content_len();
        content_offset_ = 0;
        if (content_length_ == -1) {
            is_chunk_ = true;
        }

        if (response->get_status_code() != HttpResponse::SH_HTTP_STATUS_OK && 
            response->get_status_code() != HttpResponse::SH_HTTP_STATUS_PARTIAL_CONTENT) {
            ret = kBeeErrorCode_Error_Http_Status;
            break;
        }

        if (content_length_ > 0 && content_length_ < kMaxContentLength) {
            ret = read_content((size_t)content_length_);
        } else if (is_chunk_) {
            ret = read_chunk();
        } else {
            ret = kBeeErrorCode_Error_Data;
        }
    } while (0);
    return ret;
}

BeeErrorCode HttpRcvBodyState::read_content(std::size_t len) {
    BeeErrorCode ret = kBeeErrorCode_Success;
    do {
        std::shared_ptr<AsyncStateMachine> fsm = state_machine_.lock();
        if (fsm == NULL) {
            ret = kBeeErrorCode_Error_State_Machine;
            break;
        }

        HttpStateMachine *http_fsm = (HttpStateMachine*)fsm.get();
        if (!http_fsm->connected_) {
            ret = kBeeErrorCode_Not_Connected;
            break;
        }

        if (content_offset_ >= content_length_) {
            ret = kBeeErrorCode_Eof;
            break;
        }

        if (content_offset_ + len > content_length_) {
            len = std::size_t(content_length_ - content_offset_);
        }

        if (len <= http_fsm->rcv_streambuf_.size()) {
            output_event_->data = IOBuffer(len);
            std::istream is(&http_fsm->rcv_streambuf_);
            is.read(output_event_->data.data(), len);

            int64_t	file_off = file_offset_;
            content_offset_ += len;
            file_offset_ += len;

            output_event_->setup = true;
            done();
            break;
        }

        size_t need_read_len = len - http_fsm->rcv_streambuf_.size(); //Data length need to read from network.
        http_fsm->async_read(
            boost::asio::transfer_at_least(need_read_len),
            boost::bind( 
            &HttpRcvBodyState::handle_read_content, 
            shared_from_base<HttpRcvBodyState>(),
            boost::asio::placeholders::error,
            boost::asio::placeholders::bytes_transferred,
            len,
            file_offset_,
            content_offset_));
    } while (0);
    return ret;
}

BeeErrorCode HttpRcvBodyState::read_chunk() {
    return read_chunk_size();
}

BeeErrorCode HttpRcvBodyState::read_chunk_size() {
    BeeErrorCode ret = kBeeErrorCode_Success;
    do {
        std::shared_ptr<AsyncStateMachine> fsm = state_machine_.lock();
        if (fsm == NULL) {
            ret = kBeeErrorCode_Error_State_Machine;
            break;
        }

        HttpStateMachine *http_fsm = (HttpStateMachine*)fsm.get();
        if (!http_fsm->connected_) {
            ret = kBeeErrorCode_Not_Connected;
            break;
        }

        std::string delim("\r\n");
        http_fsm->async_read_until(
            delim,
            boost::bind(
            &HttpRcvBodyState::handle_read_chunk_size, 
            shared_from_base<HttpRcvBodyState>(),
            boost::asio::placeholders::error,
            boost::asio::placeholders::bytes_transferred));
    } while (0);
    return ret;
}

BeeErrorCode HttpRcvBodyState::read_chunk_body() {
    BeeErrorCode ret = kBeeErrorCode_Success;
    do {
        std::shared_ptr<AsyncStateMachine> fsm = state_machine_.lock();
        if (fsm == NULL) {
            ret = kBeeErrorCode_Error_State_Machine;
            break;
        }

        HttpStateMachine *http_fsm = (HttpStateMachine*)fsm.get();
        if (!http_fsm->connected_) {
            ret = kBeeErrorCode_Not_Connected;
            break;
        }

        if (http_fsm->rcv_streambuf_.size() >= last_chunk_size_ + 2) {
            boost::system::error_code ec2;
            fsm->ios()->post(boost::bind(&HttpRcvBodyState::handle_read_chunk_body, shared_from_base<HttpRcvBodyState>(), ec2, http_fsm->rcv_streambuf_.size()));
        } else {
            std::size_t need_read_len = last_chunk_size_ - http_fsm->rcv_streambuf_.size() + 2;
            http_fsm->async_read(
                boost::asio::transfer_at_least(need_read_len),
                boost::bind(
                &HttpRcvBodyState::handle_read_chunk_body, 
                shared_from_base<HttpRcvBodyState>(),
                boost::asio::placeholders::error,
                boost::asio::placeholders::bytes_transferred));
        }
    } while (0);
    return ret;
}

void HttpRcvBodyState::handle_read_chunk_size(const boost::system::error_code& ec2, size_t trans_bytes) {
    BeeErrorCode ec1 = kBeeErrorCode_Read_Http_Chunk_Size_Fail;
    do {
        if (ec2 && ec2 != boost::asio::error::eof) {
            break;
        }

        std::shared_ptr<AsyncStateMachine> fsm = state_machine_.lock();
        if (fsm == NULL) {
            ec1 = kBeeErrorCode_Error_State_Machine;
            break;
        }

        HttpStateMachine *http_fsm = (HttpStateMachine*)fsm.get();
        if (!http_fsm->connected_) {
            ec1 = kBeeErrorCode_Not_Connected;
            break;
        }

        if (http_fsm->rcv_streambuf_.size() <= 2) {
            ec1 = kBeeErrorCode_Error_Data;
            break;
        }

        std::istream is(&http_fsm->rcv_streambuf_);
        is >> std::hex >> last_chunk_size_;
        char ch;
        while (is.get(ch) && ch != '\n');

        if (last_chunk_size_ > 0) {
            ec1 = read_chunk_body();
            break;
        }

        ec1 = kBeeErrorCode_Success;
        report_cache_content(ec1, ec2);
    } while (0);
    if (ec1 != kBeeErrorCode_Success && ec1 != kBeeErrorCode_Eof) {
        done(ec1, ec2);
    }
}

void HttpRcvBodyState::handle_read_chunk_body(const boost::system::error_code& ec2, size_t trans_bytes) {
    BeeErrorCode ec1 = kBeeErrorCode_Read_Http_Chunk_Body_Fail;
    do {
        if (ec2 && ec2 != boost::asio::error::eof) {
            break;
        }

        HttpRcvBodyInputEvent *input_ev = (HttpRcvBodyInputEvent*)input_event_.get();
        std::shared_ptr<AsyncStateMachine> fsm = state_machine_.lock();
        if (fsm == NULL) {
            ec1 = kBeeErrorCode_Error_State_Machine;
            break;
        }

        HttpStateMachine *http_fsm = (HttpStateMachine*)fsm.get();
        if (!http_fsm->connected_) {
            ec1 = kBeeErrorCode_Not_Connected;
            break;
        }

        if (http_fsm->rcv_streambuf_.size() >= last_chunk_size_ + 2) {
            std::istream is(&http_fsm->rcv_streambuf_);
            IOBuffer io_buf(last_chunk_size_);
            is.read(io_buf.data(), last_chunk_size_);
            is.get();
            is.get();
            
            if (input_ev->wait_complete) {
                IOBuffer tmp_buf(content_buff_.size() + last_chunk_size_);
                if (!content_buff_.empty()) {
                    memcpy(tmp_buf.data(), content_buff_.data(), content_buff_.size());
                }
                memcpy(tmp_buf.data() + content_buff_.size(), io_buf.data(), io_buf.size());
                content_buff_ = tmp_buf;
            } else {
                output_event_->data = io_buf;
                http_fsm->output(output_event_);
            }

            content_offset_ += last_chunk_size_;
            ec1 = read_chunk();
        } else if (ec2 == boost::asio::error::eof) {
            ec1 = kBeeErrorCode_Eof;
            report_cache_content(ec1, ec2);
        } else {
            ec1 = read_chunk_body();
        }
    } while (0);
    if (ec1 != kBeeErrorCode_Success && ec1 != kBeeErrorCode_Eof) {
        done(ec1, ec2);
    }
}

void HttpRcvBodyState::handle_read_content(const boost::system::error_code& ec2, size_t trans_bytes, size_t need_len, int64_t data_file_off, int64_t data_content_off) {
    BeeErrorCode ec1 = kBeeErrorCode_Read_Http_Content_Fail;
    do {
        std::shared_ptr<AsyncStateMachine> fsm = state_machine_.lock();
        if (fsm == NULL) {
            ec1 = kBeeErrorCode_Error_State_Machine;
            break;
        }

        HttpStateMachine *http_fsm = (HttpStateMachine*)fsm.get();
        if (!http_fsm->connected_) {
            ec1 = kBeeErrorCode_Not_Connected;
            break;
        }

        if (ec2 && ec2 != boost::asio::error::eof) {
            break;
        }

        size_t len = std::min<size_t>(http_fsm->rcv_streambuf_.size(), need_len);
        content_buff_ = IOBuffer(len);
        std::istream is(&http_fsm->rcv_streambuf_);
        is.read(content_buff_.data(), len);

        content_offset_ += len;
        file_offset_ += len;

        if (ec2 == boost::asio::error::eof || content_offset_ > content_length_) {
            ec1 = kBeeErrorCode_Eof;
        } else {
            ec1 = kBeeErrorCode_Success;
        }
        report_cache_content(ec1, ec2);
    } while (0);
    if (ec1 != kBeeErrorCode_Success && ec1 != kBeeErrorCode_Eof) {
        done(ec1, ec2);
    }
}

void HttpRcvBodyState::report_cache_content(BeeErrorCode ec1, const boost::system::error_code& ec2) {
    if (!content_buff_.empty()) {
        output_event_->data = content_buff_;
        output_event_->setup = true;
    }
    done(ec1, ec2);
}

////////////////////////////////////HttpStateMachine//////////////////////////////////////
HttpStateMachine::HttpStateMachine(IOSPtr ios):TcpStateMachine(ios), proxy_port_(0) {

}

HttpStateMachine::~HttpStateMachine() {

}

bool HttpStateMachine::request(
    const std::string& url,
    const std::string& ref_url, 
    int64_t range_start, 
    int64_t range_end) {
    bool ret = true;
    do {
        ret = setup_state_machine();
        if (!ret) {
            break;
        }

        request_ = setup_request(url, ref_url, range_start, range_end);
        if (request_ == NULL) {
            ret = false;
            break;
        }

        std::string host;
        unsigned int port = 0;
        request_->get_conn_host_port(host, port);

        ret = setup_socket(request_->is_https(), host);
        if (!ret) {
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

        std::ostringstream os;
        os << port;
        ret = tcp_resolve_state->setup_input_event(host, os.str());
        if (!ret) {
            break;
        }

        ret = activate_state_machine();
    } while (0);
    return ret;
}

bool HttpStateMachine::redirect(const std::string &url, std::string &host, std::string &service) {
    bool ret = true;
    do {
        std::string real_path, real_host;        
        if (url.find("http://") == 0) { //Found scheme, redirect to new host, host can be found.
            size_t start_pos = strlen("http://"); //Get host begin.
            size_t end_pos = url.find('/', start_pos); //Get host end.
            if (end_pos != std::string::npos) { //If host delimiter / found.
                real_host = url.substr(start_pos, end_pos - start_pos); //Get host.
                real_path = url.substr(end_pos); //Get path/uri.
            } else { //No path/uri.
                real_host = url.substr(start_pos); //Get host by whole.
                real_path = "/";
            }
            request_->set_host(real_host); //Set new host.
        } else { //If scheme not found, it's a local redirection, host remain the same.
            if (url.find('/') != 0) {
                real_path = "/";
            }
            real_path += url; //Set path/uri.
        }
        request_->set_path(real_path); //Update path.

        unsigned int port = 0;
        request_->get_conn_host_port(host, port);
        std::ostringstream os;
        os << port;
        service = os.str();

        ret = setup_socket(request_->is_https(), host);
        if (!ret) {
            break;
        }
    } while (0);
    return ret;
}

std::shared_ptr<HttpRequest> HttpStateMachine::setup_request(
    const std::string& url,
    const std::string& ref_url, 
    int64_t range_start, 
    int64_t range_end) {
    HttpRequest::Ptr request;
    do {
        if (url.empty()) {
            break;
        }

        request = HttpRequest::create_from_url(url, ref_url, range_start, range_end);
        if (request == NULL) {
            break;
        }

        if (!user_agent_.empty()) {			
            request->set_header("User-Agent", user_agent_);
        }

        if (!cookie_.empty()) {
            request->set_header("Cookie", cookie_);
        }

        if (!proxy_host_.empty() && proxy_port_ > 0) {
            request->set_proxy(proxy_host_, proxy_port_);
        }
    } while (0);
    return request;
}

} // namespace bee
