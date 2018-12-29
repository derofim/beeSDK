#include <service/bee_entrance.h>
#include "lua_lws_service.h"
#include "lua_lws_module.h"
#ifdef IOS
#include "platform/ios/ios_adapter.h"
#endif

namespace bee {

const int32_t kLwsThreadStartStopTimeout = 10000;

static int always_true_callback(X509_STORE_CTX *ctx, void *arg) {
    return 1;
}

static bool pre_handle_event(struct lws *wsi, lws_callback_reasons reason, void *user, void *in, size_t len) {
    bool ret = true;
    switch (reason) {
    case LWS_CALLBACK_CLIENT_ESTABLISHED:
    case LWS_CALLBACK_CLIENT_CONNECTION_ERROR:
    case LWS_CALLBACK_CLOSED:
    case LWS_CALLBACK_WS_PEER_INITIATED_CLOSE:
    case LWS_CALLBACK_WSI_DESTROY:
    case LWS_CALLBACK_CLIENT_WRITEABLE:
    case LWS_CALLBACK_CLIENT_RECEIVE:
        ret = false;
        break;
    case LWS_CALLBACK_OPENSSL_LOAD_EXTRA_CLIENT_VERIFY_CERTS: {
        SSL_CTX *ssl_ctx = (SSL_CTX *)user;
        std::string root_cert = BeeEntrance::instance()->get_root_certificare();
        //If root certificate is supplied, load it to OpenSSL for verifying server certificate.
        if (!root_cert.empty()) {
            X509_STORE *store = NULL;
            X509_LOOKUP *lookup = NULL;
            X509_VERIFY_PARAM *param = X509_VERIFY_PARAM_new();
            X509_VERIFY_PARAM_set_flags(param, X509_V_FLAG_TRUSTED_FIRST);
            SSL_CTX_set1_param((SSL_CTX*)ssl_ctx, param);
            store = SSL_CTX_get_cert_store((SSL_CTX*)ssl_ctx);
            lookup = X509_STORE_add_lookup(store, X509_LOOKUP_file());
            X509_load_cert_file(lookup, root_cert.c_str(), X509_FILETYPE_PEM);
            X509_VERIFY_PARAM_free(param);
        } else {//If root certificate is not supplied, do not verify, just return success.
            SSL_CTX_set_cert_verify_callback(ssl_ctx, always_true_callback, NULL);
        }
        ret = true;
        break;
    }
    default:
        ret = true;
        break;
    }
    return ret;
}

static int32_t lws_calback(struct lws *wsi, lws_callback_reasons reason, void *user, void *in, size_t len) {
    int32_t ret = 0;
    do {
        if (pre_handle_event(wsi, reason, user, in, len)) {
            break;
        }

        if (user == NULL) {
            ret = -1;
            break;
        }

        LuaLwsService *lws_service = (LuaLwsService*)user;
        ret = lws_service->inner_callback(wsi, reason, in, len);
    } while (0);
    return ret;
}

static const struct lws_extension exts[] = {
    {
        "permessage-deflate",
            lws_extension_callback_pm_deflate,
            "permessage-deflate; client_no_context_takeover"
    },
    {
        "deflate-frame",
            lws_extension_callback_pm_deflate,
            "deflate_frame"
    },
    { NULL, NULL, NULL /* terminator */ }
};

LuaLwsService::LuaLwsService(LuaLwsModule *lws_module)
    : lws_module_(lws_module),
      logger_("LuaLwsService") {
    logger_.Debug("LuaLwsService created %x.\n", (unsigned int)(long)this);
}

LuaLwsService::~LuaLwsService() {
    if (protocols_ != NULL) {
        delete protocols_;
    }
    logger_.Debug("LuaLwsService deleted %x.\n", (unsigned int)(long)this);
}

BeeErrorCode LuaLwsService::start(const std::vector<std::string> &protocol_vec) {
    BeeErrorCode ret = kBeeErrorCode_Success;
    do {
        if (is_running()) {
            break;
        }

        if (protocol_vec.empty()) {
            ret = kBeeErrorCode_Invalid_Param;
            break;
        }

        protocol_vec_ = protocol_vec;
        size_t count = protocol_vec_.size();
        protocols_ = new lws_protocols[count + 1];
        for (size_t i = 0; i < count; ++i) {
            memset(&protocols_[i], 0, sizeof(lws_protocols));
            protocols_[i].name = protocol_vec_[i].c_str();
            protocols_[i].callback = lws_calback;
            protocols_[i].per_session_data_size = 0;
            protocols_[i].rx_buffer_size = 32 * 1024;
            protocols_[i].tx_packet_size = 32 * 1024;
        }

        memset(&protocols_[count], 0, sizeof(lws_protocols));
        memset(&lws_info_, 0, sizeof(lws_info_));
        lws_info_.port = CONTEXT_PORT_NO_LISTEN;
        lws_info_.protocols = protocols_;
        lws_info_.gid = -1;
        lws_info_.uid = -1;
        lws_info_.extensions = exts;
        lws_info_.ws_ping_pong_interval = 0;
#if defined(LWS_OPENSSL_SUPPORT)
        lws_info_.options |= LWS_SERVER_OPTION_DO_SSL_GLOBAL_INIT;
#endif

        context_ = lws_create_context(&lws_info_);
        if (context_ == NULL) {
            ret = kBeeErrorCode_Lws_Error;
            break;
        }

        if (!SynStart(NULL, kLwsThreadStartStopTimeout)) {
            ret = kBeeErrorCode_Timeout;
            break;
        }
    } while (0);

    return ret;
}

void LuaLwsService::stop() {
    logger_.Info("LuaLwsService stopping.\n");
    if (is_running()) {
        close_all();
        SynStop(kLwsThreadStartStopTimeout);
    }

    wsi_table_.clear();

    if (context_ != NULL) {
        lws_context_destroy(context_);
        context_ = NULL;
    }
}

bool LuaLwsService::is_running() {
    return m_bRunning;
}

void LuaLwsService::connect(std::shared_ptr<LuaLwsSession> session, const std::string &url, const std::string &protocol) {
    BeeErrorCode ret = kBeeErrorCode_Success;
    char *path = NULL;
    do {
        if (!is_running()) {
            ret = kBeeErrorCode_Service_Not_Started;
            break;
        }

        int use_ssl      = LCCSCF_USE_SSL | LCCSCF_ALLOW_SELFSIGNED | LCCSCF_SKIP_SERVER_CERT_HOSTNAME_CHECK | LCCSCF_ALLOW_EXPIRED;
        int ietf_version = -1;
        const char *p    = NULL;
        const char *prot = NULL;

        lws_client_connect_info ci;
        memset(&ci, 0, sizeof(lws_client_connect_info));

        if (lws_parse_uri((char*)url.c_str(), &prot, &ci.address, &ci.port, &p) != 0) {
            ret = kBeeErrorCode_Invalid_Param;
            break;
        }

        if (p == NULL) {
            ret = kBeeErrorCode_Invalid_Param;
            break;
        }

        size_t len = strlen(p) + 2;
        path = new char[len];
        memset(path, 0, len);
        path[0] = '/';
        strcpy(path + 1, p);

        if (prot != NULL && (strcmp(prot, "ws") == 0 || strcmp(prot, "http") == 0)) {
            use_ssl = 0;
        }

        ci.ietf_version_or_minus_one = ietf_version;
        ci.ssl_connection = use_ssl;
        ci.path     = path;
        ci.context  = context_;
        ci.host     = ci.address;
        ci.origin   = ci.address;
        ci.protocol = (protocol.empty()) ? NULL : protocol.c_str();
        ci.userdata = this;

        lws *wsi    = NULL;
        ci.pwsi     = &wsi;

        comLib::SingleMutex mutex(mutex_);

        wsi = lws_client_connect_via_info(&ci);
        if (wsi == NULL) {
            ret = kBeeErrorCode_Lws_Error;
            break;
        }

        WsiObj::Ptr obj(new WsiObj);
        obj->session = session;
        wsi_table_[wsi] = obj;

        notify_writeable(wsi);
    } while (0);

    if (path != NULL) {
        delete [] path;
    }

    if (session != NULL && ret != kBeeErrorCode_Success) {
        session->report_event(NULL, LWS_CALLBACK_CLIENT_CONNECTION_ERROR, NULL, 0);
    }
}

void LuaLwsService::close(lws *wsi) {
    do {
        comLib::SingleMutex mutex(mutex_);

        if (!is_running()) {
            break;
        }

        WsiTable::iterator iter = wsi_table_.find(wsi);
        if (iter == wsi_table_.end()) {
            break;
        }

        notify_writeable(wsi); //Trigger one loop to close in lws thread.
    } while (0);
}

void LuaLwsService::close_all() {
    bool need_to_wait = false;
    if (close_promise_ == NULL) {
        close_promise_.reset(new BeePromise);
        close_promise_->active(NULL, -1);
    }

    {
        comLib::SingleMutex mutex(mutex_);
        if (!wsi_table_.empty() && context_ != NULL) {
            for (auto iter = wsi_table_.begin();iter != wsi_table_.end();) {
                WsiObj::Ptr obj = iter->second;
                //All lws connections must be closed in lws thread, so if there's any connection not
                //closed yet, must block current thread until every connection is closed.
                if (!obj->session->is_closed()) {
                    need_to_wait = true;
                    iter++;
                } else {
                    logger_.Info("LuaLwsSession %x %x already closed.\n", (unsigned int)(long)obj->session.get(), (unsigned int)(long)iter->first);
                    wsi_table_.erase(iter++);
                }

                //Async close opened connection.
                if (!obj->session->is_closed() && !obj->session->is_closing()) {
                    obj->session->close();
                }
            }
        }
    }

    if (need_to_wait) {
        close_promise_->wait_for(2000); //Wait for notify.
    } else {
        close_promise_.reset();
    }
}

void LuaLwsService::write(lws *wsi, const char *data) {
    do {
        comLib::SingleMutex mutex(mutex_);

        if (!is_running()) {
            break;
        }

        WsiTable::iterator iter = wsi_table_.find(wsi);
        if (iter == wsi_table_.end()) {
            break;
        }

        WsiObj::Ptr obj = iter->second;
        LwsData::Ptr lws_data(new LwsData(data, strlen(data)));
        obj->send_queuqe.push_back(lws_data);

        notify_writeable(wsi);
    } while (0);
}

int32_t LuaLwsService::inner_callback(lws *wsi, lws_callback_reasons reason, void *in, size_t len) {
    int32_t ret = 0;
    do {
        comLib::SingleMutex mutex(mutex_);

        WsiTable::iterator iter = wsi_table_.find(wsi);
        if (iter == wsi_table_.end()) {
            break;
        }

        WsiObj::Ptr obj = iter->second;
        if (obj->session->is_closing()) {
            wsi_table_.erase(iter);
            obj->session->set_closing(false);
            obj->session->set_closed(true);
            logger_.Trace("LuaLwsSession closed %x.\n", (unsigned int)(long)obj->session.get());
            if (wsi_table_.empty()) {
                logger_.Trace("All lws session closed.\n");
                if (close_promise_ != NULL) {
                    close_promise_->honor(kBeeErrorCode_Success);
                    close_promise_.reset();
                }
            }
            ret = -1; //Return -1 notifies libwebsockets to close ws connection.
            break;
        }

        if (reason == LWS_CALLBACK_CLIENT_WRITEABLE) {
            ret = on_writable(wsi, obj->send_queuqe);
            break;
        } else {
            if (reason == LWS_CALLBACK_CLIENT_ESTABLISHED) {
                logger_.Trace("LuaLwsSession connected %x.\n", (unsigned int)(long)obj->session.get());
                notify_writeable(wsi);
            }

            if (obj->session == NULL) {
                break;
            }

            obj->session->report_event(wsi, reason, in, len);
            break;
        }
    } while (0);
    return ret;
}

void LuaLwsService::run(void* lpParam) {
    logger_.Info("LuaLwsService running.\n");
    while (is_running()) {
        lws_service(context_, 40);
        check_writeable();
    }
    logger_.Info("LuaLwsService stopped.\n");
}

int32_t LuaLwsService::on_writable(lws *wsi, LwsDataList &send_queque) {
    int32_t ret = 0;
    do {
        if (send_queque.empty()) {
            break;
        }

        LwsData::Ptr data = send_queque.front();
        send_queque.pop_front();
        if (data == NULL) {
            break;
        }

        int32_t n = lws_write(wsi, (unsigned char*)data->data(), data->size(), static_cast<lws_write_protocol>(LWS_WRITE_TEXT));
        if (n < 0) {
            ret = -1;
            break;
        }

        if (n < (int32_t)data->size()) {
            lwsl_err("Partial write LWS_CALLBACK_CLIENT_WRITEABLE\n");
            ret = -1;
            break;
        }

        //Done for this round, check for next message later.
        lws_callback_on_writable(wsi);
    } while (0);
    return ret;
}

void LuaLwsService::init_context(const std::vector<std::string> protocol_vec) {
    protocol_vec_ = protocol_vec;
    size_t count = protocol_vec_.size();
    protocols_ = new lws_protocols[count + 1];
    for (size_t i = 0; i < count; ++i) {
        protocols_[i].name = protocol_vec_[i].c_str();
        protocols_[i].callback = lws_calback;
        protocols_[i].per_session_data_size = 0;
        protocols_[i].rx_buffer_size = 1024;
    }
    memset(&protocols_[count], 0, sizeof(lws_protocols));
    memset(&lws_info_, 0, sizeof(lws_info_));
    lws_info_.port = CONTEXT_PORT_NO_LISTEN;
    lws_info_.protocols = protocols_;
    lws_info_.gid = -1;
    lws_info_.uid = -1;
    lws_info_.extensions = exts;
    lws_info_.ws_ping_pong_interval = 0;
#if defined(LWS_OPENSSL_SUPPORT)
    lws_info_.options |= LWS_SERVER_OPTION_DO_SSL_GLOBAL_INIT;
#endif

    context_ = lws_create_context(&lws_info_);
}

void LuaLwsService::notify_writeable(lws *wsi) {
    comLib::SingleMutex mutex(mutex_);
    writeable_set_.insert(wsi);
    if (context_ != NULL) {
        lws_cancel_service(context_);
    }
}

void LuaLwsService::check_writeable() {
    comLib::SingleMutex mutex(mutex_);
    if (writeable_set_.empty()) {
        return;
    }

    for (lws *wsi : writeable_set_) {
        WsiTable::iterator iter = wsi_table_.find(wsi);
        if (iter == wsi_table_.end()) {
            continue;
        }
        if (!iter->second->session->is_closed()) {
            lws_callback_on_writable(wsi);
        }
    }

    writeable_set_.clear();
}

} // namespace bee
