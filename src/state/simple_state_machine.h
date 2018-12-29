#ifndef __SIMPLE_STATE_MACHINE_H__
#define __SIMPLE_STATE_MACHINE_H__

#include "utility/common.h"

namespace bee {

const int32_t kInvalidSimpleState = -1;
typedef std::vector<int32_t> SimpleStateChunk;
typedef std::vector<SimpleStateChunk> SimpleStateTable;

class SimpleStateMachine {
public:
    SimpleStateMachine(int32_t state_count, int32_t input_count);
    virtual ~SimpleStateMachine();

public:
    virtual void init_state_machine() = 0;
    bool switch_state(int32_t ev);
    void rollback_state();
    void set_state(int32_t state);
    void reset_state();

protected:
    SimpleStateTable state_machine_;
    int32_t state_count_;
    int32_t input_count_;
    int32_t current_state_;
    int32_t last_state_;
};

} // namespace bee

#endif
