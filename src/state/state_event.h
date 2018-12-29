#ifndef __STATE_EVENT_H__
#define __STATE_EVENT_H__

#include "utility/common.h"
#include "utility/iobuffer.h"
#include "bee/base/bee_define.h"

namespace bee {

struct StateEvent {
    typedef std::shared_ptr<StateEvent> Ptr;    
    BeeErrorCode ec1;
    boost::system::error_code ec2;
    bool setup;
    IOBuffer data;

    StateEvent() {
        ec1 = kBeeErrorCode_Success;
        setup = false;
    }

    virtual ~StateEvent() {

    }
};

} // namespace bee

#endif
