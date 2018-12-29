#ifndef __DRM_COMMON_H__
#define __DRM_COMMON_H__

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0500 // windows 2000
#endif

#ifdef WIN32
#pragma warning(disable:4996)
// boost中跨平台\n的警告
#pragma warning(disable:4819)
// MSVC 对于stdio函数的警告
#if !defined(_CRT_SECURE_NO_WARNINGS)
#define _CRT_SECURE_NO_WARNINGS
#endif
#if !defined(WIN32_LEAN_AND_MEAN)
#define WIN32_LEAN_AND_MEAN
#endif
#if !defined(NOMINMAX)
#define NOMINMAX
#endif
#include <Windows.h>
#endif

#ifdef ANDROID
#include <android/log.h>
#define BEE_LOG "bee-lib"
#define BEE_LOGD(...)  __android_log_print(ANDROID_LOG_DEBUG,BEE_LOG,__VA_ARGS__)
#define BEE_LOGI(...)  __android_log_print(ANDROID_LOG_INFO,BEE_LOG,__VA_ARGS__)
#define BEE_LOGW(...)  __android_log_print(ANDROID_LOG_WARN,BEE_LOG,__VA_ARGS__)
#define BEE_LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,BEE_LOG,__VA_ARGS__)
#define BEE_LOGF(...)  __android_log_print(ANDROID_LOG_FATAL,BEE_LOG,__VA_ARGS__)
#endif

#include <cassert>
#include <cstddef>
#include <cstdio>
#include <cstdarg>
#include <ctime>

#include <queue>
#include <deque>
#include <limits>
#include <list>
#include <memory>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <vector>
#include <future>
#include <mutex>
#include <unordered_map>

//boost
#include <boost/algorithm/string.hpp>
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/assert.hpp>
#include <boost/bind.hpp>
#include <boost/cstdint.hpp>
#include <boost/noncopyable.hpp>
#include <boost/shared_array.hpp>
#include <boost/format.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/local_time/local_time.hpp>

//openssl
#include "openssl/bn.h"
#include "openssl/dh.h"
#include "openssl/rc4.h"
#include "openssl/ssl.h"
#include "openssl/rand.h"
#include "openssl/err.h"
#include "openssl/bio.h"
#include "openssl/hmac.h"
#include "openssl/aes.h"
#include "openssl/conf.h"
#include "openssl/evp.h"
#include "openssl/sha.h"

typedef boost::int8_t int8_t;
typedef boost::int16_t int16_t;
typedef boost::int32_t int32_t;
typedef boost::int64_t int64_t;

typedef boost::uint8_t uint8_t;
typedef boost::uint16_t uint16_t;
typedef boost::uint32_t uint32_t;
typedef boost::uint64_t uint64_t;

typedef std::shared_ptr<boost::asio::io_service> IOSPtr;
typedef std::shared_ptr<boost::asio::io_service::work> IOSWorkPtr;
typedef std::shared_ptr<std::thread> ThreadPtr;
typedef boost::asio::ip::tcp::resolver::query TcpResolverQuery;
typedef std::shared_ptr<TcpResolverQuery> TcpResolverQueryPtr;

using boost::asio::ip::tcp;

namespace std {
#ifdef UNICODE
#ifndef _T
    #define _T(x) L##x
#endif
	typedef wstring tstring;
    typedef wostringstream tostringstream;
    typedef wofstream tofstream;
    typedef wifstream  tifstream;
    typedef wiostream tiostream;
    #define   _ttoi _wtoi
    typedef wchar_t  tchar_t;
	#define _tstrlen		wcslen	
#else
#ifdef _T
    #undef  _T
    #define _T(x) x
#endif
	typedef string tstring;
    typedef ostringstream tostringstream;
    typedef ofstream tofstream;
    typedef ifstream  tifstream;
    typedef iostream tiostream;
    #define _ttoi   atoi
    typedef char  tchar_t;
	#define _tstrlen		strlen	
#endif

}

#if _MSC_VER
#define snprintf sprintf_s
#endif

typedef enum SimpleLogDst {
    kSLD_Console = 0,
    kSLD_Debug,
}SimpleLogDst;

#define OffsetOfSM(s,m) (size_t) &(((s*)0)->m)
#define SizeOfSM(s,m) sizeof(((s*)0)->m)

inline std::string format_time(boost::posix_time::ptime now) {
    static std::locale loc(std::cout.getloc(), new boost::posix_time::time_facet("%Y-%m-%d %H:%M:%S"));
    std::basic_stringstream<char> ss;
    ss.imbue(loc);
    ss << now;
    return ss.str();
}

inline void simple_log(SimpleLogDst dst, const char *fmt, ...) {
    std::string strTime = std::move(format_time(boost::posix_time::microsec_clock::local_time()));
    char prefix[128] = {0};
#ifdef WIN32
    DWORD tid = GetCurrentThreadId();
#else
    pthread_t tid = pthread_self();
#endif
    int  prefix_length = sprintf(prefix, "[%s][PlayerDemo][%lu] ", strTime.c_str(), tid);
    auto msg_str = std::vector<char>(1024);
    size_t length = msg_str.size();
    memcpy(msg_str.data(), prefix, prefix_length);
    int  available_length = length - prefix_length;
    char *p = msg_str.data() + prefix_length;

    va_list ap;
    va_start(ap, fmt);
    int status = std::vsnprintf(p, available_length, fmt, ap);
    va_end(ap);

    if (status < 0) {
        throw std::runtime_error{ "string formatting error" };
    } else if (status >= available_length) {
        length = status + prefix_length + 1;
        available_length = length - prefix_length;
        msg_str.resize(length);
        p = msg_str.data() + prefix_length;
        va_list ap;
        va_start(ap, fmt);
        std::vsnprintf(p, available_length, fmt, ap);
        va_end(ap);
    }
    
#ifdef WIN32
    if (dst == kSLD_Console) {
        printf("%s",msg_str.data());
    } else if (dst == kSLD_Debug) {
        ::OutputDebugStringA(msg_str.data());
        ::OutputDebugStringA("");
    } else {

    }
#elif defined ANDROID
    BEE_LOGD("%s", msg_str.data());
#else
    printf("%s", msg_str.data());
#endif
}

inline void simple_log(SimpleLogDst dst, int level, const char *source, int line, const char *fmt, ...) {
    std::string strTime = std::move(format_time(boost::posix_time::microsec_clock::local_time()));
    char prefix[256] = { 0 };
#ifdef WIN32
    DWORD tid = GetCurrentThreadId();
#else
    pthread_t tid = pthread_self();
#endif
    int  prefix_length = sprintf(prefix, "[%d][%s][%d][%s][PlayerDemo][%lu] ", level, source == NULL ? "" : source, line, strTime.c_str(), tid);
    auto msg_str = std::vector<char>(1024);
    size_t length = msg_str.size();
    memcpy(msg_str.data(), prefix, prefix_length);
    int  available_length = length - prefix_length;
    char *p = msg_str.data() + prefix_length;

    va_list ap;
    va_start(ap, fmt);
    int status = std::vsnprintf(p, available_length, fmt, ap);
    va_end(ap);

    if (status < 0) {
        throw std::runtime_error{ "string formatting error" };
    } else if (status >= available_length) {
        length = status + prefix_length + 1;
        available_length = length - prefix_length;
        msg_str.resize(length);
        p = msg_str.data() + prefix_length;
        va_list ap;
        va_start(ap, fmt);
        std::vsnprintf(p, available_length, fmt, ap);
        va_end(ap);
    }
    
#ifdef WIN32
    if (dst == kSLD_Console) {
        printf("%s", msg_str.data());
    } else if (dst == kSLD_Debug) {
        ::OutputDebugStringA(msg_str.data());
    } else {

    }
#elif defined ANDROID
    BEE_LOGD("%s", msg_str.data());
#else
    printf("%s", msg_str.data());
#endif
}

//////////////////////////////////HTTP Params////////////////////////////////////////
const std::string kHttpRangeStr = "range";
const std::string kHttpTimeoutStr = "conn_timeout";
const std::string kHttpCacheStr = "videocache";
const std::string kManualReadBody = "manual";

///////////////////////////////////SystemParam///////////////////////////////////////
const std::string kBeeVersion = "1.0.0.1";
typedef struct SystemParam {
    int32_t         platform_type;
    int32_t         net_type;
    std::string     app_name;
    std::string     app_version;
    std::string     bee_version;
    std::string     system_info;
    std::string     machine_code;
    std::string     log_path;    
    int32_t         log_level;
    uint32_t        session_count;

    SystemParam() {
        platform_type = 0;
        net_type      = 0;
        bee_version   = kBeeVersion;
        log_level     = 4;
        session_count = 0;
    }
}SystemParam;

#endif 
