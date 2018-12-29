#ifndef __STATE_MACHINE_H__
#define __STATE_MACHINE_H__

#include "utility/common.h"
#include "state.h"
#include "state_event.h"

namespace bee {

///////////////////////////////////////Macros///////////////////////////////////
#define BEGIN_STATE_MACHINE(class) \
bool class::setup_state_machine() { \
    State::Ptr state; \
    StateRouter router; \
    bool completed = false; \
    class::Ptr sm = shared_from_base<class>(); \
    do { 

#define BEGIN_STATE(id, class, fun, activate) \
    state.reset(new class(id)); \
    router = boost::bind(fun, sm, _1); \
    if (!state->setup(router, sm, activate)) break;

#define SWITCH_TO(id, fun) \
    if (!state->add_transform(id, boost::bind(fun, sm, _1, _2))) break;

#define END_STATE

#define END_STATE_MACHINE \
    completed = !state_graph_.empty(); \
    } while (0); \
    return completed; }

#define DECLARE_STATE_MACHINE bool setup_state_machine();
const int32_t kInvalidState  = -1;    
const int32_t kFinishedState = -2;

////////////////////////////////////AsyncStateMachine//////////////////////////////////////
class AsyncStateMachine : public std::enable_shared_from_this<AsyncStateMachine> {
public:
    typedef std::unordered_map<int32_t,State::Ptr> StateGraph;
    typedef std::shared_ptr<AsyncStateMachine> Ptr;
    AsyncStateMachine(IOSPtr ios);
    virtual ~AsyncStateMachine();
    IOSPtr ios(){return ios_;}

protected:
    virtual bool setup_state_machine() = 0;
    void uninstall_state_machine();
    bool activate_state_machine();
    bool switch_to_state(int32_t next_state_id, const TransformFunction &transform, StateEvent::Ptr last_output_event);
    bool add_state(State::Ptr state, bool activate = false);
    bool add_transform(int32_t source, int32_t target, const TransformFunction &transform);
    void execute_state(State::Ptr state);
    int32_t get_current_state(){return current_state_id_;}
    virtual void output(StateEvent::Ptr ev){}    

    template <typename Derived>
    std::shared_ptr<Derived> shared_from_base() {
        return std::static_pointer_cast<Derived>(shared_from_this());
    }

protected:
    IOSPtr ios_;
    StateGraph state_graph_;
    State::Ptr activate_state_;
    int32_t current_state_id_;

    //////////////////////////////////Friends////////////////////////////////////////
    friend class State;
};

} // namespace bee

#endif
