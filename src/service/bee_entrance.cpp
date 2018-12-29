#include "bee_entrance.h"
#include "bee_promise.h"
#include "bee/base/bee_version.h"
#include "bee/base/bee_sink.h"
#include "bee/base/bee_service.h"
#include "network/io_service.h"
#include "session/session_manager.h"
#include "log/logger.h"
#include "lua/module/lua_webrtc_service.h"

#ifdef IOS
#include "webrtc/rtc_base/ssladapter.h"
#elif defined(WIN32)
#include "webrtc/rtc_base/ssladapter.h"
#else
#include "webrtc/base/ssladapter.h"
#endif

#ifdef IOS
#include "platform/ios/ios_adapter.h"
#endif

#ifdef ENABLE_BREAKPAD
bool DumpCallback(const google_breakpad::MinidumpDescriptor& descriptor,
                  void* context,
                  bool succeeded) {
    //Do nothing.
    return succeeded;
}
#endif

namespace bee {

std::shared_ptr<BeeEntrance> BeeEntrance::instance_;

//////////////////////////////////SessionManager////////////////////////////////////////
BeeEntrance::BeeEntrance()
    : SimpleStateMachine(kBeeSvcState_Count, kBeeSvcInput_Count), 
      gslb_ts_(0),
      logger_("BeeEntrance") {

}

BeeEntrance::~BeeEntrance() {

}

#ifdef ENABLE_BREAKPAD
void BeeEntrance::breakpad_init(const char* path){
    if (NULL == breakpad_handler_){
        std::string dump_path;
#ifdef ANDROID
        if (path == NULL || (path != NULL && (strlen(path) == 0))) {
            dump_path = "/sdcard";
        } else {
            dump_path = path;
        }
#else
        return;
#endif
        google_breakpad::MinidumpDescriptor descriptor(dump_path);
        breakpad_handler_.reset(new google_breakpad::ExceptionHandler(descriptor, NULL, DumpCallback, NULL, true, -1));
    }
}

void BeeEntrance::breakpad_uninit(){
    if (NULL != breakpad_handler_){
        breakpad_handler_.reset();
    }
}
#endif

BeeEntrance::Ptr BeeEntrance::instance() {
    if (instance_ == NULL) {
        if (instance_ == NULL) {
            instance_.reset(new BeeEntrance);
            instance_->init_state_machine();
        }
    }
    return instance_;
}

void BeeEntrance::destroy_instance() {
    if (instance_ != NULL) {
        if (instance_ != NULL) {
            instance_.reset();
        }
    }
}

bool BeeEntrance::is_instance_exist() {
    return instance_ != NULL;
}

BeeErrorCode BeeEntrance::start(const BeeSystemParam &param, std::shared_ptr<BeeSink> sink, int32_t timeout, int32_t &ec2) {
    BeeErrorCode ret = kBeeErrorCode_Success;
    do {
#ifdef ENABLE_BREAKPAD
        breakpad_init(param.log_path);
#endif

        //Initialize ssl first.
        init_ssl();

        save_param(param, sink);

        //Open log.
        LogManager::Open(
            param.log_path == NULL ? "" : param.log_path,
            param.log_level,
            param.log_max_line,
            param.log_volume_count,
            param.log_volume_size,
            "bee",
            (sink_ != NULL)?BeeEntrance::log_callback:NULL);

        print_version();

        if (!switch_state(kBeeSvcInput_DoInit)) {
            ret = kBeeErrorCode_Invalid_State;
            break;
        }

        //Allocate sessions.
        if (session_mgr_ == NULL) {
            session_mgr_.reset(new SessionManager);
            session_mgr_->init(sys_param_.session_count);
        }

        //Start io service,thread.
        if (io_service_ == NULL) {
            io_service_.reset(new IOService);
            io_service_->start();
        }

        //Start statusd timer.
        if (sys_param_.enable_statusd) {
            start_statusd_timer(true);
        }

        if (!switch_state(kBeeSvcInput_InitDone)) {
            ret = kBeeErrorCode_Invalid_State;
            break;
        }

        BeeInitPromise::Ptr promise;
        promise.reset(new BeeInitPromise);

        IOSPtr ios = io_service_->ios();
        init_promise_ = promise;
        init_promise_->active(ios, timeout);
#define FORCE_LOCAL_LUA
#ifndef FORCE_LOCAL_LUA
        if (!start_lua_update_timer(timeout)) {
            ret = kBeeErrorCode_Invalid_Param;
            break;
        }
#else
        if (!load_default_lua()) {
            ret = kBeeErrorCode_Engine_Script_Error;
            break;
        }
#endif

        if (!switch_state(kBeeSvcInput_Get_Core)) {
            ret = kBeeErrorCode_Invalid_State;
            break;
        }

#ifndef FORCE_LOCAL_LUA
        ret = promise->wait();
        ec2 = promise->get_ec2();
#else
        if (!switch_state(kBeeSvcInput_Got_Core)) {
            ret = kBeeErrorCode_Invalid_State;
            break;
        }
#endif
        LuaWebrtcService::init_service(
            param.enable_video_encoder_hw_acceleration, 
            param.enable_video_decoder_hw_acceleration,
            param.enable_custom_audio_source);
    } while (0);

    if (ret != kBeeErrorCode_Success) {
        if (init_promise_ != NULL) {
            init_promise_->honor(ret);
            init_promise_.reset();
        }
        stop_lua_update_timer();
    }

    return ret;
}

BeeErrorCode BeeEntrance::stop() {
    BeeErrorCode ret = kBeeErrorCode_Success;
    do {
        if (!switch_state(kBeeSvcInput_DoUninit)) {
            ret = kBeeErrorCode_Invalid_State;
            break;
        }

        force_stop();

#ifdef ENABLE_BREAKPAD
        breakpad_uninit();
#endif
        clear_ssl();
    } while (0);
    return ret;
}

BeeErrorCode BeeEntrance::sync_open_session(bee_handle &handle, std::vector<BeeCapability> &capability) {
    BeeErrorCode ec = kBeeErrorCode_Success;
    do {
        if (!check_ios()) {
            ec = kBeeErrorCode_Service_Not_Started;
            break;
        }

        BeeSessionPromise::Ptr promise(new BeeSessionPromise);
        promise->active(io_service_->ios(), -1);
        io_service_->ios()->post(boost::bind(&BeeEntrance::do_open_session, shared_from_this(), promise));

        ec = promise->wait();
        handle = promise->get_session_id();
        capability = std::move(promise->capability);
    } while (0);
    return ec;
}

BeeErrorCode BeeEntrance::async_open_session(BeeOpenCallback cb, void *opaque) {
    BeeErrorCode ret = kBeeErrorCode_Success;
    do {
        if (cb == NULL) {
            ret = kBeeErrorCode_Invalid_Param;
            break;
        }

        if (!check_ios()) {
            ret = kBeeErrorCode_Service_Not_Started;
            break;
        }

        std::shared_ptr<BeeAsyncOpenPromise> promise(new BeeAsyncOpenPromise);
        promise->active(io_service_->ios(), -1);
        promise->set_callback(cb);
        promise->set_opaque(opaque);
        io_service_->ios()->post(boost::bind(&BeeEntrance::do_open_session, shared_from_this(), promise));
    } while (0);
    return ret;
}

BeeErrorCode BeeEntrance::close_session(bee_handle handle) {
    BeeErrorCode ret = kBeeErrorCode_Success;
    do {
        if (!check_ios()) {
            ret = kBeeErrorCode_Service_Not_Started;
            break;
        }

        BeeSessionPromise::Ptr promise(new BeeSessionPromise);
        promise->active(io_service_->ios(), -1);
        io_service_->ios()->post(boost::bind(&BeeEntrance::do_close_session, shared_from_this(), handle, promise));
        ret = promise->wait();
    } while (0);
    return ret;
}

BeeErrorCode BeeEntrance::sync_execute(bee_handle handle, bee_int32_t svc_code, const std::string &cmd, const std::string &args, bee_int32_t timeout) {
    BeeErrorCode ret = kBeeErrorCode_Success;
    do {
        if (!check_ios()) {
            ret = kBeeErrorCode_Service_Not_Started;
            break;
        }

        if (cmd.empty() || args.empty()) {
            ret = kBeeErrorCode_Invalid_Param;
            break;
        }

        BeeSessionPromise::Ptr promise(new BeeSessionPromise);
        promise->set_session_id(handle);
        promise->active(io_service_->ios(), timeout);
        io_service_->ios()->post(boost::bind(&BeeEntrance::do_execute, shared_from_this(), handle, svc_code, cmd, args, promise));
        ret = promise->wait();
    } while (0);
    return ret;
}

BeeErrorCode BeeEntrance::async_set_root_certificate(const char *certificate) {
    BeeErrorCode ret = kBeeErrorCode_Success;
    do {
        if (!check_ios()) {
            ret = kBeeErrorCode_Service_Not_Started;
            break;
        }

        if (certificate == NULL) {
            ret = kBeeErrorCode_Invalid_Param;
            break;
        }

        io_service_->ios()->post(boost::bind(&BeeEntrance::do_set_root_certificate, shared_from_this(), std::string(certificate)));
    } while (0);
    return ret;
}

#ifdef ANDROID
BeeErrorCode BeeEntrance::set_codec_egl_context(JNIEnv* jni, jobject local_egl_context, jobject remote_egl_context) {
    BeeErrorCode ret = kBeeErrorCode_Success;
    do {
        if (!check_ios()) {
            ret = kBeeErrorCode_Service_Not_Started;
            break;
        }

        LuaWebrtcService::set_codec_egl_context(jni, local_egl_context, remote_egl_context);
    } while (0);
    return ret;
}

void BeeEntrance::set_application_context(JNIEnv* jni, jobject context) {
    LuaWebrtcService::set_application_context(jni, context);
}
#elif defined(WIN32)
void BeeEntrance::set_video_encoder_factory(cricket::WebRtcVideoEncoderFactory *factory) {
    LuaWebrtcService::set_video_encoder_facotry(factory);
}

void BeeEntrance::set_video_decoder_factory(cricket::WebRtcVideoDecoderFactory *factory) {
    LuaWebrtcService::set_video_decoder_facotry(factory);
}
#endif

void BeeEntrance::init_state_machine() {
    for (int32_t i = 0;i < kBeeSvcState_Count;++i) {
        for (int32_t j = 0;j< kBeeSvcInput_Count;++j) {
            state_machine_[i][j] = kInvalidState;
        }
    }

    state_machine_[kBeeSvcState_Idle][kBeeSvcInput_DoInit]               = kBeeSvcState_Initializing;
    state_machine_[kBeeSvcState_Initializing][kBeeSvcInput_InitDone]     = kBeeSvcState_Initializd;
    state_machine_[kBeeSvcState_Initializing][kBeeSvcInput_DoUninit]     = kBeeSvcState_Unitializing;
    state_machine_[kBeeSvcState_Initializd][kBeeSvcInput_Get_Core]       = kBeeSvcState_Getting_Core;
    state_machine_[kBeeSvcState_Initializd][kBeeSvcInput_DoUninit]       = kBeeSvcState_Unitializing;
    state_machine_[kBeeSvcState_Getting_Core][kBeeSvcInput_Got_Core]     = kBeeSvcState_Ready;
    state_machine_[kBeeSvcState_Getting_Core][kBeeSvcInput_DoUninit]     = kBeeSvcState_Unitializing;
    state_machine_[kBeeSvcState_Ready][kBeeSvcInput_Execute]             = kBeeSvcState_Ready;
    state_machine_[kBeeSvcState_Ready][kBeeSvcInput_Got_Core]            = kBeeSvcState_Ready;
    state_machine_[kBeeSvcState_Ready][kBeeSvcInput_DoUninit]            = kBeeSvcState_Unitializing;
    state_machine_[kBeeSvcState_Unitializing][kBeeSvcInput_UninitDone]   = kBeeSvcState_Idle;
}

void BeeEntrance::save_param(const BeeSystemParam &param, std::shared_ptr<BeeSink> sink) {
    sys_param_.platform_type = param.platform_type;
    sys_param_.net_type = param.net_type;
    sys_param_.app_name = param.app_name == NULL ? "" : std::string(param.app_name);
    sys_param_.app_version = param.app_version == NULL ? "" : std::string(param.app_version);
    sys_param_.system_info = param.system_info == NULL ? "" : std::string(param.system_info);
    sys_param_.machine_code = param.machine_code == NULL ? "" : std::string(param.machine_code);
    sys_param_.log_level = param.log_level;
    sys_param_.log_max_line = param.log_max_line;
    sys_param_.log_volume_count = param.log_volume_count;
    sys_param_.log_volume_size = param.log_volume_size;
    sys_param_.log_path = param.log_path == NULL ? "" : std::string(param.log_path);
    sys_param_.session_count = param.session_count;
    sys_param_.enable_statusd = param.enable_statusd;
    sink_ = sink;
}

bool BeeEntrance::load_default_lua() {
#ifdef ANDROID
    std::string path = "/sdcard/new_bee.lua";
#elif defined(IOS)
    std::string path = IOSAdapter::get_defualt_lua_file_path();
#else
    std::string path = "new_bee.lua";
#endif
    FILE *fp = fopen(path.c_str(), "rb");
    if (fp == NULL) {
        return false;
    }

    fseek(fp, 0, SEEK_END);
    int32_t size = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    lua_.resize(size);
    fread((void*)lua_.data(), size, 1, fp);
    fclose(fp);
    logger_.Info("Local lua loaded.\n");
    return true;
}

bool BeeEntrance::check_lua_md5(const std::string &md5) {
    bool ret = false;
    do {
        if (md5.empty()) {
            break;
        }

        ret = (md5 == lua_md5_);
        if (!ret) {
            break;
        }

        switch_state(kBeeSvcInput_Got_Core);
        if (init_promise_ != NULL) {
            init_promise_->set_ec2(0);
            init_promise_->honor(kBeeErrorCode_Success);
            init_promise_.reset();
        }

        io_service_->ios()->post(boost::bind(&BeeEntrance::close_ca_client, shared_from_this()));
    } while (0);
    return ret;
}

void BeeEntrance::handle_ca_data(
    BeeErrorCode ec1, 
    const boost::system::error_code& ec2, 
    const std::string &lua, 
    const std::string &md5, 
    const std::string &gslb_key, 
    uint64_t gslb_ts) {
    BeeErrorCode ret1 = ec1;
    int32_t ret2 = ec2.value();
    if (ec1 == kBeeErrorCode_Success && !ec2) {
        switch_state(kBeeSvcInput_Got_Core);
        lua_ = lua;
        lua_name_ = DRM_LUA_NAME;
        lua_md5_ = md5;
        gslb_key_ = gslb_key;
        gslb_ts_ = gslb_ts;
    } else if (lua_.empty()) {
        if (load_default_lua()) {
            switch_state(kBeeSvcInput_Got_Core);
            lua_name_ = LOCAL_DRM_LUA_NAME;
            ret1 = kBeeErrorCode_Success;
            ret2 = 0;
            logger_.Warn("Try downloading lua script from CA failed, will use local default, may not be compatible!!!\n");
        } else {
            reset();
            stop_lua_update_timer();
        }
    }

    if (init_promise_ != NULL) {
        init_promise_->set_ec2(ret2);
        init_promise_->honor(ret1);
        init_promise_.reset();
    }

    io_service_->ios()->post(boost::bind(&BeeEntrance::close_ca_client, shared_from_this()));
}

bool BeeEntrance::check_ios() {
    return io_service_ != NULL && io_service_->ios() != NULL;
}

void BeeEntrance::reset() {
    reset_state();
}

void BeeEntrance::do_open_session(std::shared_ptr<BeeSessionPromise> promise) {
    BeeErrorCode ret = kBeeErrorCode_Success;
    do {
        if (promise == NULL) {
            break;
        }

        if (!switch_state(kBeeSvcInput_Execute)) {
            ret = kBeeErrorCode_Invalid_State;
            break;
        }

        if (session_mgr_ == NULL) {
            ret = kBeeErrorCode_Null_Pointer;
            break;
        }

        std::vector<BeeCapability> capability;
        int32_t session_id = session_mgr_->open_idle_session(io_service_->ios(), ret, sys_param_, DRM_LUA_NAME, lua_, current_live_session_id_, capability);
        if (session_id == -1) {
            break;
        }

        promise->set_session_id(session_id);
        promise->capability = std::move(capability);

        if (statusd_client_ != NULL) {
            statusd_client_->set_sid(current_live_session_id_, true);
        }
    } while (0);
    
    if (promise != NULL) {
        promise->honor(ret);
    }
}

void BeeEntrance::do_close_session(bee_handle handle, std::shared_ptr<BeeSessionPromise> promise) {
    BeeErrorCode ret = kBeeErrorCode_Success;
    do {
        if (!switch_state(kBeeSvcInput_Execute)) {
            ret = kBeeErrorCode_Invalid_State;
            break;
        }

        if (session_mgr_ == NULL) {
            ret = kBeeErrorCode_Null_Pointer;
            break;
        }

        session_mgr_->close_busy_session(handle);
        
        current_live_session_id_.clear();
        if (statusd_client_ != NULL) {
            statusd_client_->set_sid(current_live_session_id_, false);
        }
    } while (0);

    if (promise != NULL) {
        promise->honor(ret);
    }
}

void BeeEntrance::do_execute(bee_handle handle, bee_int32_t svc_code, const std::string &command, const std::string &args, std::shared_ptr<BeeSessionPromise> promise) {
    BeeErrorCode ret = kBeeErrorCode_Success;
    do {
        if (promise == NULL) {
            break;
        }

        if (!switch_state(kBeeSvcInput_Execute)) {
            ret = kBeeErrorCode_Invalid_State;
            break;
        }

        if (session_mgr_ == NULL) {
            ret = kBeeErrorCode_Null_Pointer;
            break;
        }

        BeeSession::Ptr session = session_mgr_->get_busy_session(handle);
        if (session == NULL) {
            ret = kBeeErrorCode_Invalid_Session;
            break;
        }

        ret = session->execute(svc_code, command, args);
    } while (0);

    if (promise != NULL) {
        promise->honor(ret);
    }
}

void BeeEntrance::do_reg_svc(std::shared_ptr<BeeService> svc, std::shared_ptr<BeePromise> promise) {
    BeeErrorCode ret = kBeeErrorCode_Success;
    do {
        if (svc == NULL) {
            ret = kBeeErrorCode_Invalid_Param;
            break;
        }

        BeeSession::Ptr session = session_mgr_->get_busy_session(svc->get_handle());
        if (session == NULL) {
            ret = kBeeErrorCode_Invalid_Session;
            break;
        }

        ret = session->reg_svc(svc);
    } while (0);

    if (promise != NULL) {
        promise->honor(ret);
    }
}

void BeeEntrance::do_unreg_svc(std::shared_ptr<BeeService> svc, std::shared_ptr<BeePromise> promise) {
    BeeErrorCode ret = kBeeErrorCode_Success;
    do {
        if (svc == NULL) {
            ret = kBeeErrorCode_Invalid_Param;
            break;
        }

        BeeSession::Ptr session = session_mgr_->get_busy_session(svc->get_handle());
        if (session == NULL) {
            ret = kBeeErrorCode_Invalid_Session;
            break;
        }

        ret = session->unreg_svc(svc);
    } while (0);

    if (promise != NULL) {
        promise->honor(ret);
    }
}

void BeeEntrance::close_ca_client() {
    if (ca_client_ != NULL) {
        ca_client_->stop();
        ca_client_.reset();
    }
}

bool BeeEntrance::start_lua_update_timer(int32_t timeout) {
    bool started = false;
    if (io_service_ != NULL && io_service_->ios() != NULL) {
        lua_update_timer_ = AsyncWaitTimer::create(*(io_service_->ios()));
        lua_update_timer_->set_wait_seconds(kLuaUpdateInterval);
        lua_update_timer_->set_wait_times(-1);
        lua_update_timer_->set_immediately(true); //Update right now.
        lua_update_timer_->async_wait(boost::bind(&BeeEntrance::handle_lua_update_timeout, shared_from_this(), timeout));
        started = true;
    }
    logger_.Info("Lua update timer start %s\n", started ? "success" : "failed");
    return started;
}

void BeeEntrance::stop_lua_update_timer() {
    if (lua_update_timer_ != NULL) {
        lua_update_timer_->cancel();
        lua_update_timer_.reset();
    }
}

void BeeEntrance::handle_lua_update_timeout(int32_t timeout) {
    if (io_service_ != NULL && io_service_->ios() != NULL) {
        ca_client_.reset(new CAClient(io_service_->ios()));
        ca_client_->async_request_core(kCAHost, kCAService, timeout, shared_from_this());
    }
}

bool BeeEntrance::start_statusd_timer(bool immediately) {
    bool started = false;

    //Stop old timer first.
    if (statusd_timer_ != NULL) {
        statusd_timer_->cancel();
        statusd_timer_.reset();
    }

    //Lua may modify this, so double check here.
    if (!sys_param_.enable_statusd) {
        return started;
    }

    //Start timer, if called from start, handle immediately, or will delay for several second.
    if (io_service_ != NULL && io_service_->ios() != NULL) {
        statusd_timer_ = AsyncWaitTimer::create(*(io_service_->ios()));
        statusd_timer_->set_wait_seconds(statusd_check_interval_);
        statusd_timer_->set_wait_times(-1);  //Always reconnect if not login.
        statusd_timer_->set_immediately(immediately);
        statusd_timer_->async_wait(boost::bind(&BeeEntrance::handle_statusd_timeout, shared_from_this()));
        started = true;
    }

    logger_.Info("Statusd timer start %s\n", started ? "success" : "failed");
    return started;
}

void BeeEntrance::stop_statusd_timer() {
    if (statusd_timer_ != NULL) {
        statusd_timer_->cancel();
        statusd_timer_.reset();
    }
}

void BeeEntrance::handle_statusd_timeout() {
    do {
        if (io_service_ == NULL) {
            break;
        }

        if (!sys_param_.enable_statusd) {
            break;
        }

        //If statusd server is already logined.
        if (is_statusd_logined()) {
            break;
        }

        //If statusd server not logined, reconnect it.
        if (statusd_client_ != NULL) {
            statusd_client_->stop();
            statusd_client_.reset();
        }

        //Connect now.
        statusd_client_.reset(new StatusdClient(io_service_->ios()));
        statusd_client_->connect(sys_param_.machine_code, shared_from_this());
    } while (0);
}

void BeeEntrance::close_statusd_client() {
    if (statusd_client_ != NULL) {
        statusd_client_->stop();
        statusd_client_.reset();
    }
}

void BeeEntrance::handle_statusd_error(BeeErrorCode ec1, const boost::system::error_code& ec2) {
    if (ec1 != kBeeErrorCode_Success || ec2) {
        logger_.Error("Statusd error ec1:%d ec2:%d %s\n", ec1, ec2.value(), ec2.message().c_str());
    }
}

bool BeeEntrance::is_statusd_logined() {
    return statusd_client_ != NULL && statusd_client_->is_logined();
}

void BeeEntrance::force_stop() {
    LuaWebrtcService::uninit_service();

    stop_lua_update_timer();
    stop_statusd_timer();

    if (session_mgr_ != NULL) {
        session_mgr_->uninit();
        session_mgr_.reset();
    }

    if (io_service_ != NULL) {
        io_service_->stop();
        io_service_.reset();
    }

    close_ca_client();
    close_statusd_client();
    cleanup_crypto_data(); //Must call for openssl memory leak.   
    LogManager::Close();

    reset();
}

void BeeEntrance::init_ssl() {
    rtc::InitializeSSL();
}

void BeeEntrance::clear_ssl() {
    rtc::CleanupSSL();
}

void BeeEntrance::configure_net_log(int32_t log_level, TimeType time_base) {
    if (statusd_client_ != NULL) {
        statusd_client_->configure_log(log_level, time_base);
    }
}

void BeeEntrance::write_net_log(const std::string &log) {
    if (is_statusd_logined()) {
        statusd_client_->write_log(log);
    }
}

void BeeEntrance::log_callback(const char *log) {
    BeeEntrance::instance()->on_log(log);
}

void BeeEntrance::on_log(const char *log) {
    if (sink_ != NULL) {
        sink_->on_log(log);
    }
}

void BeeEntrance::print_version() {
    logger_.Info("-------------New Start-------------\n");
    logger_.Info("BeeSDK version: %s.\n", BEE_VERSION);
    logger_.Info("BeeSDK git branch: %s.\n", BUILD_BRANCH);
    logger_.Info("BeeSDK git commit: %s.\n", BUILD_COMMIT);
    logger_.Info("BeeSDK build time: %s.\n", BUILD_TIME);
    logger_.Info("BeeSDK build by: %s.\n", BUILD_BY);
}

BeeErrorCode BeeEntrance::reg_svc(std::shared_ptr<BeeService> svc) {
    BeePromise::Ptr promise(new BeePromise);
    promise->active(io_service_->ios(), -1);
    io_service_->ios()->post(boost::bind(&BeeEntrance::do_reg_svc, shared_from_this(), svc, promise));
    return promise->wait();
}

BeeErrorCode BeeEntrance::unreg_svc(std::shared_ptr<BeeService> svc) {
    BeePromise::Ptr promise(new BeePromise);
    promise->active(io_service_->ios(), -1);
    io_service_->ios()->post(boost::bind(&BeeEntrance::do_unreg_svc, shared_from_this(), svc, promise));
    return promise->wait();
}

} // namespace bee
