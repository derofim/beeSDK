#include "bee_promise.h"
#include "bee/base/bee_define.h"

namespace bee {

///////////////////////////////////BeePromise///////////////////////////////////////
BeePromise::BeePromise():
    session_id_(-1), 
    honored_(false),
    sync_(true) {
}

BeePromise::~BeePromise() {
}

void BeePromise::active(IOSPtr ios, int32_t timeout) {
    if (timeout > 0 && ios != NULL) {
        timer_ = AsyncWaitTimer::create(*ios);
        timer_->set_wait_millseconds(timeout);
        timer_->set_wait_times(1);
        timer_->set_immediately(false);
        timer_->async_wait(boost::bind(&BeePromise::handle_timeout, shared_from_this()));
    }

    if (sync_) {
        promise_.reset(new std::promise<BeeErrorCode>);
    }
}

void BeePromise::honor(BeeErrorCode ec1, bool reschedule_timer) {
    if (sync_) {
        if (!honored_) {
            if (timer_ != NULL) {
                timer_->cancel();
                timer_.reset();
            }
            sync_honor(ec1);
            honored_ = true;
        }
    } else {
        if (timer_ != NULL) {
            if (reschedule_timer) {
                timer_->re_async_wait();
            } else {
                timer_->cancel();
                timer_.reset();
            }
        }
        async_honor(ec1);
    }
}

void BeePromise::give_up() {
    honor(kBeeErrorCode_Give_Up);
}

BeeErrorCode BeePromise::wait() {
    if (sync_) {
        if (promise_ != NULL) {
            std::shared_future<BeeErrorCode> future = promise_->get_future();
            return future.get();
        } else {
            return kBeeErrorCode_Null_Pointer;
        }
    } else {
        return kBeeErrorCode_Not_Implemented;
    }
}

BeeErrorCode BeePromise::wait_for(TimeType ms) {
    if (sync_) {
        if (promise_ != NULL) {
            std::shared_future<BeeErrorCode> future = promise_->get_future();
            std::future_status status = future.wait_until(std::chrono::system_clock::now() + std::chrono::milliseconds(ms));
            if (status == std::future_status::ready) {
                return kBeeErrorCode_Success;
            } else {
                return kBeeErrorCode_Timeout;
            }
        } else {
            return kBeeErrorCode_Null_Pointer;
        }
    } else {
        return kBeeErrorCode_Not_Implemented;
    }
}

void BeePromise::handle_timeout() {
    honor(kBeeErrorCode_Timeout);
}

void BeePromise::sync_honor(BeeErrorCode ec1) {
    if (promise_ != NULL) {
        promise_->set_value(ec1);
    }
}

////////////////////////////////////BeeAsyncInitPromise//////////////////////////////////////
BeeAsyncInitPromise::BeeAsyncInitPromise():callback_(NULL), opaque_(NULL) {
    sync_ = false;
}

BeeAsyncInitPromise::~BeeAsyncInitPromise() {

}

BeeErrorCode BeeAsyncInitPromise::wait() {
    return kBeeErrorCode_Success;
}

void BeeAsyncInitPromise::async_honor(BeeErrorCode ec1) {
    if (callback_ != NULL) {
        callback_(ec1, ec2_, opaque_);
    }
}

void BeeAsyncInitPromise::give_up() {
    if (callback_ != NULL) {
        callback_(kBeeErrorCode_Give_Up, -1, opaque_);
    }
}

////////////////////////////////////BeeAsyncOpenPromise//////////////////////////////////////
BeeAsyncOpenPromise::BeeAsyncOpenPromise():callback_(NULL) {
    sync_ = false;
}

BeeAsyncOpenPromise::~BeeAsyncOpenPromise() {

}

BeeErrorCode BeeAsyncOpenPromise::wait() {
    return kBeeErrorCode_Success;
}

void BeeAsyncOpenPromise::async_honor(BeeErrorCode ec1) {
    if (callback_ != NULL) {
        callback_(session_id_, ec1, opaque_);
    }
}

void BeeAsyncOpenPromise::give_up() {
    if (callback_ != NULL) {
        callback_(-1, kBeeErrorCode_Give_Up, opaque_);
    }
}

} // namespace bee
