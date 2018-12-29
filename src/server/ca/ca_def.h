#ifndef __CA_DEFINE_H__
#define __CA_DEFINE_H__

#include "utility/common.h"
#include "utility/crypto.h"

#pragma pack(1)

typedef enum CAState {
    eCAState_Resolve = 0,
    eCAState_ConnectCA,
    eCAState_SendPubKey,
    eCAState_RcvPubKey,
    eCAState_SendSignature,
    eCAState_RcvSignature,
    eCAState_RequestShareKey,
    eCAState_RcvShareKey,
    eCAState_RequestLua,
    eCAState_RcvLua
}CAState;

#define CA_MARK                     0x53484341
#define CA_RESERVED                 0x0

// protocols 
#define CLIENT_CA_PUBLIC_KEY        0x10  
#define CA_CLIENT_PUBLIC_KEY        0x20  

#define CLIENT_CA_SIGNATURE         0x11 
#define CA_CLIENT_SIGNATURE         0x21 

#define CLIENT_REQUEST_ENCRYPT_KEY_WITH_UID  0x12
#define CLIENT_REQUEST_ENCRYPT_KEY_WITH_STR  0x13
#define CLIENT_REQUEST_LUA          0x14
#define CA_REPLY_ENCRYPT_KEY        0x22
#define CA_REPLY_ENCRYPT_LUA        0x23
#define CA_REPLY_ENCRYPT_BIGLUA     0x24
#define CA_REPLY_ENCRYPT_MD5        0x25

#define CA_CLIENT_ERROR             0x33
#define CLIENT_CA_ERROR             CA_CLIENT_ERROR

#define CA_HEART                    0x44

// error code
#define ERROR_PROTOCOL_FORMAT       0x01                        // protocol format error
#define ERROR_REQUEST_UNORDERED     0x02                        // request unordered, invalid request
#define ERROR_CLIENT_SIGNATURE      CLIENT_CA_SIGNATURE	        // Player's signature checking failed
#define ERROR_CA_SIGNATUR           CA_CLIENT_SIGNATURE         // CA's signature check failed.
#define ERROR_CLIENT_CA_PUBLIC_KEY  CLIENT_CA_PUBLIC_KEY
#define CA_MD5_SIZE	                32

#define DRM_LUA_NAME "new_live"
#define LOCAL_DRM_LUA_NAME "local"

#pragma pack(1)
struct CAHeader {
    uint16_t    length; 	      /* Protocol length */
    uint8_t     code;             /* command code */
    uint32_t    mark;             /* constant value 0x53484341*/
};

struct CABigHeader {
    uint32_t    length; 	      /* Protocol length */
    uint8_t     code;             /* command code */
    uint32_t    mark;             /* constant value 0x53484341*/
};

typedef CAHeader CAHeart;

// player -> CA
struct PlayerPubKey {
    CAHeader header; // code 0x10
    uint8_t  pub_key[PUBLIC_KEY_LENGTH]; /* public key */
};

// CA-> player
// code 0x20
typedef PlayerPubKey CAPubKey;  

// player->CA signature
struct PlayerSignature {
    CAHeader header; // code 0x11
    uint8_t  signature[ENCRYPT_BUFFER_SIZE]; /* Signature, encrypted by public key of CA*/
};

/* Encrypted by public key of Player*/
// code 0x21
typedef PlayerSignature CASignature; 

/* After signature checking, send this requests, this key will be used in encrypt content from CDN */
struct REQCAKey {   
    CAHeader header; //  code 0x12
    uint64_t  uid;
};
/* CA send to Player the key for encrypting data which from CDN */

// CAKey should be encrypted.
struct CAKey {
    uint64_t msec; //the time when getting encrypt key. milliseconds
    char key[ENCRYPT_KEY_SIZE];
    char md5[CA_MD5_SIZE];
}; 

struct CALua {
    CAHeader header;  
    uint32_t lua_len;
    char lua[0];
}; 

struct CABigLua {
    CABigHeader header;  
    uint32_t lua_len;
    char lua[0];
}; 

struct RESCAKey {
    CAHeader header; // code 0x22
    uint8_t  cakey[0];
};

struct CAError {
    CAHeader header; // code 0x33
    uint8_t      error;   
};

struct Signature {
    uint16_t sum;
    uint8_t r[32];
    uint8_t hash[SHA_DIGEST_LENGTH];
};

struct CALuaData {
    uint32_t lua_len;
    char lua[0];
};

#pragma pack()

const size_t kClientPubPkgLen = sizeof(PlayerPubKey);
const size_t kServerPubPkgLen = sizeof(CAPubKey);
const size_t kSignatureLen = sizeof(Signature);
const size_t kCAHeaderLen = sizeof(CAHeader);
const size_t kCABigHeaderLen = sizeof(CABigHeader);

#ifdef TEST_CA
const std::string kCAHost = "106.120.154.145";
const std::string kCAService = "808";
static const char *kBackupCAIps[] = {
    "106.120.154.145"
};
#else
const std::string kCAHost = "ca.tv.sohu.com";
const std::string kCAService = "80";
static const char *kBackupCAIps[] = {
    "220.181.119.4",
    "111.13.123.133",
    "61.135.131.77",
    "220.181.119.4",
    "111.13.123.133"
};
#endif
const size_t kBackupCAIpCount = sizeof(kBackupCAIps) / sizeof(kBackupCAIps[0]);

#endif
