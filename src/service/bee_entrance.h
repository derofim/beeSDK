#ifndef __BEE_ENTRANCE_H__
#define __BEE_ENTRANCE_H__

#include "utility/common.h"
#include "utility/timer.h"
#include "bee/base/bee.h"
#include "server/ca/ca_client.h"
#include "server/statusd/statusd_client.h"
#include "state/simple_state_machine.h"

#ifdef ENABLE_BREAKPAD
#ifdef ANDROID
#include <jni.h>
#include "breakpad/client/linux/handler/exception_handler.h"
#include "breakpad/client/linux/handler/minidump_descriptor.h"
#endif
#endif

namespace bee {
    
//////////////////////////////////State Machine////////////////////////////////////////
typedef enum BeeSvcInputEvent {
    kBeeSvcInput_DoInit = 0,
    kBeeSvcInput_InitDone,
    kBeeSvcInput_Get_Core,
    kBeeSvcInput_Got_Core,
    kBeeSvcInput_Execute,
    kBeeSvcInput_DoUninit,
    kBeeSvcInput_UninitDone,
    kBeeSvcInput_Count
}BeeSvcInputEvent;

typedef enum BeeSvcState {
    kBeeSvcState_Idle = 0,
    kBeeSvcState_Initializing,
    kBeeSvcState_Initializd,
    kBeeSvcState_Getting_Core,
    kBeeSvcState_Ready,
    kBeeSvcState_Unitializing,
    kBeeSvcState_Count
}BeeSvcState;

#ifdef TEST_CA
const int32_t kLuaUpdateInterval = 5; //5S
#else
const int32_t kLuaUpdateInterval = 300; //300S
#endif

const int32_t kStatusdCheckInterval = 10000; //10S

///////////////////////////////////BeeEntrance///////////////////////////////////////
class IOService;
class BeeSession;
class SessionManager;
class SyncDataPromise;
class BeePromise;
class BeeInitPromise;
class BeeSessionPromise;
class BeeService;

class BeeEntrance 
    : public std::enable_shared_from_this<BeeEntrance>, 
      public SimpleStateMachine, 
      public CAHandler,
      public StatusdHandler {
public:
    typedef std::shared_ptr<BeeEntrance> Ptr;
    ~BeeEntrance();
    static Ptr   instance();
    static void  destroy_instance();
    static bool  is_instance_exist();

public:
    BeeErrorCode start(const BeeSystemParam &param, std::shared_ptr<BeeSink> sink, int32_t timeout, int32_t &ec2);
    BeeErrorCode stop();
    BeeErrorCode sync_open_session(bee_handle &handle, std::vector<BeeCapability> &capability);
    BeeErrorCode async_open_session(BeeOpenCallback cb, void *opaque);
    BeeErrorCode close_session(bee_handle handle);
    BeeErrorCode sync_execute(bee_handle handle, bee_int32_t svc_code, const std::string &cmd, const std::string &args, bee_int32_t timeout);
    BeeErrorCode async_set_root_certificate(const char *certificate);
    const std::string &get_root_certificare() const {return root_certificate_path_;}
    void force_stop();
    void init_ssl();
    void clear_ssl();
    void configure_net_log(int32_t log_level, TimeType time_base);
    void write_net_log(const std::string &log);
    static void BEE_CALLBACK log_callback(const char *log);
    BeeErrorCode reg_svc(std::shared_ptr<BeeService> svc);
    BeeErrorCode unreg_svc(std::shared_ptr<BeeService> svc);
#ifdef ANDROID
    BeeErrorCode set_codec_egl_context(JNIEnv* jni, jobject local_egl_context, jobject remote_egl_context);
    void set_application_context(JNIEnv* jni, jobject context);
#elif defined(WIN32)
    void set_video_encoder_factory(cricket::WebRtcVideoEncoderFactory *factory);
    void set_video_decoder_factory(cricket::WebRtcVideoDecoderFactory *factory);
#endif

protected:
    BeeEntrance();
#ifdef ENABLE_BREAKPAD
    void breakpad_init(const char* path);
    void breakpad_uninit();
#endif
    void init_state_machine();
    void save_param(const BeeSystemParam &param, std::shared_ptr<BeeSink> sink);
    bool load_default_lua();
    bool check_lua_md5(const std::string &md5);
    void handle_ca_data(
        BeeErrorCode ec1, 
        const boost::system::error_code& ec2, 
        const std::string &lua, 
        const std::string &md5, 
        const std::string &gslb_key, 
        uint64_t gslb_ts);
    bool check_ios();
    void reset();
    void do_open_session(std::shared_ptr<BeeSessionPromise> promise);
    void do_close_session(bee_handle handle, std::shared_ptr<BeeSessionPromise> promise);
    void do_execute(bee_handle handle, bee_int32_t svc_code, const std::string &command, const std::string &args, std::shared_ptr<BeeSessionPromise> promise);
    void do_reg_svc(std::shared_ptr<BeeService> svc, std::shared_ptr<BeePromise> promise);
    void do_unreg_svc(std::shared_ptr<BeeService> svc, std::shared_ptr<BeePromise> promise);
    void close_ca_client();
    void do_set_root_certificate(const std::string &certificate) {root_certificate_path_ = certificate;}
    bool start_lua_update_timer(int32_t timeout);
    void stop_lua_update_timer();
    void handle_lua_update_timeout(int32_t timeout);
    bool start_statusd_timer(bool immediately);
    void stop_statusd_timer();
    void handle_statusd_timeout();
    void close_statusd_client();
    void handle_statusd_error(BeeErrorCode ec1, const boost::system::error_code& ec2);
    bool is_statusd_logined();
    void on_log(const char *log);
    void print_version();
    
protected:
    static std::shared_ptr<BeeEntrance> instance_;
    std::shared_ptr<IOService>      io_service_;
    std::shared_ptr<SessionManager> session_mgr_;
    std::shared_ptr<CAClient>       ca_client_;
    std::shared_ptr<StatusdClient>  statusd_client_;
    std::shared_ptr<BeeInitPromise> init_promise_;
#ifdef ENABLE_BREAKPAD
    std::shared_ptr<google_breakpad::ExceptionHandler> breakpad_handler_;
#endif
    std::string lua_;
    std::string lua_name_ = DRM_LUA_NAME;
    std::string lua_md5_;
    std::string gslb_key_;
    uint64_t    gslb_ts_;
    AsyncWaitTimer::Ptr lua_update_timer_;
    AsyncWaitTimer::Ptr statusd_timer_;
    SystemParam sys_param_;
    std::string root_certificate_path_;
    Logger      logger_;
    int32_t     statusd_check_interval_ = kStatusdCheckInterval;
    std::string current_live_session_id_; //Only store last opened session id, could be overridden.
    std::shared_ptr<BeeSink> sink_;
};

} // namespace bee

#endif
