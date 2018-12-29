#include "ca_client.h"
#include "utility/timer.h"
#include "utility/algorithm.h"

namespace bee {

///////////////////////////////////AsyncStateMachine Definition///////////////////////////////////////
BEGIN_STATE_MACHINE(CAClient)
    BEGIN_STATE(eCAState_Resolve, TcpResolveState, &CAClient::handle_Resolve, true)
        SWITCH_TO(eCAState_ConnectCA, &CAClient::Transform_Resolve_To_ConnectCA)
    END_STATE

    BEGIN_STATE(eCAState_ConnectCA, TcpConnectState, &CAClient::handle_ConnectCA, false)
        SWITCH_TO(eCAState_SendPubKey, &CAClient::Transform_ConnectCA_To_SendPubKey)
    END_STATE

    BEGIN_STATE(eCAState_SendPubKey, TcpSendState, &CAClient::handle_SendPubKey, false)
        SWITCH_TO(eCAState_RcvPubKey, &CAClient::Transform_To_RcvTlv)
    END_STATE

    BEGIN_STATE(eCAState_RcvPubKey, TcpReceiveTlvState, &CAClient::handle_RcvPubKey, false)
        SWITCH_TO(eCAState_SendSignature, &CAClient::Transform_RcvPubKey_To_SendSignature)
    END_STATE

    BEGIN_STATE(eCAState_SendSignature,TcpSendState, &CAClient::handle_SendSignature, false)
        SWITCH_TO(eCAState_RcvSignature,&CAClient::Transform_To_RcvTlv)
    END_STATE

    BEGIN_STATE(eCAState_RcvSignature, TcpReceiveTlvState, &CAClient::handle_RcvSignature, false)
        SWITCH_TO(eCAState_RequestShareKey, &CAClient::Transform_RcvSignature_To_RequestShareKey)
    END_STATE

    BEGIN_STATE(eCAState_RequestShareKey, TcpSendState, &CAClient::handle_RequestShareKey, false)
        SWITCH_TO(eCAState_RcvShareKey, &CAClient::Transform_To_RcvTlv)
    END_STATE

    BEGIN_STATE(eCAState_RcvShareKey, TcpReceiveTlvState, &CAClient::handle_RcvShareKey, false)
        SWITCH_TO(eCAState_RequestLua, &CAClient::Transform_RcvShareKey_To_RequestLua)
    END_STATE

    BEGIN_STATE(eCAState_RequestLua, TcpSendState, &CAClient::handle_RequestLua, false)
        SWITCH_TO(eCAState_RcvLua, &CAClient::Transform_To_RcvBigTlv)
    END_STATE

    BEGIN_STATE(eCAState_RcvLua, TcpReceiveTlvState, &CAClient::handle_RcvLua, false)
    END_STATE
END_STATE_MACHINE

int32_t CAClient::last_connected_backup_ca_index_ = 0;

CAClient::CAClient(IOSPtr ios):
    TcpStateMachine(ios),
    client_rsa_(NULL),
    server_rsa_(NULL),
    gslb_ts_(0),
    logger_("CAClient") {
}

CAClient::~CAClient() {
    reset();
}

BeeErrorCode CAClient::async_request_core(const std::string &host, const std::string &service, int32_t timeout, CAHandler::SPtr handler) {
    BeeErrorCode ret = kBeeErrorCode_Success;
    do {
        if (ios_ == NULL) {
            ret = kBeeErrorCode_Service_Not_Started;
            break;
        }

        handler_ = handler;
        ios_->post(boost::bind(&CAClient::request_core, shared_from_base<CAClient>(), host, service, timeout));
    } while (0);
    return ret;
}

int32_t CAClient::handle_Resolve(StateEvent::Ptr ev) {
    int32_t next_state = kInvalidState;
    do {
        if (ev->ec1 != kBeeErrorCode_Success || ev->ec2) {
            using_backup_ca_addr_ = (kBackupCAIpCount > 0);
            if (using_backup_ca_addr_) {
                logger_.Warn("Resolve fail with error %d %d, will using backup address.\n", ev->ec1, ev->ec2.value());
            }
        }

        next_state = eCAState_ConnectCA;
    } while (0);
    return next_state;
}

int32_t CAClient::handle_ConnectCA(StateEvent::Ptr ev) {
    int32_t next_state = kInvalidState;
    do {
        TcpConnectOutputEvent *output_ev = (TcpConnectOutputEvent*)ev.get();
        if (output_ev->ec1 != kBeeErrorCode_Success || output_ev->ec2) {
            break;
        }

        if (!generate_client_rsa_key()) {
            ev->ec1 = kBeeErrorCode_Crypto_Error;
            break;
        }

        if (using_backup_ca_addr_) {
            last_connected_backup_ca_index_ = output_ev->index;
        }

        next_state = eCAState_SendPubKey;
    } while (0);
    return next_state;
}

int32_t CAClient::handle_SendPubKey(StateEvent::Ptr ev) {
    int32_t next_state = kInvalidState;
    do {
        if (ev->ec1 != kBeeErrorCode_Success || ev->ec2) {
            break;
        }

        next_state = eCAState_RcvPubKey;
    } while (0);
    return next_state;
}

int32_t CAClient::handle_RcvPubKey(StateEvent::Ptr ev) {
    int32_t next_state = kInvalidState;
    do {
        if (ev->ec1 != kBeeErrorCode_Success || ev->ec2) {
            break;
        }

        TcpRcvTlvOutputEvent *output_ev = (TcpRcvTlvOutputEvent*)ev.get();
        CAPubKey *pkg = (CAPubKey *)output_ev->rcv_buffer;
        if (pkg == NULL) {
            ev->ec1 = kBeeErrorCode_Null_Pointer;
            break;
        }

        if (ntohl(pkg->header.mark) != CA_MARK) {
            ev->ec1 = kBeeErrorCode_Invalid_Protocol_Mark;
            break;
        }

        if (pkg->header.code != CA_CLIENT_PUBLIC_KEY) {
            ev->ec1 = kBeeErrorCode_Invalid_Message;
            break;
        }

        if (!process_server_rsa_key((char*)pkg->pub_key,sizeof(pkg->pub_key))) {
            ev->ec1 = kBeeErrorCode_Crypto_Error;
            break;
        }

        if (!create_signature()) {
            ev->ec1 = kBeeErrorCode_Signature_Error;
            break;
        }

        next_state = eCAState_SendSignature;
    } while (0);
    return next_state;
}

int32_t CAClient::handle_SendSignature(StateEvent::Ptr ev) {
    int32_t next_state = kInvalidState;
    do {
        if (ev->ec1 != kBeeErrorCode_Success || ev->ec2) {
            break;
        }

        next_state = eCAState_RcvSignature;
    } while (0);
    return next_state;
}

int32_t CAClient::handle_RcvSignature(StateEvent::Ptr ev) {
    int32_t next_state = kInvalidState;
    do {
        if (ev->ec1 != kBeeErrorCode_Success || ev->ec2) {
            break;
        }

        TcpRcvTlvOutputEvent *output_ev = (TcpRcvTlvOutputEvent*)ev.get();
        CASignature *pkg = (CASignature *)output_ev->rcv_buffer;
        if (pkg == NULL) {
            ev->ec1 = kBeeErrorCode_Null_Pointer;
            break;
        }

        if (ntohl(pkg->header.mark) != CA_MARK) {
            ev->ec1 = kBeeErrorCode_Invalid_Protocol_Mark;
            break;
        }

        if (pkg->header.code != CA_CLIENT_SIGNATURE) {
            ev->ec1 = kBeeErrorCode_Invalid_Message;
            break;
        }

        uint32_t pkg_len = ntohs(pkg->header.length);
        uint32_t in_len = pkg_len - kCAHeaderLen;
        IOBuffer dec_buff(in_len);
        int32_t out_len = 0;
        if (!rsa_decrypt(client_rsa_, (uint8_t*)output_ev->rcv_buffer + kCAHeaderLen, in_len, (uint8_t*)dec_buff.data(), out_len, RSA_PKCS1_PADDING)) {
            ev->ec1 = kBeeErrorCode_Crypto_Error;
            break;
        }

        if (out_len == -1) {
            ev->ec1 = kBeeErrorCode_Crypto_Error;
            break;
        }

        if (!check_signature((uint8_t*)dec_buff.data(),out_len,(const uint8_t *)server_public_key_.c_str(),server_public_key_.size())) {
            ev->ec1 = kBeeErrorCode_Signature_Error;
            break;
        }

        next_state = eCAState_RequestShareKey;
    } while (0);
    return next_state;
}

int32_t CAClient::handle_RequestShareKey(StateEvent::Ptr ev) {
    int32_t next_state = kInvalidState;
    do {
        if (ev->ec1 != kBeeErrorCode_Success || ev->ec2) {
            break;
        }

        next_state = eCAState_RcvShareKey;
    } while (0);
    return next_state;
}

int32_t CAClient::handle_RcvShareKey(StateEvent::Ptr ev) {
    int32_t next_state = kInvalidState;
    do {
        if (ev->ec1 != kBeeErrorCode_Success || ev->ec2) {
            break;
        }

        TcpRcvTlvOutputEvent *output_ev = (TcpRcvTlvOutputEvent*)ev.get();
        CASignature *pkg = (CASignature *)output_ev->rcv_buffer;
        if (pkg == NULL) {
            ev->ec1 = kBeeErrorCode_Null_Pointer;
            break;
        }

        if (ntohl(pkg->header.mark) != CA_MARK) {
            ev->ec1 = kBeeErrorCode_Invalid_Protocol_Mark;
            break;
        }

        if (pkg->header.code != CA_REPLY_ENCRYPT_MD5) {
            ev->ec1 = kBeeErrorCode_Invalid_Message;
            break;
        }

        uint32_t pkg_len = ntohs(pkg->header.length);
        uint32_t in_len = pkg_len - kCAHeaderLen;
        IOBuffer dec_buff(in_len);
        int32_t out_len = 0;
        if (!rsa_decrypt(client_rsa_, (uint8_t*)output_ev->rcv_buffer + kCAHeaderLen, in_len, (uint8_t*)dec_buff.data(), out_len, RSA_PKCS1_PADDING)) {
            ev->ec1 = kBeeErrorCode_Crypto_Error;
            break;
        }

        if (out_len == -1) {
            ev->ec1 = kBeeErrorCode_Crypto_Error;
            break;
        }

        CAKey *rs = (CAKey *)dec_buff.data();
        gslb_ts_  = _ntohll(rs->msec);
        gslb_key_.assign(rs->key, sizeof(rs->key)); 
        lua_md5_.assign(rs->md5, sizeof(rs->md5));
        CAHandler::SPtr handler = handler_.lock();
        bool same_version = false;
        if (handler != NULL) {
            same_version = handler->check_lua_md5(lua_md5_);
        }
        
        if (same_version) {
            logger_.Info("Old md5, using cached lua.\n");
            next_state = kFinishedState;
        } else {
            logger_.Info("New md5, will download lua.\n");
            next_state = eCAState_RequestLua;
        }
    } while (0);
    return next_state;
}

int32_t CAClient::handle_RequestLua(StateEvent::Ptr ev) {
    int32_t next_state = kInvalidState;
    do {
        if (ev->ec1 != kBeeErrorCode_Success || ev->ec2) {
            break;
        }

        next_state = eCAState_RcvLua;
    } while (0);
    return next_state;
}

int32_t CAClient::handle_RcvLua(StateEvent::Ptr ev) {
    int32_t next_state = kInvalidState;
    do {
        if (ev->ec1 != kBeeErrorCode_Success || ev->ec2) {
            break;
        }

        TcpRcvTlvOutputEvent *output_ev = (TcpRcvTlvOutputEvent*)ev.get();
        CABigHeader *header = (CABigHeader *)output_ev->rcv_buffer;
        if (header == NULL) {
            ev->ec1 = kBeeErrorCode_Null_Pointer;
            break;
        }

        if (ntohl(header->mark) != CA_MARK) {
            ev->ec1 = kBeeErrorCode_Invalid_Protocol_Mark;
            break;
        }

        if (header->code != CA_REPLY_ENCRYPT_BIGLUA) {
            ev->ec1 = kBeeErrorCode_Invalid_Message;
            break;
        }

        uint32_t pkg_len = ntohl(header->length);
        uint32_t in_len = pkg_len - kCABigHeaderLen;
        IOBuffer dec_buff(in_len);
        int32_t  out_len = 0;
        uint8_t  *lua_ptr = (uint8_t*)output_ev->rcv_buffer + kCABigHeaderLen;
        if (!rc4(lua_ptr, in_len, (uint8_t*)gslb_key_.c_str(), gslb_key_.size(), lua_ptr, out_len)) {
            ev->ec1 = kBeeErrorCode_Crypto_Error;
            break;
        }

        CALuaData *ca_lua = (CALuaData *)lua_ptr;
        ca_lua->lua_len   = ntohl(ca_lua->lua_len);

        uLong dz_len = ca_lua->lua_len * 3;
        IOBuffer dz(dz_len);
        if (-1 == gz_decompress((Byte*)ca_lua->lua, ca_lua->lua_len, (Byte*)dz.data(), &dz_len)) {
            ev->ec1 = kBeeErrorCode_Decompress_Error;
            break;
        }

        lua_.assign(dz.data(), dz_len); 

#ifdef WIN32
        FILE *fp = fopen("test.lua","wb+");
        fwrite(lua_.c_str(),lua_.size(),1,fp);
        fclose(fp);
#endif

        CAHandler::SPtr handler = handler_.lock();
        if (handler != NULL) {
            handler->handle_ca_data(kBeeErrorCode_Success, ev->ec2, lua_, lua_md5_, gslb_key_, gslb_ts_);
        }

        next_state = kFinishedState;
    } while (0);
    return next_state;
}

bool CAClient::Transform_Resolve_To_ConnectCA(StateEvent::Ptr input, StateEvent::Ptr output) {
    bool ret = true;
    do {
        TcpResolveOutputEvent *resolve_ev = (TcpResolveOutputEvent*)input.get();        
        if (resolve_ev->resolved_endpoints.empty() && 
            using_backup_ca_addr_ &&
            kBackupCAIpCount > 0) {
            //Resolve fail, using backup CA address.
            for (int32_t i = 0; i < kBackupCAIpCount; i++) {
                logger_.Info("Add backup CA endpoint %dth %s:%s.\n", i, kBackupCAIps[i], kCAService.c_str());
                boost::asio::ip::address addr = boost::asio::ip::address::from_string(kBackupCAIps[i]);
                tcp::endpoint ep(addr, atoi(kCAService.c_str()));
                resolve_ev->resolved_endpoints.push_back(ep);
            }
        }

        TcpConnectInputEvent *connect_ev = (TcpConnectInputEvent*)output.get();
        connect_ev->endpoints = resolve_ev->resolved_endpoints;
        connect_ev->connect_timeout = connect_timeout_;
        if (using_backup_ca_addr_ && 
            last_connected_backup_ca_index_ >= 0 && 
            last_connected_backup_ca_index_ < kBackupCAIpCount) {
            logger_.Info("First priority endpoint %dth %s:%s.\n", 
                last_connected_backup_ca_index_, 
                kBackupCAIps[last_connected_backup_ca_index_], 
                kCAService.c_str());
            connect_ev->start_index = last_connected_backup_ca_index_;
        }
        connect_ev->setup = true;
    } while (0);
    return ret;
}

bool CAClient::Transform_ConnectCA_To_SendPubKey(StateEvent::Ptr input, StateEvent::Ptr output) {
    bool ret = true;
    do {
        if (client_public_key_.empty()) {
            ret = false;
            break;
        }

        output->data = IOBuffer(kClientPubPkgLen);

        PlayerPubKey *pkg  = (PlayerPubKey *)output->data.data();
        pkg->header.length = htons(kClientPubPkgLen);
        pkg->header.code   = CLIENT_CA_PUBLIC_KEY;
        pkg->header.mark   = htonl(CA_MARK);
        memcpy(pkg->pub_key, client_public_key_.c_str(), client_public_key_.length() > PUBLIC_KEY_LENGTH ? PUBLIC_KEY_LENGTH : client_public_key_.length());

        output->setup = true;
    } while (0);
    return ret;
}

bool CAClient::Transform_RcvPubKey_To_SendSignature(StateEvent::Ptr input, StateEvent::Ptr output) {
    bool ret = true;
    do {
        if (output->data.size() < 1024) {
            output->data = IOBuffer(1024);
        }

        int32_t out_len = 0;
        ret = rsa_encrypt(server_rsa_, (uint8_t *)&signature_, kSignatureLen, (uint8_t *)(output->data.data() + kCAHeaderLen), out_len, RSA_PKCS1_PADDING);
        if (!ret) {
            input->ec1 = kBeeErrorCode_Crypto_Error;
            break;
        }

        CAHeader *pkg   = (CAHeader *)output->data.data();
        uint16_t len    = (uint16_t)(kCAHeaderLen + out_len);
        pkg->length     = htons(len);
        pkg->code       = CLIENT_CA_SIGNATURE;
        pkg->mark       = htonl(CA_MARK);

        if (len < output->data.capacity()) {
            output->data.resize(len);
        }
        output->setup = true;
    } while (0);
    return ret;
}

bool CAClient::Transform_RcvSignature_To_RequestShareKey(StateEvent::Ptr input, StateEvent::Ptr output) {
    bool ret = true;
    do {
        if (output->data.size() < 1024) {
            output->data = IOBuffer(1024);
        }

        uint8_t uid[256] = {0};
        size_t uid_len = strlen(DRM_LUA_NAME);
        memcpy(uid, DRM_LUA_NAME, uid_len);

        int32_t out_len = 0;
        ret = rsa_encrypt(server_rsa_, uid, uid_len, (uint8_t *)(output->data.data() + kCAHeaderLen), out_len, RSA_PKCS1_PADDING);
        if (!ret) {
            input->ec1 = kBeeErrorCode_Crypto_Error;
            break;
        }

        CAHeader *pkg   = (CAHeader *)output->data.data();
        uint16_t len    = (uint16_t)(kCAHeaderLen + out_len);
        pkg->length     = htons(len);
        pkg->code       = CLIENT_REQUEST_ENCRYPT_KEY_WITH_STR;
        pkg->mark       = htonl(CA_MARK);

        if (len < output->data.capacity()) {
            output->data.resize(len);
        }
        output->setup = true;
    } while (0);
    return ret;
}

bool CAClient::Transform_RcvShareKey_To_RequestLua(StateEvent::Ptr input, StateEvent::Ptr output) {
    bool ret = true;
    do {    
        if (output->data.size() < 1024) {
            output->data = IOBuffer(1024);
        }

        CAHeader *pkg   = (CAHeader *)output->data.data();
        pkg->length     = htons(kCAHeaderLen);
        pkg->code       = CLIENT_REQUEST_LUA;
        pkg->mark       = htonl(CA_MARK);

        output->data.resize(kCAHeaderLen);
        output->setup = true;
    } while (0);
    return ret;
}

bool CAClient::Transform_To_RcvTlv(StateEvent::Ptr input, StateEvent::Ptr output) {
    bool ret = true;
    do {
        TcpRcvTlvInputEvent *ev = (TcpRcvTlvInputEvent*)output.get();
        ev->length_offset = OffsetOfSM(CAHeader,length);
        ev->length_size = SizeOfSM(CAHeader,length);
        ev->setup = true;
    } while (0);
    return ret;
}

bool CAClient::Transform_To_RcvBigTlv(StateEvent::Ptr input, StateEvent::Ptr output) {
    bool ret = true;
    do {
        TcpRcvTlvInputEvent *ev = (TcpRcvTlvInputEvent*)output.get();
        ev->length_offset = OffsetOfSM(CABigHeader,length);
        ev->length_size = SizeOfSM(CABigHeader,length);
        ev->setup = true;
    } while (0);
    return ret;
}

void CAClient::reset() {
    if (client_rsa_ != NULL) {
        RSA_free(client_rsa_);
        client_rsa_ = NULL;
    }

    if (server_rsa_ != NULL) {
        RSA_free(server_rsa_);
        server_rsa_ = NULL;
    }
}

bool CAClient::generate_client_rsa_key() {
    bool ret  = true;
    do {
        if (client_rsa_ != NULL) {
            break;
        }

        ret = rsa_generate_key(client_rsa_,client_public_key_);
    } while (0);
    return ret;
}

bool CAClient::process_server_rsa_key(const char *pub_key, size_t pub_key_len) {
    bool ret = true;
    do {
        if (pub_key == NULL || pub_key_len == 0) {
            ret = false;
            break;
        }

        server_public_key_.assign(pub_key, pub_key_len);
        ret = rsa_generate_from_pub_key(pub_key, pub_key_len, server_rsa_);
        if (!ret) {
            break;
        }
    } while (0);
    return ret;
}

bool CAClient::create_signature()  {
    if (client_public_key_.empty()) {
        return false;
    }

    memset(&signature_, 0, sizeof(Signature));
    uint32_t seedp = (uint32_t)MillisecTimer::get_tickcount();
    char value[128] = {0};
    const char *ori = client_public_key_.c_str() + sizeof("-----BEGIN PUBLIC KEY-----");
    uint32_t r[4] = {0}, value_len = 0, ori_pos = 0;
    for(int32_t i = 0; i < 4; i++) {
        r[i] = (uint32_t)rand_r(&seedp);
        for(int32_t j = 0; j < 32; j++) {
            if ((r[i] >> j) & 1) {
                value[value_len++] = ori[ori_pos++];
            } else {
                ori_pos++;
            }
        }
        r[i] = htonl(r[i]);
    }

    sha1((uint8_t*)value,value_len,signature_.hash,sizeof(signature_.hash));

    uint8_t *pr = (uint8_t *)r;
    value[2] = 0;
    for(int32_t i = 0, j = 0; i < 16; i++, j+=2) {
        snprintf(value, 3, "%02x", pr[i]);
        signature_.r[j] = value[0];
        signature_.r[j+1] = value[1];
    }

    uint8_t *s = (uint8_t *)&signature_;
    signature_.sum = check_sum(s+2, sizeof(Signature) - 2);
    signature_.sum = htons(signature_.sum);

    return true;
}

bool CAClient::check_signature(const uint8_t *signature, uint32_t signature_length, const uint8_t *pub_key, uint32_t pub_key_length) {
    bool ret = true;
    do {
        if (sizeof(Signature) != signature_length) {
            ret = false;
            break;
        }

        if (sizeof("-----BEGIN PUBLIC KEY-----") >= pub_key_length) {
            ret = false;
            break;
        }

        pub_key += sizeof("-----BEGIN PUBLIC KEY-----");
        pub_key_length -= sizeof("-----BEGIN PUBLIC KEY-----");

        Signature *s = (Signature *)signature;
        if (s->sum != ntohs(check_sum(signature + 2, signature_length - 2))) {
            ret = false;
            break;
        }

        uint8_t v[128] = {0};
        uint32_t v_len = 0, ori_pos = 0, r[4] = {0};
        uint8_t *pr = (uint8_t *)r;

        char *endptr = NULL;
        v[2] = 0;
        for(int32_t i = 0, j = 0; i < 32; i += 2, j++) {
            v[0] = s->r[i];
            v[1] = s->r[i+1];
            uint32_t b = strtol((const char *)v, &endptr, 16);
            if (0 != *endptr) {
                ret = false;
                break;
            }
            pr[j] = b&0xff;
        }

        if (!ret) {
            break;
        }

        for(int32_t i = 0; i < 4; i++) {
            r[i] = ntohl(r[i]);
            for (int32_t j = 0; j < 32; j++) {
                if (pub_key_length == ori_pos) {
                    ret = false;
                    break;
                }

                if ((r[i] >> j) & 1) {
                    v[v_len++] = pub_key[ori_pos++];
                } else {
                    ori_pos++;
                }
            }

            if (!ret) {
                break;
            }
        }

        if (!ret) {
            break;
        }

        uint8_t hash[SHA_DIGEST_LENGTH] = {0};
        sha1(v,v_len,hash,sizeof(hash));
        ret = (0 == memcmp(s->hash, hash, SHA_DIGEST_LENGTH));
    } while (0);
    return ret;
}

uint16_t CAClient::check_sum(const uint8_t *buffer, uint32_t len) {
    uint32_t sum = 0;
    uint32_t i = 0;
    for(; i < len; i += 2) {
        sum += (uint16_t)((buffer[i] << 8) | buffer[i + 1]);
    }

    if ( i != len ) {
        sum += buffer[len-1];
    }

    sum = (sum >> 16) + (sum & 0xffff);
    sum += (sum >> 16);
    return ~sum;
}

bool CAClient::request_core(const std::string &host, const std::string &service, int32_t timeout) {
    connect_timeout_ = timeout;
    bool ret = start(host, service);
    if (!ret) {
        CAHandler::SPtr handler = handler_.lock();
        if (handler != NULL) {
            handler->handle_ca_data(kBeeErrorCode_Error_State_Machine, boost::system::error_code(), "", "", "", 0);
        }
    }
    return ret;
}

void CAClient::output(StateEvent::Ptr ev) {
    CAHandler::SPtr handler = handler_.lock();
    if (handler != NULL && 
        ev != NULL && 
        ev->ec1 != kBeeErrorCode_Success &&
        ev->ec1 != kBeeErrorCode_State_Machine_Finished) {
        handler->handle_ca_data(ev->ec1, ev->ec2, "", "", "", 0);
    }
}

} //namespace bee
