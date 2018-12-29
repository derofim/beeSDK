#ifndef __LOGGER_IMPL_H__
#define __LOGGER_IMPL_H__

#include "utility/common.h"
#include "bee/base/bee_define.h"
#include "network/io_service.h"

#ifdef ANDROID
#include <android/log.h>
#endif

namespace bee {

////////////////////////////////////LogAppender//////////////////////////////////////
class LogAppender {
public:
    typedef std::shared_ptr<LogAppender> Ptr;
    LogAppender() {}
    virtual ~LogAppender() {}

public:
	virtual void Open(const std::string &prefix) = 0;
	virtual void Close() = 0;
	virtual void Append(const std::string &log)=0;
};

///////////////////////////////////LogAppenderConsole///////////////////////////////////////
class LogAppenderConsole : public LogAppender {
public:
    typedef std::shared_ptr<LogAppenderConsole> Ptr;
    LogAppenderConsole() {}
    virtual ~LogAppenderConsole() {}

public:
    virtual void Open(const std::string &prefix) {}
    virtual void Close() {}
    virtual void Append(const std::string &log) { printf("%s", log.c_str()); }
};

////////////////////////////////////LogAppenderCallback//////////////////////////////////////
class LogAppenderCallback : public LogAppender {
public:
    typedef std::shared_ptr<LogAppenderCallback> Ptr;
    LogAppenderCallback() {}
    virtual ~LogAppenderCallback() {}

public:
    virtual void Open(const std::string &prefix) {}
    virtual void Close() {}
    virtual void Append(const std::string &log) { if (callback_ != NULL) callback_(log.c_str()); }
    void SetLogCallback(BeeLogCallback callback) { callback_ = callback; }

private:
    BeeLogCallback callback_ = NULL;
};

#ifdef WIN32
///////////////////////////////////LogAppenderDebug///////////////////////////////////////
class LogAppenderDebug : public LogAppender {
public:
    typedef std::shared_ptr<LogAppenderDebug> Ptr;
    LogAppenderDebug() {}
    virtual ~LogAppenderDebug() {}

public:
    virtual void Open(const std::string &prefix) {}
    virtual void Close() {}
    virtual void Append(const std::string &log) {
        ::OutputDebugStringA(log.data());
    }
};
#endif // #ifdef WIN32

#ifdef ANDROID
///////////////////////////////////LogAppenderLogcat///////////////////////////////////////
class LogAppenderLogcat : public LogAppender {
public:
    typedef std::shared_ptr<LogAppenderLogcat> Ptr;
    LogAppenderLogcat() {}
    virtual ~LogAppenderLogcat() {}

public:
    virtual void Open(const std::string &prefix) {}
    virtual void Close() {}
    virtual void Append(const std::string &log) {
        __android_log_print(ANDROID_LOG_DEBUG, "bee", "%s", log.c_str());
    }
};
#endif // #ifdef ANDROID

//////////////////////////////////LogAppenderFile////////////////////////////////////////
class LogAppenderFile : public LogAppender {
public:
    typedef std::shared_ptr<LogAppenderFile> Ptr;
    LogAppenderFile();
    virtual ~LogAppenderFile();

public:
    virtual void Open(const std::string &prefix);
    virtual void Close();
    virtual void Append(const std::string &log);

public:
	void SetLogFilePath(const std::string &path);
	void SetLogFileMaxLine(int32_t max_line);
    void Open(bool append = false);
    void NewLog();
    void SetVolumeCount(int32_t volume_count);
    void SetVolumeSize(int32_t volume_size);
    int32_t GetMaxVolumeIndex(const std::string &dir, const std::string &name);
    int32_t GetIndexFromVolume(const std::string &name, const std::string &volume_name);
    void RollingVolumes();
    static bool LogFilenameCompare(const std::string &s1, const std::string &s2);
    std::string GetFileName();

private:
	std::ofstream log_file_stream_;
    int32_t current_lines_ = 0;
    int32_t max_lines_ = 100 * 1024;
    std::string log_file_full_path_tag_;
    std::string log_path_;
    std::string log_file_name_;
    int32_t volume_count_ = 0;
    int32_t volume_size_ = 10 * 1024 * 1024; //Default to 10M.
    int32_t current_volume_size_ = 0;
    int32_t volume_index_ = 0;
};

///////////////////////////////////LogAppenderNet///////////////////////////////////////
class LogAppenderNet : public LogAppender {
public:
    typedef std::shared_ptr<LogAppenderNet> Ptr;
    LogAppenderNet();
    virtual ~LogAppenderNet();

public:
    virtual void Open(const std::string &prefix);
    virtual void Close();
    virtual void Append(const std::string &log);
};

///////////////////////////////////LoggerImpl///////////////////////////////////////
class LoggerImpl {
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
    static void Append(const std::string &log);
    static BeeLogLevel GetLogLevel() { return log_level_; }

private:
    static void InnerOpen(
        const std::string &path, 
        BeeLogLevel level, 
        int32_t max_line, 
        int32_t volume_count,
        int32_t volume_size,
        const std::string &prefix, 
        BeeLogCallback callback, 
        std::shared_ptr<std::promise<bool> > promise);
    static void InnerClose(std::shared_ptr<std::promise<bool> > promise);
    static void InnerAppend(const std::string &log);

private:
    static std::shared_ptr<IOService> io_service_;
    static bool opened_;
    static BeeLogLevel log_level_;
    static std::vector<LogAppender::Ptr> appenders_;
};

} //namespace bee

#endif // #ifndef __LOGGER_IMPL_H__
