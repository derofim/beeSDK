#include "io_service.h"

namespace bee {

IOService::IOService():running_(false), logger_("IOService"){
    ios_.reset(new boost::asio::io_service);
    //logger_.Debug("IOService %x created\n", this);
}

IOService::~IOService() {
    //logger_.Debug("IOService %x delete\n", this);
}

bool IOService::start() {
    bool ret = true;
    do {
        if (running_) {
            break;
        }

        ios_->reset();
        work_.reset(new boost::asio::io_service::work(*ios_));
        thread_.reset(new std::thread(boost::bind(&boost::asio::io_service::run,ios_)));

        running_ = true;

        //logger_.Trace("IOService started.\n");
    } while (0);
    return ret;
}

bool IOService::stop() {
    bool ret = true;
    do {
        if (!running_) {
            break;
        }

        work_.reset();
        ios_->stop();
        wait();
        ios_.reset();

        running_ = false;

        //logger_.Trace("IOService stopped.\n");
    } while (0);
    return ret;
}

bool IOService::wait() {
    bool ret = true;
    do {
        if (!running_) {
            break;
        }

        if (thread_ == NULL) {
            break;
        }

        thread_->join();
        thread_.reset();
    } while (0);
    return ret;
}

} // namespace bee
