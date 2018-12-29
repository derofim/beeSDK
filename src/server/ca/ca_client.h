#ifndef __CA_CLIENT_H__
#define __CA_CLIENT_H__

#include "ca_def.h"
#include "bee/base/bee_define.h"
#include "network/tcp_state_machine.h"
#include "log/logger.h"

namespace bee {

//////////////////////////////////CAHandler////////////////////////////////////////
class CAHandler {
public:
    typedef std::shared_ptr<CAHandler> SPtr;
    typedef std::weak_ptr<CAHandler> WPtr;
    CAHandler(){}
    virtual ~CAHandler(){}

public:
    virtual bool check_lua_md5(const std::string &md5) = 0;
    virtual void handle_ca_data(
        BeeErrorCode ec1,
        const boost::system::error_code& ec2,
        const std::string &lua,
        const std::string &md5,
        const std::string &gslb_key,
        uint64_t gslb_ts) = 0;
};

///////////////////////////////////CAClient///////////////////////////////////////
class CAClient : public TcpStateMachine {
public:
    typedef std::shared_ptr<CAClient> Ptr;
    CAClient(IOSPtr ios);
	virtual ~CAClient();

public:
    DECLARE_STATE_MACHINE
    BeeErrorCode async_request_core(const std::string &host, const std::string &service, int32_t timeout, CAHandler::SPtr handler);

    //Routers
    int32_t handle_Resolve(StateEvent::Ptr ev);
    int32_t handle_ConnectCA(StateEvent::Ptr ev);
    int32_t handle_SendPubKey(StateEvent::Ptr ev);
    int32_t handle_RcvPubKey(StateEvent::Ptr ev);
    int32_t handle_SendSignature(StateEvent::Ptr ev);
    int32_t handle_RcvSignature(StateEvent::Ptr ev);
    int32_t handle_RequestShareKey(StateEvent::Ptr ev);
    int32_t handle_RcvShareKey(StateEvent::Ptr ev);
    int32_t handle_RequestLua(StateEvent::Ptr ev);
    int32_t handle_RcvLua(StateEvent::Ptr ev);

    //Transforms
    bool Transform_Resolve_To_ConnectCA(StateEvent::Ptr input, StateEvent::Ptr output);
    bool Transform_ConnectCA_To_SendPubKey(StateEvent::Ptr input, StateEvent::Ptr output);
    bool Transform_RcvPubKey_To_SendSignature(StateEvent::Ptr input, StateEvent::Ptr output);
    bool Transform_RcvSignature_To_RequestShareKey(StateEvent::Ptr input, StateEvent::Ptr output);
    bool Transform_RcvShareKey_To_RequestLua(StateEvent::Ptr input, StateEvent::Ptr output);
    bool Transform_To_RcvTlv(StateEvent::Ptr input, StateEvent::Ptr output);
    bool Transform_To_RcvBigTlv(StateEvent::Ptr input, StateEvent::Ptr output);

protected:
    void reset();
    bool generate_client_rsa_key();
    bool process_server_rsa_key(const char *pub_key, size_t pub_key_len);
    bool create_signature();
    bool check_signature(const uint8_t *signature, uint32_t signature_length, const uint8_t *pub_key, uint32_t pub_key_length);
    uint16_t check_sum(const uint8_t *buffer, uint32_t len);
    bool request_core(const std::string &host, const std::string &service, int32_t timeout);
    void output(StateEvent::Ptr ev);
    
private:
    RSA *client_rsa_;
    RSA *server_rsa_;
    std::string client_public_key_;
    std::string server_public_key_;
    Signature   signature_;
    std::string lua_;
    std::string lua_md5_;
    std::string gslb_key_;
    uint64_t    gslb_ts_;
    CAHandler::WPtr handler_;
    Logger      logger_;
    int32_t     connect_timeout_ = -1;
    bool        using_backup_ca_addr_ = false;
    static int32_t last_connected_backup_ca_index_;
};

} // namespace bee

#endif
