#include "bee_connection.h"

//////////////////////////////////Http Constants////////////////////////////////////////
const char *kFlashCrossDomainHeader   = "<cross-domain-policy><allow-access-from domain=\"*\"to-ports=\"*\"/></cross-domain-policy>\0";
const char *kHttpSuccess              = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: 0\r\nConnection: close\r\n\r\n";
const size_t kReadLenOnce = 32*1024;

////////////////////////////////////BeeConnection//////////////////////////////////////
BeeConnection::BeeConnection(
    std::shared_ptr<tcp::socket> socket,
    size_t timeout_sec, 
    HttpConnectionHandler::SPtr handler):
    HttpConnection(socket, timeout_sec, handler),
    ios_(socket->get_io_service()),
    bee_handle_(NULL),
    play_(NULL),
    read_(NULL),
    stop_(NULL),
    fp_(NULL) {
}

BeeConnection::~BeeConnection() {
}

bool BeeConnection::on_http_request(const HttpRequest &request, void *opaque) {    
    bool ret = false;
    do {
        std::ostringstream stream;
        std::string head_line = request.get_path();
        if(head_line.find("/crossdomain.xml") != std::string::npos)
        {//crossdomain
            stream << "HTTP/1.1 200 OK\r\nContent-Type: text/xml\r\nContent-Length: "
                   << strlen(kFlashCrossDomainHeader)
                   << "\r\n\r\n"
                   << kFlashCrossDomainHeader;
            write(stream.str());
            ret = true;
            break;
        }

        std::string stream_name;
        ret = parse_request(head_line, stream_name);
        if (!ret) {
            break;
        }
        
        if (opaque == NULL) {
            ret = false;
            break;
        }

        PlayBeeCallbacks *cbs = (PlayBeeCallbacks*)opaque;
        play_ = cbs->play;
        read_ = cbs->read;
        stop_ = cbs->stop;
        if (play_ == NULL || read_ == NULL || stop_ == NULL) {
            ret = false;
            break;
        }

//         char f[32] = {0};
//         sprintf(f, "%d.mp4", native());
//         fp_ = fopen(f, "wb+");

        std::string user_agent = request.get_header("User-Agent");
        ret = play_(stream_name, user_agent, bee_handle_, (void*)native());
        if (!ret) {
            break;
        }

        write_chunk_header();        
    } while (0);
    return ret;
}

bool BeeConnection::parse_request(const std::string head_line, std::string &stream_name) {
    bool ret = true;
    do {
        if (head_line.empty()) {
            ret = false;
            break;
        }

        std::vector<std::string> out1;
        Split(out1, head_line, "?");
        if (out1.size() != 2 || out1[0].empty() || out1[1].empty()) {
            ret = false;
            break;
        }

        if (out1[0] != "/bee") {
            ret = false;
            break;
        }

        std::vector<std::string> out2;
        Split(out2, out1[1], "=");
        if (out2.size() != 2 || out2[0].empty() || out2[1].empty()) {
            ret = false;
            break;
        }

        if (out2[0] != "stream") {
            ret = false;
            break;
        }

        stream_name = out2[1];
    } while (0);
    return ret;
}

void BeeConnection::on_close(const error_code &ec) {
    simple_log(kSLD_Debug, "[%x] close connection err:%d, msg:%s\n", this, ec.value(), ec.message().c_str());
    if (stop_ != NULL) {
        stop_(bee_handle_);
    }

    if (fp_ != NULL) {
        fclose(fp_);
        fp_ = NULL;
    }
}

void BeeConnection::read_data() {
    if (read_ != NULL) {
        read_(bee_handle_, 0, kReadLenOnce, -1, (void*)native());
    }
}

void BeeConnection::on_play(bool success, bee_handle handle) {
    if (success) {
        bee_handle_ = handle;
        read_data();
    } else {
        boost::system::error_code ec(10000, boost::system::generic_category());
        on_http_error(ec);
    }
}

void BeeConnection::on_read(bee_handle handle, const bee_uint8_t *data, bee_uint32_t len, bool eof) {
    do {
        IOBuffer buff(len);
        if (len > 0) {
            memcpy(buff.data(), data, len);
        }
        ios_.post(boost::bind(&BeeConnection::dispatch_data, shared_from_base<BeeConnection>(), buff, eof));
    } while (0);
}

void BeeConnection::dispatch_data(const IOBuffer &data, bool eof) {
    if (!data.empty()) {
        write_chunk(data);
    }

    if (eof) {
        read_data();
    }

    if (fp_ != NULL) {
        fwrite(data.data(), data.size(), 1, fp_);
    }   
}