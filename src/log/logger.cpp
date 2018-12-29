#include "log/logger.h"
#include "log/logger_impl.h"

namespace bee {

////////////////////////////////////LogManager//////////////////////////////////////
void LogManager::Open(
    const std::string &path, 
    BeeLogLevel level, 
    int32_t max_line,
    int32_t volume_count,
    int32_t volume_size,
    const std::string &prefix, 
    BeeLogCallback callback) {
    LoggerImpl::Open(path, level, max_line, volume_count, volume_size, prefix, callback);
}

void LogManager::Close() {
    LoggerImpl::Close();
}

///////////////////////////////////Logger///////////////////////////////////////
Logger::Logger(const std::string &tag) : tag_(tag) {

}

Logger::~Logger() {

}

#define GENERAL_LOG_FORMAT(X) \
    std::string time_string = format_time(boost::posix_time::microsec_clock::local_time()); \
    long tid = GetTid(); \
    size_t msg_length = 1024; \
    std::string msg_str = std::string(msg_length, 0); \
    int32_t prefix_length = sprintf((char*)msg_str.data(), "[%s][%s][%s][%ld] ", time_string.c_str(), tag_.c_str(), BeeLogLevelStr[X], tid); \
    int32_t available_length = msg_length - prefix_length; \
    if (available_length <= 0) { \
        throw std::runtime_error{ "Too long log prefix." }; \
    } \
    char *p = (char*)msg_str.data() + prefix_length; \
    va_list ap; \
    va_start(ap, fmt); \
    int32_t status = std::vsnprintf(p, available_length, fmt, ap); \
    va_end(ap); \
    if (status < 0) { \
        throw std::runtime_error{ "string formatting error" }; \
    } else if (status >= available_length) { \
        msg_length = status + prefix_length + 1; \
        available_length = msg_length - prefix_length; \
        msg_str.resize(msg_length, 0); \
        p = (char*)msg_str.data() + prefix_length; \
        va_list ap; \
        va_start(ap, fmt); \
        std::vsnprintf(p, available_length, fmt, ap); \
        va_end(ap); \
    } \
    LoggerImpl::Append(msg_str);

#define GENERAL_LOG_FORMAT_T(T, X) \
    std::string time_string = format_time(boost::posix_time::microsec_clock::local_time()); \
    long tid = GetTid(); \
    size_t msg_length = 1024; \
    std::string msg_str = std::string(msg_length, 0); \
    int32_t prefix_length = sprintf((char*)msg_str.data(), "[%s][%s][%s][%ld] ", time_string.c_str(), T.c_str(), BeeLogLevelStr[X], tid); \
    int32_t available_length = msg_length - prefix_length; \
    if (available_length <= 0) { \
        throw std::runtime_error{ "Too long log prefix." }; \
    } \
    char *p = (char*)msg_str.data() + prefix_length; \
    va_list ap; \
    va_start(ap, fmt); \
    int32_t status = std::vsnprintf(p, available_length, fmt, ap); \
    va_end(ap); \
    if (status < 0) { \
        throw std::runtime_error{ "string formatting error" }; \
    } else if (status >= available_length) { \
        msg_length = status + prefix_length + 1; \
        available_length = msg_length - prefix_length; \
        msg_str.resize(msg_length, 0); \
        p = (char*)msg_str.data() + prefix_length; \
        va_list ap; \
        va_start(ap, fmt); \
        std::vsnprintf(p, available_length, fmt, ap); \
        va_end(ap); \
    } \
    LoggerImpl::Append(msg_str);

#define GENERAL_LOG_FORMAT_TSL(T, X, S, L) \
    std::string time_string = format_time(boost::posix_time::microsec_clock::local_time()); \
    long tid = GetTid(); \
    size_t msg_length = 1024; \
    std::string msg_str = std::string(msg_length, 0); \
    int32_t prefix_length = sprintf((char*)msg_str.data(), "[%s][%s][%s][%ld][%s:%d] ", time_string.c_str(), T.c_str(), BeeLogLevelStr[X], tid, S, L); \
    int32_t available_length = msg_length - prefix_length; \
    if (available_length <= 0) { \
        throw std::runtime_error{ "Too long log prefix." }; \
    } \
    char *p = (char*)msg_str.data() + prefix_length; \
    va_list ap; \
    va_start(ap, fmt); \
    int32_t status = std::vsnprintf(p, available_length, fmt, ap); \
    va_end(ap); \
    if (status < 0) { \
        throw std::runtime_error{ "string formatting error" }; \
    } else if (status >= available_length) { \
        msg_length = status + prefix_length + 1; \
        available_length = msg_length - prefix_length; \
        msg_str.resize(msg_length, 0); \
        p = (char*)msg_str.data() + prefix_length; \
        va_list ap; \
        va_start(ap, fmt); \
        std::vsnprintf(p, available_length, fmt, ap); \
        va_end(ap); \
    } \
    LoggerImpl::Append(msg_str);

void Logger::Debug(const char* fmt, ...) {
    if (!IsDebugEnabled()) {
        return;
    }
    GENERAL_LOG_FORMAT(kLogLevel_Debug);
}

void Logger::Info(const char* fmt, ...) {
    if (!IsInfoEnabled()) {
        return;
    }
	GENERAL_LOG_FORMAT(kLogLevel_Info);
}

void Logger::Trace(const char* fmt, ...) {
    if (!IsTraceEnabled()) {
        return;
    }
	GENERAL_LOG_FORMAT(kLogLevel_Trace);
}

void Logger::Warn(const char* fmt, ...) {
    if (!IsWarnEnabled()) {
        return;
    }
	GENERAL_LOG_FORMAT(kLogLevel_Warn);
}

void Logger::Error(const char* fmt, ...) {
    if (!IsErrorEnabled()) {
        return;
    }
	GENERAL_LOG_FORMAT(kLogLevel_Error);
}

void Logger::Fatal(const char* fmt, ...) {
    if (!IsFatalEnabled()) {
        return;
    }
	GENERAL_LOG_FORMAT(kLogLevel_Fatal);
}

void Logger::Log(const std::string tag, BeeLogLevel level, const char *source, int32_t line_num, const char* fmt, ...) {
    switch (level) {
    case kLogLevel_Fatal: {
            if (!IsFatalEnabled()) {
                return;
            }
            GENERAL_LOG_FORMAT_TSL(tag, kLogLevel_Fatal, source, line_num);
            break;
        }
    case kLogLevel_Error: {
            if (!IsErrorEnabled()) {
                return;
            }
            GENERAL_LOG_FORMAT_TSL(tag, kLogLevel_Error, source, line_num);
            break; 
        }
    case kLogLevel_Warn: {
            if (!IsWarnEnabled()) {
                return;
            }
            GENERAL_LOG_FORMAT_TSL(tag, kLogLevel_Warn, source, line_num);
            break;
        }
    case kLogLevel_Trace: {
            if (!IsTraceEnabled()) {
                return;
            }
            GENERAL_LOG_FORMAT_TSL(tag, kLogLevel_Trace, source, line_num);
            break;
        }
    case kLogLevel_Info: {
            if (!IsInfoEnabled()) {
                return;
            }
            GENERAL_LOG_FORMAT_TSL(tag, kLogLevel_Info, source, line_num);
            break;
        }
    case kLogLevel_Debug: 
    default: {
            if (!IsDebugEnabled()) {
                return;
            }
            GENERAL_LOG_FORMAT_TSL(tag, kLogLevel_Debug, source, line_num);
            break;
        }
    }
}

void Logger::DT(const std::string tag, const char* fmt, ...) {
    if (!IsDebugEnabled()) {
        return;
    }
    GENERAL_LOG_FORMAT_T(tag, kLogLevel_Debug);
}

void Logger::IF(const std::string tag, const char* fmt, ...) {
    if (!IsInfoEnabled()) {
        return;
    }
    GENERAL_LOG_FORMAT_T(tag, kLogLevel_Info);
}

void Logger::TR(const std::string tag, const char* fmt, ...) {
    if (!IsTraceEnabled()) {
        return;
    }
    GENERAL_LOG_FORMAT_T(tag, kLogLevel_Trace);
}

void Logger::WA(const std::string tag, const char* fmt, ...) {
    if (!IsWarnEnabled()) {
        return;
    }
    GENERAL_LOG_FORMAT_T(tag, kLogLevel_Warn);
}

void Logger::ER(const std::string tag, const char* fmt, ...) {
    if (!IsErrorEnabled()) {
        return;
    }
    GENERAL_LOG_FORMAT_T(tag, kLogLevel_Error);
}

void Logger::FT(const std::string tag, const char* fmt, ...) {
    if (!IsFatalEnabled()) {
        return;
    }
    GENERAL_LOG_FORMAT_T(tag, kLogLevel_Fatal);
}

bool Logger::IsDebugEnabled() {
	return (LoggerImpl::GetLogLevel() >= kLogLevel_Debug);
}

bool Logger::IsInfoEnabled() {
	return (LoggerImpl::GetLogLevel() >= kLogLevel_Info);
}

bool Logger::IsTraceEnabled() {
	return (LoggerImpl::GetLogLevel() >= kLogLevel_Trace);
}

bool Logger::IsWarnEnabled() {
	return (LoggerImpl::GetLogLevel() >= kLogLevel_Warn);
}

bool Logger::IsErrorEnabled() {
	return (LoggerImpl::GetLogLevel() >= kLogLevel_Error);
}

bool Logger::IsFatalEnabled() {
	return (LoggerImpl::GetLogLevel() >= kLogLevel_Fatal);
}

long Logger::GetTid() {
#ifdef WIN32
    DWORD tid = GetCurrentThreadId();
#elif defined(IOS)
    __uint64_t tid = 0;
    if (pthread_threadid_np(0, &tid)) {
        tid = pthread_mach_thread_np(pthread_self());
    }
#else
    pthread_t tid = pthread_self();
#endif
    return (long)tid;
}

} //namespace bee
