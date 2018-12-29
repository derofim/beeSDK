#ifndef __STATE_H__
#define __STATE_H__

#include "utility/common.h"
#include "state_event.h"

namespace bee {

class AsyncStateMachine;
typedef std::function<int32_t(StateEvent::Ptr)> StateRouter;
typedef std::function<bool(StateEvent::Ptr,StateEvent::Ptr)> TransformFunction;
typedef std::unordered_map<int32_t,TransformFunction> TransformTable;

///////////////////////////////////Macros///////////////////////////////////////
#define STATE_MACHINE_INPUT(class)  virtual void create_input_event() {input_event_.reset(new class);}
#define STATE_MACHINE_OUTPUT(class) virtual void create_output_event(){output_event_.reset(new class);}

///////////////////////////////////State///////////////////////////////////////
class State : public std::enable_shared_from_this<State> {
public:
    typedef std::shared_ptr<State> Ptr;
    State(int32_t id);
    virtual ~State();

public:
    STATE_MACHINE_INPUT(StateEvent)
    STATE_MACHINE_OUTPUT(StateEvent)
    virtual BeeErrorCode execute() = 0;

public:
    bool add_transform(int32_t target,const TransformFunction &transform);
    StateEvent::Ptr get_input_event(){return input_event_;}
    StateEvent::Ptr get_output_event(){return output_event_;}
    int32_t get_id(){return id_;}
    int32_t get_type(){return type_;}
    bool setup(const StateRouter &router,std::shared_ptr<AsyncStateMachine> state_machine,bool activate);
    void uninstall();

protected:
    bool switch_to_state(int32_t next_state_id,const TransformFunction &transform);
    bool switch_to_next();
    bool done();
    bool done(BeeErrorCode ec1, const boost::system::error_code &ec2);
    void output(StateEvent::Ptr ev);
    void attach_router(const StateRouter &router){state_router_ = router;}
    void attach_state_machine(std::shared_ptr<AsyncStateMachine> state_machine){state_machine_ = state_machine;}
    bool check_event(){return input_event_ != NULL && input_event_->setup && output_event_ != NULL;}

    template <typename Derived>
    std::shared_ptr<Derived> shared_from_base() {
        return std::static_pointer_cast<Derived>(shared_from_this());
    }

protected:
    int32_t id_;
    int32_t type_;
    StateRouter state_router_;
    TransformTable transform_table_;
    std::weak_ptr<AsyncStateMachine> state_machine_;
    StateEvent::Ptr input_event_;
    StateEvent::Ptr output_event_;
};

} // namespace bee

#endif
