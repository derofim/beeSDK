#ifndef __LOCAL_SERVER_H__
#define __LOCAL_SERVER_H__

#include "io_service.h"
#include "tcp_server.h"
#include "http_connection.h"

template <class T> //T must be derived from HttpConnection
class LocalServer : 
    public TcpServerHandler,
    public HttpConnectionHandler,
    public std::enable_shared_from_this<LocalServer<T> > {
public:
    typedef std::shared_ptr<LocalServer> Ptr;
    typedef std::shared_ptr<TcpServer> TcpServerPtr;
    typedef std::shared_ptr<T> ConnectionPtr;
    typedef std::unordered_map<SOCKET,ConnectionPtr> ConnectionTable;
    LocalServer();
    ~LocalServer();

public:
    bool start(const std::string &ip, uint16_t port, void *opaque);
    bool stop();
    void shutdown(HttpConnection::Ptr connection, const error_code &ec);
    void on_accept(std::shared_ptr<tcp::socket> socket,const error_code &ec);
    void handle_connection_event(HttpConnection::Ptr connection, const error_code &ec);
    HttpConnection::Ptr get_connection(SOCKET sock);

private:
    IOService::Ptr ios_;
    TcpServerPtr tcp_server_; 
    ConnectionTable connection_table_;
    bool is_running_;
    std::string local_ip_;
    uint16_t local_port_;
    void *opaque_;
};

template <class T>
LocalServer<T>::LocalServer():
    is_running_(false),
    local_ip_("0.0.0.0"),
    local_port_(8888),
    opaque_(NULL) {

}

template <class T>
LocalServer<T>::~LocalServer() {

}

template <class T>
bool LocalServer<T>::start(const std::string &ip, uint16_t port, void *opaque) {
    bool ret = true;
    do {
        if (is_running_) {
            break;
        }

        local_ip_ = ip;
        local_port_ = port;
        opaque_ = opaque;

        ios_.reset(new IOService);
        ret = ios_->start();
        if (!ret) {
            break;
        }

        if (tcp_server_ != NULL) {
            tcp_server_->stop();
            tcp_server_.reset();
        }

        tcp_server_ = TcpServer::create(*(ios_->ios()),local_ip_,local_port_,shared_from_this());
        ret = tcp_server_->start();
    } while (0);
    return ret;
}

template <class T>
bool LocalServer<T>::stop() {
    if (tcp_server_ != NULL) {
        tcp_server_->stop();
        tcp_server_.reset();
    }

    connection_table_.clear();
    if (ios_ != NULL) {
        ios_->stop();
    }
    return true;
}

template <class T>
void LocalServer<T>::on_accept(std::shared_ptr<tcp::socket> socket, const error_code &ec) {
    do {
        if (ec) {
            break;
        }

        SOCKET sock = socket->native();
        ConnectionTable::iterator iter = connection_table_.find(sock);
        if (iter != connection_table_.end()) {
            iter->second->stop();
            connection_table_.erase(iter);
        }

        ConnectionPtr connection(new T(socket, 5, shared_from_this()));
        connection->set_opaque(opaque_);
        connection_table_[sock] = connection;
        connection->start();
    } while (0);
}

template <class T>
void LocalServer<T>::handle_connection_event(HttpConnection::Ptr connection, const error_code &ec) {
    if (ec) {
        shutdown(connection, ec);
    }
}

template <class T>
HttpConnection::Ptr LocalServer<T>::get_connection(SOCKET sock) {
    ConnectionTable::iterator iter = connection_table_.find(sock);
    if (iter != connection_table_.end()) {
        return iter->second;
    } else {
        return HttpConnection::Ptr();
    }
}

template <class T>
void LocalServer<T>::shutdown(HttpConnection::Ptr connection, const error_code &ec) {
    do {
        if (connection == NULL) {
            break;
        }

        ConnectionTable::iterator iter = connection_table_.find(connection->native());
        if (iter == connection_table_.end()) {
            break;
        }

        connection->stop();
        connection->on_close(ec);
        connection_table_.erase(iter);
    } while (0);
}

#endif //#ifndef __LOCAL_SERVER_H__
