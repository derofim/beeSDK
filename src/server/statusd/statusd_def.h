#ifndef __STATUSD_DEFINE_H__
#define __STATUSD_DEFINE_H__

#include "utility/common.h"

#pragma pack(1)

typedef enum StatusdState {
    eStatusdState_Resolve = 0,
    eStatusdState_Connect,
    eStatusdState_Login,
    eStatusdState_WriteLog,
    eStatusdState_Rcv
}StatusdState;

#define LIVE_MARK                           0x32505553
#define SETUP_PROTOCOL_HEADER(p,len,code)   { (p)->header.pkg_len=(len);(p)->header.pkg_mark=LIVE_MARK;(p)->header.pkg_code=(code);}

#define STATUSD_LOGIN                       0xA00
#define STATUSD_RESET_SID                   0xA01
#define STATUSD_REPORT_LOG                  0xA02
#define STATUSD_LOG_TYPE                    0x2710

#pragma pack(1)

typedef struct PkgHeader {
    uint32_t pkg_len = 0;
    uint32_t pkg_mark = 0;
    uint16_t pkg_code = 0;
} PkgHeader;

typedef struct PkgAppLogin {
    PkgHeader header;
    char uid[256];
    char sid[256];
} PkgAppLogin;

typedef struct PkgResetSID {
    PkgHeader header;
    char sid[256];
} PkgResetSID;

typedef struct PkgAppLog {
    PkgHeader header;
    uint32_t log_type = 0;
    uint64_t log_time = 0;
    char log_msg[0];
} PkgAppLog;

#pragma pack()

const size_t kStatusdHeaderLen = sizeof(PkgHeader);
const size_t kPkgAppLoginLen = sizeof(PkgAppLogin);
const size_t kPkgResetSIDLen = sizeof(PkgResetSID);
const size_t kPkgAppLogLen = sizeof(PkgAppLog);

const std::string kStatusHost = "106.120.154.76";
const std::string kStatusPort = "8080";
const int32_t kStatusdConnectTimeout = 5000;
const int32_t kStatusHeartbeatInterval = 30000;

#endif
