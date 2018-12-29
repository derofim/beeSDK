#include "state.h"
#include "state_event.h"
#include "async_state_machine.h"

namespace bee {

//////////////////////////////////State////////////////////////////////////////
State::State(int32_t id):
    id_(id),
    type_(-1) {

}

State::~State() {
}

bool State::add_transform(int32_t target,const TransformFunction &transform) {
    bool ret = true;
    do {
        if (transform == NULL) {
            ret = false;
            break;
        }

        if (transform_table_.find(target) != transform_table_.end()) {
            ret = false;
            break;
        }

        transform_table_[target] = transform;
    } while (0);
    return ret;
}

bool State::switch_to_state(int32_t next_state_id,const TransformFunction &transform) {
    bool ret = true;
    do {
        std::shared_ptr<AsyncStateMachine> state_machine = state_machine_.lock();
        if (state_machine == NULL) {
            ret = false;
            break;
        }

        ret = state_machine->switch_to_state(next_state_id,transform,output_event_);
    } while (0);
    return ret;
}

bool State::switch_to_next() {
    bool ret = true;
    do {
        if (output_event_ == NULL || !output_event_->setup) {
            ret = false;
            break;
        }

        if (state_router_ == NULL) {
            output_event_->ec1 = kBeeErrorCode_Error_State_Machine;
            ret = false;
            break;
        }

        int32_t next_state_id = state_router_(output_event_);
        if (next_state_id == kInvalidState) {
            if (output_event_->ec1 == kBeeErrorCode_Success) {
                output_event_->ec1 = kBeeErrorCode_State_Machine_Interrupted;
            }
            ret = false;
            break;
        } else if (next_state_id == kFinishedState) {
            output_event_->ec1 = kBeeErrorCode_State_Machine_Finished;
            ret = false;
            break;
        }

        TransformTable::iterator iter = transform_table_.find(next_state_id);
        if (iter == transform_table_.end()) {
            output_event_->ec1 = kBeeErrorCode_Error_State_Machine;
            ret = false;
            break;
        }

        ret = switch_to_state(next_state_id, iter->second);
    } while (0);

    if (!ret) {
        output(output_event_);
    }    
    return ret;
}

bool State::done() {
    return switch_to_next();
}

bool State::done(BeeErrorCode ec1, const boost::system::error_code &ec2) {
    bool ret = true;
    do {
        StateEvent::Ptr output_ev = get_output_event();
        if (output_ev != NULL) {
            output_ev->ec1 = ec1;
            output_ev->ec2 = ec2;
        }

        ret = done();
    } while (0);

    return ret;
}

void State::output(StateEvent::Ptr ev) {
    AsyncStateMachine::Ptr fsm = state_machine_.lock();
    if (fsm) {
        fsm->output(ev);
    }
}

bool State::setup(const StateRouter &router,std::shared_ptr<AsyncStateMachine> state_machine,bool activate) {
    bool ret = true;
    do {
        if (router == NULL || state_machine == NULL) {
            ret = false;
            break;
        }

        attach_router(router);
        attach_state_machine(state_machine);
        create_input_event();
        create_output_event();

        state_machine->add_state(shared_from_this(), activate);
    } while (0);
    return ret;
}

void State::uninstall() {
    state_router_ = NULL;
    transform_table_.clear();
}

} // namespace bee
