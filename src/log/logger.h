#ifndef __LOGGER_H__
#define __LOGGER_H__

#include "utility/common.h"
#include "bee/base/bee_define.h"

namespace bee {

class LogManager {
public:
	static void Open(
        const std::string &path, 
        BeeLogLevel level, 
        int32_t max_line, 
        int32_t volume_count,
        int32_t volume_size,
        const std::string &prefix, 
        BeeLogCallback callback);
	static void Close();
};

////////////////////////////////////Logger//////////////////////////////////////
class Logger {
public:
	Logger(const std::string &tag);
	virtual ~Logger();

public:
    void Debug(const char* fmt, ...);
    void Info(const char* fmt, ...);
    void Trace(const char* fmt, ...);
    void Warn(const char* fmt, ...);
    void Error(const char* fmt, ...);
    void Fatal(const char* fmt, ...);
    static void Log(const std::string tag, BeeLogLevel level, const char *source, int32_t line_num, const char* fmt, ...);
    static void DT(const std::string tag, const char* fmt, ...);
    static void IF(const std::string tag, const char* fmt, ...);
    static void TR(const std::string tag, const char* fmt, ...);
    static void WA(const std::string tag, const char* fmt, ...);
    static void ER(const std::string tag, const char* fmt, ...);
    static void FT(const std::string tag, const char* fmt, ...);
    static bool IsDebugEnabled();
    static bool IsInfoEnabled();
    static bool IsTraceEnabled();
    static bool IsWarnEnabled();
    static bool IsErrorEnabled();
    static bool IsFatalEnabled();
    static long GetTid();

private:
    std::string tag_;
};

} //namespace bee

#endif // #ifndef __LOGGER_H__
