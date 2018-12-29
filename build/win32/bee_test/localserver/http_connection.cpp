#include "http_connection.h"

#define LOCAL_SERVER_BUFFER_SIZE 200

HttpConnection::HttpConnection(std::shared_ptr<tcp::socket> socket,size_t timeout_sec, std::shared_ptr<HttpConnectionHandler> handler): 
    socket_(socket), 
    timeout_sec_(timeout_sec),
    is_stopped_(false),
    current_write_size(0),
    invalid_time_(0),
    total_size_(0),
    current_size_(0),
    header_size_(0),
    handler_(handler),
    opaque_(NULL) {
}

HttpConnection::~HttpConnection() {

}

bool HttpConnection::start() {
    return read();
}

bool HttpConnection::stop() {
    if (is_stopped_) {
        return true;
    }

    boost::system::error_code ignore_error;
    socket_->lowest_layer().shutdown(boost::asio::ip::tcp::socket::shutdown_both, ignore_error);    
    socket_->lowest_layer().close(ignore_error);
    is_stopped_ = true;

    if (request_ != NULL) {
        request_.reset();
    }

    return true;
}

bool HttpConnection::read() {
	if (is_stopped_ || socket_ == NULL) {
		return false;
	}

    socket_->async_read_some(
        boost::asio::buffer(rcv_buf_,kHttpBufferSize),
        boost::bind(&HttpConnection::handle_read,
        shared_from_this(),
        boost::asio::placeholders::error,
        boost::asio::placeholders::bytes_transferred));

    return true;
}

void HttpConnection::handle_read(const error_code &ec,size_t trans_bytes) {
    if (is_stopped_) {
        return;
    }

    bool ret = false;
    do {
        if (ec || trans_bytes == 0) {
            break;
        }

        if (rcv_data_.find("\r\n\r\n") == rcv_data_.npos) {
            invalid_time_++;
        } else {
            invalid_time_ = 0;
        }

        if (invalid_time_ > 10) {
            rcv_data_.clear();
            break;
        }

        rcv_data_ += std::string(rcv_buf_,trans_bytes);

        //TBD <policy-file-request/>

        //Parse http requests,maybe more than one in buffer(packet splicing).
        bool valid = true;
        while (1) {
            HttpRequest request;
            if (!request.assign(rcv_data_)) {
                break;
            }

            if (request.get_method() != "POST" && request.get_method() != "GET") {
                continue;
            }

            valid = on_http_request(request, opaque_);
            if (!valid) {
                break;
            }
        }

        ret = valid;
    } while (0);

    if (!ret) {
        on_http_error(ec);
    } else {
        read();
    }
}

void HttpConnection::write(const std::string& data) {
    write_with_cache(IOBuffer(data),false);
}

void HttpConnection::write(const char *data,size_t len,bool isheader,int32_t real_mp4_size) {
    if (isheader) {
        std::ostringstream stream;
        stream << "HTTP/1.1 200 OK\r\nServer: Bee\r\nContent-Type: video/mp4\r\nContent-Length: "
               << real_mp4_size
               << "\r\nConnection: close\r\n\r\n"
               << std::string(data,len);

        total_size_ = real_mp4_size;
        header_size_ = stream.str().size() - len;

        write_with_cache(IOBuffer(stream.str()),true);
    } else {
        write_with_cache(IOBuffer(data,len),false);
    }
}

void HttpConnection::write_chunk_header() {
    std::ostringstream stream;
    stream << "HTTP/1.1 200 OK\r\n"
           << "Server: Bee\r\n"
           << "Content-Type: video/mp4\r\n"
           << "Transfer-Encoding: chunked\r\n"
           << "Connection: keep-alive\r\n\r\n";
    std::string header = stream.str();
    header_size_ = header.size();
    write_with_cache(IOBuffer(header), true);
}

void HttpConnection::write_chunk(const IOBuffer &data) {
    size_t len = data.size();
    std::ostringstream stream;
    stream << std::hex << len << "\r\n";
    std::string chunk_size_hex = stream.str();
    size_t chunk_size_hex_size = chunk_size_hex.size();

    IOBuffer buff(len + chunk_size_hex_size + 2);
    memcpy(buff.data(), chunk_size_hex.c_str(), chunk_size_hex_size);
    memcpy(buff.data() + chunk_size_hex_size, data.data(), len);
    memcpy(buff.data() + chunk_size_hex_size + len, "\r\n", 2);

    write_with_cache(buff, false);
}

void HttpConnection::write_with_cache(const IOBuffer &io_buf,bool header) {
    if (is_stopped_) {
        return;
    }

	if (current_write_buffer.empty()) {
        current_write_buffer = io_buf;
        current_write_size = current_write_buffer.size();
	} else {
        write_buffer_queue.push(io_buf);
		return;
	}

    boost::asio::async_write(
        *socket_,
        boost::asio::buffer(current_write_buffer.data(), current_write_buffer.size()),
        boost::bind(
        &HttpConnection::handle_write,
        shared_from_this(),
        header,
        boost::asio::placeholders::error,
        boost::asio::placeholders::bytes_transferred));
}

void HttpConnection::handle_write(bool header,const boost::system::error_code& ec,size_t trans_bytes)
{
    if (total_size_ > 0) {
        if (header) {
            if (trans_bytes >= header_size_) {
                current_size_ += (trans_bytes - header_size_);
                header = false;
            }
        } else {
            current_size_ += trans_bytes;
            header = false;
        }
    }
 
    if (is_stopped_) {
        return;
    }

	if (ec || trans_bytes == 0) {
        on_http_error(ec);
		return;
	}
   
    size_t consume_size = std::min<size_t>(current_write_buffer.size(),trans_bytes);
    current_write_buffer.consume(consume_size);

	if (current_write_buffer.size() == 0) {
		if (!write_buffer_queue.empty()) {
            current_write_buffer = write_buffer_queue.front();
            current_write_size = current_write_buffer.size();
			write_buffer_queue.pop();
		}
	}

	if (current_write_buffer.size() > 0) {
		boost::asio::async_write(
			*socket_,
			boost::asio::buffer(current_write_buffer.data(), current_write_buffer.size()),
			boost::bind(
			&HttpConnection::handle_write,
			shared_from_this(),
            header,
			boost::asio::placeholders::error,
			boost::asio::placeholders::bytes_transferred));
	}
}

bool HttpConnection::is_queue_empty(void) {
	return write_buffer_queue.size() <= LOCAL_SERVER_BUFFER_SIZE;
}

void HttpConnection::on_http_error(const error_code &ec) {
    HttpConnectionHandler::SPtr handler = handler_.lock();
    if (handler != NULL) {
        handler->handle_connection_event(shared_from_this(), ec);
    }
}
