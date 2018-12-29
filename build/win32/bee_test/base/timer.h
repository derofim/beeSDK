#ifndef __TIMER_H__
#define __TIMER_H__

#include "common.h"

#ifdef WIN32
#else
#include <sys/time.h>
#endif

#if (BOOST_VERSION >= 104900)
#include <boost/asio/steady_timer.hpp>
#endif

#ifdef __IOS__
#include <mach/mach_time.h>
#endif

typedef uint64_t TimeType;

class MillisecTimer {
public:
    MillisecTimer() {restart();}
    ~MillisecTimer() {}

    void restart() {
        tick_count_ = this->get_tickcount();
    }

    void forward(TimeType forward_tick_cont) {
        tick_count_ += forward_tick_cont;
    }

    TimeType elapsed() {
        TimeType now_tick_count = this->get_tickcount();
        if (now_tick_count < tick_count_) {
            return 0xFFFFFFFF-tick_count_+now_tick_count;
        } else {
            return now_tick_count - tick_count_;
        }
    }

    static TimeType get_tickcount() {
#ifdef WIN32
        //return ::GetTickCount();
        LARGE_INTEGER freq,counter;
        QueryPerformanceFrequency(&freq);
        QueryPerformanceCounter(&counter);  
        return (1000000L * counter.QuadPart / freq.QuadPart)/1000L;
#elif defined ANDROID
		timespec ts;
		clock_gettime(CLOCK_MONOTONIC,&ts);
		return ts.tv_sec * (TimeType)1000 + ts.tv_nsec / 1000000;
#elif defined __IOS__
		mach_timebase_info_data_t mach_info;
		mach_timebase_info(&mach_info);
		double factor = static_cast<double>(mach_info.numer) / mach_info.denom;
		return (mach_absolute_time() * factor) / 1000000L;
#elif defined _LINUX_
		timespec ts;
		clock_gettime(CLOCK_MONOTONIC,&ts);
		return ts.tv_sec * (TimeType)1000 + ts.tv_nsec / 1000000;
#else 
		struct timeval tv;
		gettimeofday(&tv, 0);
		return tv.tv_sec * (TimeType)1000 + tv.tv_usec / 1000;
#endif
    }

private:
    TimeType tick_count_;
};

typedef MillisecTimer TickTimer;

class MicrosecTimer {
public:
    MicrosecTimer() {}
    ~MicrosecTimer() {}

    void restart() {

    }

    TimeType elapsed() {
        return 0;
    }

private:
};

class AsyncWaitTimer : public std::enable_shared_from_this<AsyncWaitTimer> {
public:
    typedef std::shared_ptr<AsyncWaitTimer> Ptr;
    typedef boost::function<void(int32_t)> WaitFunc;
    
    AsyncWaitTimer::Ptr static create(boost::asio::io_service &ios) {
        return AsyncWaitTimer::Ptr(new AsyncWaitTimer(ios));
    }

public:
    bool is_cancel() const { return is_cancel_; }

    void set_wait_millseconds(int32_t mill_sec) {
        dead_mill_seconds_ = mill_sec;
    }

    void set_wait_seconds(int32_t sec) {
        dead_mill_seconds_ = sec * 1000;
    }

    void set_wait_times(int32_t times) {
        total_times_ = times;
        if (times != 0) {
            //is_cancel_ = false;
        }
    }

    void clear_invoke_times() { invoke_times_ = 0; }

    void async_wait(WaitFunc wait_func) {
        if (!is_cancel_ && dead_mill_seconds_) {
            wait_func_ = wait_func;
            invoke_times_ = 0;
#if (BOOST_VERSION >= 104900)
			using std::chrono::milliseconds;
#else
			using boost::posix_time::milliseconds;
#endif
            boost::system::error_code ec;
            deadline_timer_.expires_from_now(immediately_?milliseconds(0):milliseconds(dead_mill_seconds_), ec);
            deadline_timer_.async_wait(
                boost::bind(
                &AsyncWaitTimer::on_timer, 
                shared_from_this(),
                boost::asio::placeholders::error));
        }
    }

    void re_async_wait() {
        if (!is_cancel_ && dead_mill_seconds_) {
#if (BOOST_VERSION >= 104900)
			using std::chrono::milliseconds;
#else
			using boost::posix_time::milliseconds;
#endif
            boost::system::error_code ec;
            deadline_timer_.expires_from_now(immediately_?milliseconds(0):milliseconds(dead_mill_seconds_), ec);
            deadline_timer_.async_wait(
                boost::bind(
                &AsyncWaitTimer::on_timer, 
                shared_from_this(),
                boost::asio::placeholders::error));
        }
    }

    void cancel() {
        is_cancel_ = true;
        boost::system::error_code ec;
        deadline_timer_.cancel(ec);
    }

    void set_immediately(bool immediately){immediately_ = immediately;}

private:
    AsyncWaitTimer(boost::asio::io_service &ios) : 
       deadline_timer_(ios), 
       total_times_(-1),
       is_cancel_(false), 
       invoke_times_(0), 
       dead_mill_seconds_(0),
       immediately_(true) {}

    void on_timer(const boost::system::error_code &ec) {
        if (wait_func_ && !is_cancel_ && ec != boost::asio::error::operation_aborted) {
            wait_func_(invoke_times_++);
            if (total_times_ < 0 || (total_times_ > 0 && invoke_times_ < total_times_)) {
#if (BOOST_VERSION >= 104900)
                using std::chrono::milliseconds;
#else
				using boost::posix_time::milliseconds;
#endif
                boost::system::error_code ec;
                deadline_timer_.expires_from_now(milliseconds(dead_mill_seconds_), ec);
                deadline_timer_.async_wait(
                    boost::bind(
                    &AsyncWaitTimer::on_timer, 
                    shared_from_this(),
                    boost::asio::placeholders::error));
            } else {
                //
            }
        }
    }

private:
#if (BOOST_VERSION >= 104900)
    boost::asio::steady_timer deadline_timer_;
#else
	boost::asio::deadline_timer deadline_timer_;
#endif 
	int32_t total_times_;
	int32_t dead_mill_seconds_;
    int32_t invoke_times_;
    WaitFunc wait_func_;
    bool is_cancel_;
    bool immediately_;
};

#endif
