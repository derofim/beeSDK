#include "simple_state_machine.h"

namespace bee {

SimpleStateMachine::SimpleStateMachine(int32_t state_count, int32_t input_count):
    state_count_(state_count),
    input_count_(input_count),
    current_state_(0),
    last_state_(0) {

    if (state_count > 0 && input_count > 0) {
        state_machine_.resize(state_count_);
        for (int32_t i = 0;i < state_count_;++i) {
            state_machine_[i].resize(input_count);
        }
    }
}

SimpleStateMachine::~SimpleStateMachine() {
}

bool SimpleStateMachine::switch_state(int32_t ev) {
    bool ret = true;
    do {
        if (ev < 0 || ev >= input_count_) {
            ret = false;
            break;
        }

        int32_t next_state = state_machine_[current_state_][ev];
        if (next_state == kInvalidSimpleState) {
            ret = false;
            break;
        }

        last_state_    = current_state_;
        current_state_ = next_state;
    } while (0);
    return ret;
}

void SimpleStateMachine::rollback_state() {
    current_state_ = last_state_;
}

void SimpleStateMachine::set_state(int32_t state) {
    if (state >= 0 && state < state_count_) {
        current_state_ = state;
        last_state_ = state;
    }
}

void SimpleStateMachine::reset_state() {
    current_state_ = 0;
    last_state_ = 0;
}

} // namespace bee
