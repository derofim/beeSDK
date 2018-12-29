#ifndef __TCP_SERVER_H__
#define __TCP_SERVER_H__

#include "base/common.h"

using namespace boost::asio;
using boost::system::error_code;
using ip::tcp;

/////////////////////////////////TcpServerHandler/////////////////////////////////////////
class TcpServerHandler
{
public:
    TcpServerHandler(){}
    virtual ~TcpServerHandler(){}

public:
    virtual void on_accept(
        std::shared_ptr<tcp::socket> socket,
        const error_code &ec) = 0;
};

///////////////////////////////////TcpServer///////////////////////////////////////
class TcpServer : public std::enable_shared_from_this<TcpServer>
{
public:
    typedef std::weak_ptr<TcpServerHandler> ConnectHandlerPtr;

    static std::shared_ptr<TcpServer> create(
        io_service& ios, 
        const std::string ip,
        uint16_t port, 
        std::shared_ptr<TcpServerHandler> handler);
    ~TcpServer();

public:
    bool start();
    void stop();
    bool is_running() const { return is_running_; }

private:
    TcpServer(
        io_service& ios, 
        const std::string ip,
        unsigned int port,
        std::shared_ptr<TcpServerHandler> handler);

    void accept_one();
    void handle_accept(
        std::shared_ptr<tcp::socket> socket,
        const error_code &ec);

private:
    io_service&         ios_;
    std::string         listen_ip_;
    uint16_t            listen_port_;
    tcp::endpoint       listen_endpoint_;
    tcp::acceptor       acceptor_;
    ConnectHandlerPtr   connect_handler_;
    bool                is_running_;
};

#endif //#ifndef __TCP_SERVER_H__
