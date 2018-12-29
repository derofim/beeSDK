#include "async_state_machine.h"

namespace bee {

AsyncStateMachine::AsyncStateMachine(IOSPtr ios):ios_(ios), current_state_id_(-1) {

}

AsyncStateMachine::~AsyncStateMachine() {

}

bool AsyncStateMachine::add_state(State::Ptr state, bool activate) {
    bool ret = true;
    do {
        if (state == NULL) {
            ret = false;
            break;
        }

        if (activate && activate_state_ != NULL) {
            ret = false;
            break;
        }

        int32_t id = state->get_id();
        if (state_graph_.find(id) != state_graph_.end()) {
            ret = false;
            break;
        }

        state_graph_[id] = state;
        
        if (activate) {
            activate_state_ = state;
        }
    } while (0);
    return ret;
}

bool AsyncStateMachine::add_transform(int32_t source, int32_t target, const TransformFunction &transform) {
    bool ret = true;
    do {
        if (transform == NULL) {
            ret = false;
            break;
        }

        StateGraph::iterator iter = state_graph_.find(source);
        if (iter == state_graph_.end()) {
            ret = false;
            break;
        }

        State::Ptr source_state = iter->second;
        if (source_state == NULL) {
            ret = false;
            break;
        }

        ret = source_state->add_transform(target,transform);
    } while (0);
    return ret;
}

void AsyncStateMachine::execute_state(State::Ptr state) {
    BeeErrorCode ret = state->execute();
    if (ret != kBeeErrorCode_Success) {
        StateEvent::Ptr ev = state->get_output_event();
        ev->ec1 = ret;
        output(ev);
    }
}

bool AsyncStateMachine::activate_state_machine() {
    bool ret = true;
    do {
        if (ios_ == NULL) {
            ret = false;
            break;
        }

        if (current_state_id_ != -1) {
            ret = false;
            break;
        }

        if (activate_state_ == NULL) {
            ret = false;
            break;
        }

        if (state_graph_.empty()) {
            ret = false;
            break;
        }

        current_state_id_ = activate_state_->get_id();
        ios_->post(boost::bind(&AsyncStateMachine::execute_state, shared_from_this(), activate_state_));
    } while (0);
    return ret;
}

void AsyncStateMachine::uninstall_state_machine() {
    StateGraph::iterator iter = state_graph_.begin();
    for (;iter != state_graph_.end();) {
        State::Ptr state = iter->second;
        state->uninstall();
        state_graph_.erase(iter++);
    }
}

bool AsyncStateMachine::switch_to_state(int32_t next_state_id, const TransformFunction &transform, StateEvent::Ptr last_output_event) {
    bool ret = true;
    do {
        if (transform == NULL || last_output_event == NULL) {
            ret = false;
            break;
        }

        StateGraph::iterator iter = state_graph_.find(next_state_id);
        if (iter == state_graph_.end()) {
            ret = false;
            break;
        }

        State::Ptr next_state = iter->second;
        if (next_state == NULL) {
            ret = false;
            break;
        }

        StateEvent::Ptr input_ev = next_state->get_input_event();
        if (input_ev == NULL) {
            ret = false;
            break;
        }

        ret = transform(last_output_event,input_ev);
        if (!ret) {
            break;
        }

        current_state_id_ = next_state_id;
        ios_->post(boost::bind(&AsyncStateMachine::execute_state, shared_from_this(), next_state));
    } while (0);
    return ret;
}

} // namespace bee
