#ifndef __BEE_CONNECTION_H__
#define __BEE_CONNECTION_H__

#include "base/common.h"
#include "localserver/local_server.h"
#include "bee_api.h"

//////////////////////////////////Callbacks////////////////////////////////////////
#ifdef  WIN32
#define TEST_CALLBACK __stdcall
#else
#define TEST_CALLBACK
#endif

typedef bool(TEST_CALLBACK *PlayBeeStreamCb)(const std::string &stream_name, const std::string &user_agent, bee_handle &handle, void *opaque);
typedef BeeErrorCode(TEST_CALLBACK *ReadBeeStreamCb)(bee_handle handle, bee_int64_t offset, bee_int32_t len, bee_int32_t timeout, void *opaque);
typedef void(TEST_CALLBACK *StopBeeStreamCb)(bee_handle handle);

typedef struct PlayBeeCallbacks {
    PlayBeeStreamCb play;
    ReadBeeStreamCb read;
    StopBeeStreamCb stop;

    PlayBeeCallbacks() {
        play = NULL;
        read = NULL;
        stop = NULL;
    }
}PlayBeeCallbacks;

//////////////////////////////////BeeConnection////////////////////////////////////////
class BeeConnection : public HttpConnection {
public:
    typedef std::shared_ptr<BeeConnection> Ptr;
    BeeConnection(std::shared_ptr<tcp::socket> socket, size_t timeout_sec, HttpConnectionHandler::SPtr handler);
    virtual ~BeeConnection();

public:
    bool on_http_request(const HttpRequest &request, void *opaque);
    void on_play(bool success, bee_handle handle);
    void on_read(bee_handle handle, const bee_uint8_t *data, bee_uint32_t len, bool eof);   

protected:
    bool parse_request(const std::string head_line, std::string &stream_name);
    void on_close(const error_code &ec);
    void read_data();
    void dispatch_data(const IOBuffer &data, bool eof);

protected:
    int32_t bee_handle_;
    boost::asio::io_service &ios_;
    PlayBeeStreamCb play_;
    ReadBeeStreamCb read_;
    StopBeeStreamCb stop_;
    FILE *fp_;
};

//////////////////////////////////BeeLocalServer////////////////////////////////////////
typedef LocalServer<BeeConnection> BeeLocalServer;

#endif
