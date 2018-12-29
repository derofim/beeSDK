#ifndef __IO_SERVICE_H__
#define __IO_SERVICE_H__

#include "base/common.h"

class IOService {
public:
    typedef std::shared_ptr<IOService> Ptr;
    IOService();
    ~IOService();

public:
    bool start();
    bool stop();
    IOSPtr ios(){return ios_;}
    bool is_running(){return running_;}

protected:
    bool wait();

protected:
    IOSPtr ios_;
    IOSWorkPtr work_;
    ThreadPtr thread_;
    bool running_;
};

#endif
