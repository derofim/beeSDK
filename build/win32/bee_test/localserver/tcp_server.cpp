#include "tcp_server.h"

std::shared_ptr<TcpServer> TcpServer::create(        
    io_service& ios, 
    const std::string ip,
    uint16_t port, 
    std::shared_ptr<TcpServerHandler> handler)
{
    return std::shared_ptr<TcpServer>(new TcpServer(ios,ip,port,handler));
}

TcpServer::TcpServer(
    io_service& ios, 
    const std::string ip,
    unsigned int port,
    std::shared_ptr<TcpServerHandler> handler): 
    ios_(ios),
    listen_ip_(ip),
    listen_port_(port),
    acceptor_(ios),
    connect_handler_(handler),
    is_running_(false)
{

}

TcpServer::~TcpServer()
{

}

bool TcpServer::start()
{
    bool ret = false;
    error_code ec;
    do 
    {
        if (is_running_)
        {
            ret = true;
            break;
        }

        if (acceptor_.is_open())
        {
            acceptor_.close(ec);
            if (ec)
            {
                break;
            }
        }

        ip::address addr = ip::address::from_string(listen_ip_,ec);
        if (ec)
        {
            break;
        }

        listen_endpoint_ = tcp::endpoint(addr, listen_port_);
        acceptor_.open(listen_endpoint_.protocol(),ec);
        if (ec)
        {
            break;
        }

        acceptor_.set_option(tcp::acceptor::reuse_address(false),ec);
        if (ec)
        {
            break;
        }

        acceptor_.set_option(tcp::acceptor::linger(true,0),ec);
        if (ec)
        {
            break;
        }

        acceptor_.bind(listen_endpoint_,ec);
        if (ec)
        {
            break;
        }

        acceptor_.listen(0x7fffffff,ec);
        if (ec)
        {
            break;
        }

        ret = true;
        is_running_ = true;
        accept_one();
    } while (0);

    return ret;
}

void TcpServer::stop()
{
    if (!is_running_)
    {
        return;
    }

    error_code ec;
    acceptor_.cancel(ec);
    acceptor_.close(ec);

    is_running_ = false;
}

void TcpServer::accept_one()
{
    if (!is_running_)
    {
        return;
    }

    std::shared_ptr<tcp::socket> socket(new tcp::socket(ios_));

    acceptor_.async_accept(
        *socket,
        boost::bind(
        &TcpServer::handle_accept,
        shared_from_this(),
        socket,
        boost::asio::placeholders::error));
}

void TcpServer::handle_accept(
    std::shared_ptr<tcp::socket> socket,
    const error_code &ec)
{
    if (!is_running_)
    {
        return;
    }

    if (!ec)
    {
        std::shared_ptr<TcpServerHandler> handler = connect_handler_.lock();
        if (handler != NULL)
        {
            handler->on_accept(socket,ec);
        }
    }
    else if (ec == boost::asio::error::operation_aborted)
    {
        return;
    }

    accept_one();
}

