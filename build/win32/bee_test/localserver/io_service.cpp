#include "io_service.h"

IOService::IOService():running_(false) {
    ios_.reset(new boost::asio::io_service);
}

IOService::~IOService() {

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

        simple_log(kSLD_Debug,"IOService started\n");
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

        simple_log(kSLD_Debug,"IOService stopped\n");
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
