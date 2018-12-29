#ifndef __BEE_PROMISE_H__
#define __BEE_PROMISE_H__

#include "utility/common.h"
#include "utility/timer.h"
#include "utility/iobuffer.h"
#include "bee/base/bee.h"

namespace bee {

////////////////////////////////////BeePromise//////////////////////////////////////
class BeePromise : public std::enable_shared_from_this<BeePromise> {
public:
    typedef std::shared_ptr<BeePromise> Ptr;
    BeePromise();
    virtual ~BeePromise();

public:
    virtual void active(IOSPtr ios, int32_t timeout);
    virtual void honor(BeeErrorCode ec1, bool reschedule_timer = false);
    virtual void give_up();
    virtual BeeErrorCode wait();
    virtual BeeErrorCode wait_for(TimeType ms);

protected:
    virtual void handle_timeout();
    virtual void sync_honor(BeeErrorCode ec1);
    virtual void async_honor(BeeErrorCode ec1){}

public:
    void    set_session_id(int32_t session_id){session_id_ = session_id;}
    int32_t get_session_id(){return session_id_;}

protected:
    int32_t session_id_;
    bool    honored_;
    bool    sync_;
    std::shared_ptr<std::promise<BeeErrorCode> > promise_;
    AsyncWaitTimer::Ptr timer_;
};

////////////////////////////////////BeeInitPromise//////////////////////////////////////
class BeeInitPromise : public BeePromise {
public:
    typedef std::shared_ptr<BeeInitPromise> Ptr;
    BeeInitPromise():ec2_(-1){sync_ = true;}
    virtual ~BeeInitPromise(){}

public:
    void    set_ec2(int32_t ec2){ec2_ = ec2;}
    int32_t get_ec2(){return ec2_;}

protected:
    int32_t ec2_;
};

/////////////////////////////////////BeeAsyncInitPromise/////////////////////////////////////
class BeeAsyncInitPromise : public BeeInitPromise {
public:
    typedef std::shared_ptr<BeeAsyncInitPromise> Ptr;
    BeeAsyncInitPromise();
    virtual ~BeeAsyncInitPromise();
    
public:
    virtual BeeErrorCode wait();
    virtual void async_honor(BeeErrorCode ec1);
    virtual void give_up();
    
public:
    void    set_callback(BeeInitCallback callback){callback_ = callback;}
    void    set_opaque(void *opaque){opaque_ = opaque;}

protected:
    BeeInitCallback callback_;
    void    *opaque_;
};

///////////////////////////////////BeeSessionPromise///////////////////////////////////////
class BeeSessionPromise : public BeePromise {
public:
    typedef std::shared_ptr<BeeSessionPromise> Ptr;
    BeeSessionPromise():session_id_(-1){sync_ = true;}
    virtual ~BeeSessionPromise(){}

public:
    void    set_session_id(int32_t session_id){session_id_ = session_id;}
    int32_t get_session_id(){return session_id_;}

public:
    std::vector<BeeCapability> capability;

protected:
    int32_t session_id_;
};

/////////////////////////////////////BeeAsyncOpenPromise/////////////////////////////////////
class BeeAsyncOpenPromise : public BeeSessionPromise {
public:
    typedef std::shared_ptr<BeeAsyncOpenPromise> Ptr;
    BeeAsyncOpenPromise();
    virtual ~BeeAsyncOpenPromise();

public:
    virtual BeeErrorCode wait();
    virtual void async_honor(BeeErrorCode ec1);
    virtual void give_up();

public:
    void    set_callback(BeeOpenCallback callback){callback_ = callback;}
    void    set_opaque(void *opaque){opaque_ = opaque;}

protected:
    BeeOpenCallback callback_;
    void    *opaque_;
};

/////////////////////////////////////BeePlayPromise/////////////////////////////////////
class BeePlayPromise : public BeeSessionPromise {
public:
    typedef std::shared_ptr<BeePlayPromise> Ptr;
    BeePlayPromise():ec2_(0){}
    virtual ~BeePlayPromise(){}

public:
    void    set_ec2(int32_t ec2){ec2_ = ec2;}
    int32_t get_ec2(){return ec2_;}
    void    set_message(const std::string &message){message_ = message;}
    std::string get_message(){return message_;}

protected:
    int32_t ec2_;
    std::string message_;
};

///////////////////////////////////BeeReadPromise///////////////////////////////////////
class BeeReadPromise : public BeeSessionPromise {
public:
    typedef std::shared_ptr<BeeReadPromise> Ptr;
    BeeReadPromise():read_len_(0), seq_(0), eof_(false) {}
    virtual ~BeeReadPromise(){}

public:
    virtual void     set_read_len(uint32_t len) {read_len_ = len;}
    virtual uint32_t get_read_len() {return read_len_;}
    virtual void     set_eof(bool eof) {eof_ = eof;}
    virtual bool     get_eof() {return eof_;}
    virtual void     set_seq(uint32_t seq) {seq_ = seq;}
    virtual uint32_t get_seq() {return seq_;}

protected:
    uint32_t read_len_;
    uint32_t seq_;
    bool     eof_;
};

} // namespace bee

#endif
