#ifndef __HTTP_CONNECTION_H__
#define __HTTP_CONNECTION_H__

#include "base/common.h"
#include "base/iobuffer.h"
#include "base/timer.h"
#include "base/algorithm.h"
#include "http/http_request.h"

using namespace boost::asio;
using boost::system::error_code;
using ip::tcp;

const size_t kHttpBufferSize = 32 * 1024;

//////////////////////////////////HttpConnectionHandler////////////////////////////////////////
class HttpConnection;
class HttpConnectionHandler {
public:
    typedef std::shared_ptr<HttpConnectionHandler> SPtr;
    typedef std::weak_ptr<HttpConnectionHandler> WPtr;
    HttpConnectionHandler(){}
    virtual ~HttpConnectionHandler(){}

public:
    virtual void handle_connection_event(std::shared_ptr<HttpConnection> connection, const error_code &ec) = 0;
};

///////////////////////////////////HttpConnection///////////////////////////////////////
class HttpConnection : 
    private boost::noncopyable, 
    public std::enable_shared_from_this<HttpConnection> {
public:
    typedef std::shared_ptr<HttpConnection> Ptr;
    typedef std::shared_ptr<tcp::socket> SocketPtr;

    HttpConnection(std::shared_ptr<tcp::socket> socket,size_t timeout_sec, HttpConnectionHandler::SPtr handler);
    ~HttpConnection();

public:
    bool start();
    bool stop();
    bool read();
    void write(const std::string& data);
    void write(const char *data,size_t len,bool isheader,int32_t real_mp4_size);
    void write_chunk_header();
    void write_chunk(const IOBuffer &data);
    void write_with_cache(const IOBuffer &io_buf,bool header);
    bool is_queue_empty(void);
    SOCKET native(){return socket_ == NULL?INVALID_SOCKET:socket_->native();}
    void set_opaque(void *opaque){opaque_ = opaque;}
    virtual void on_close(const error_code &ec) = 0;

protected:
    void handle_read(const error_code &ec,size_t trans_bytes);
    void handle_write(bool header,const error_code &ec,size_t trans_bytes);

    template <typename Derived>
    std::shared_ptr<Derived> shared_from_base() {
        return std::static_pointer_cast<Derived>(shared_from_this());
    }

protected:
    virtual bool on_http_request(const HttpRequest &request, void *opaque) = 0;
    virtual void on_http_write(size_t trans_bytes){}
    virtual void on_http_error(const error_code &ec);

protected:
    SocketPtr                   socket_;
    HttpRequest::HttpRequestPtr request_;
    char                        rcv_buf_[kHttpBufferSize];
    std::string                 rcv_data_;
    std::size_t                 timeout_sec_;
    bool                        is_stopped_;
    IOBuffer                    current_write_buffer;
    size_t                      current_write_size;
    std::queue<IOBuffer>        write_buffer_queue;
    size_t                      invalid_time_;
    size_t                      total_size_;
    size_t                      current_size_;
    size_t                      header_size_;
    HttpConnectionHandler::WPtr handler_;
    void                        *opaque_;
};

#endif //#ifndef __HTTP_CONNECTION_H__
