#ifndef __LUA_LWS_SERVICE_H__
#define __LUA_LWS_SERVICE_H__

#include "lws_config.h"
#include "libwebsockets.h"
#include "lua_lws_session.h"
#include "utility/common.h"
#include "utility/timer.h"
#include "bee/base/bee_define.h"
#include "network/io_service.h"
#include "log/logger.h"
#include "comLib/Mutex.h"
#include "comLib/Thread.h"
#include "service/bee_promise.h"
#include <list>

typedef struct lws_context_creation_info lws_context_creation_info;
typedef struct lws_client_connect_info lws_client_connect_info;
typedef struct lws_context lws_context;
typedef struct lws_protocols lws_protocols;
typedef enum lws_callback_reasons lws_callback_reasons;

namespace bee {

///////////////////////////////////LwsData///////////////////////////////////////
class LwsData {
public:
    typedef std::shared_ptr<LwsData> Ptr;
    LwsData(const char *buff, size_t len):is_free(true){
        buff_ = new char[len + LWS_PRE];
        memcpy(buff_ + LWS_PRE, buff, len);
        size_ = len;
    }
    ~LwsData(){
        if (buff_ != NULL) {
            delete [] buff_;
        }
    }
    char *data() {return buff_ + LWS_PRE;}
    size_t size() {return size_;}

public:
    char *buff_;
    size_t size_;
    Ptr next;
    bool is_free;
};

typedef std::list<LwsData::Ptr> LwsDataList;

////////////////////////////////////WsiObj//////////////////////////////////////
//Decoupling with LuaLwsSession class, using handle to search LuaLwsSession, using lws to search WsiObj in different thread.
typedef struct WsiObj { 
    typedef std::shared_ptr<WsiObj> Ptr;
    std::shared_ptr<LuaLwsSession> session;
    LwsDataList send_queuqe;
}WsiObj;

//////////////////////////////////LuaLwsService////////////////////////////////////////
class LuaLwsModule;
class LuaLwsService : comLib::Thread {
public:
    typedef std::shared_ptr<LuaLwsService> Ptr;
    typedef std::unordered_map<lws*, WsiObj::Ptr> WsiTable;
    LuaLwsService(LuaLwsModule *lws_module);
    ~LuaLwsService();

public:
    BeeErrorCode start(const std::vector<std::string> &protocol_vec);
    void    stop();
    bool    is_running();
    void    connect(std::shared_ptr<LuaLwsSession> session, const std::string &url, const std::string &protocol);
    void    close(lws *wsi);
    void    close_all();
    void    write(lws *wsi, const char *data);
    int32_t inner_callback(lws *wsi, lws_callback_reasons reason, void *in, size_t len);

protected:
    void    run(void* lpParam);
    int32_t on_writable(lws *wsi, LwsDataList &send_queque);
    void    init_context(const std::vector<std::string> protocol_vec);
    void    notify_writeable(lws *wsi);
    void    check_writeable();

protected:
    LuaLwsModule *lws_module_ = NULL;
    lws_context *context_ = NULL;
    lws_context_creation_info lws_info_;
    std::vector<std::string> protocol_vec_;
    WsiTable wsi_table_;
    lws_protocols *protocols_ = NULL;
    Logger logger_;
    comLib::Mutex mutex_;
    std::set<lws*> writeable_set_;
    BeePromise::Ptr close_promise_;
};

} // namespace bee

#endif
