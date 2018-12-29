#include "log/logger_impl.h"
#include "utility/file_util.h"
#include "utility/algorithm.h"
#include "service/bee_entrance.h"

#ifdef WIN32
#include <direct.h>
#endif

namespace bee {

LogAppenderFile::LogAppenderFile() {
#ifdef WIN32
    log_file_full_path_tag_ = ".\\";
#else
    log_file_full_path_tag_ = "./";
#endif
}

LogAppenderFile::~LogAppenderFile() {

}

void LogAppenderFile::Open(const std::string &prefix) {
    if (!prefix.empty()) {
        log_file_full_path_tag_ = FileUtil::path_cat(log_path_, prefix);
    } else {
        log_file_full_path_tag_ = FileUtil::path_cat(log_path_, "log");
    }

    if (volume_count_ > 0) {
        log_file_name_ = prefix + ".log";
        volume_index_ = GetMaxVolumeIndex(log_path_, log_file_name_);
        if (volume_index_ > volume_count_) {
            volume_index_ = volume_count_;
        }
        Open(true);
    } else {
        Open(false);
    }
}

void LogAppenderFile::Close() {
    try {
        log_file_stream_.clear();
        log_file_stream_.close();
    } catch (...) {
        printf("[FATAL] Close log file failed\n");
    }
}

void LogAppenderFile::Append(const std::string &log) {
    if (volume_count_ > 0) {
        if (volume_size_ > 0 && current_volume_size_ >= volume_size_) {
            NewLog();
            current_volume_size_ = 0;
        }
        current_volume_size_ += (int32_t)strlen(log.c_str());
    } else {
        current_lines_++;
        if (current_lines_ >= max_lines_) {
            NewLog();
            current_lines_ = 0;
        }
    }    

    if (!log_file_stream_.is_open()) {
        return;
    }

    log_file_stream_ << log.c_str() << std::flush;
}

void LogAppenderFile::SetLogFilePath(const std::string &path) {
    if (!path.empty()) {
        log_path_ = path;        
        FileUtil::create_long_directory(log_path_);
    }
}

void LogAppenderFile::SetLogFileMaxLine(int32_t max_line) {
    if (max_line < 100) {
        return;
    }
    max_lines_ = max_line;
}

void LogAppenderFile::Open(bool append) {
    std::ios_base::openmode mode = append ? (std::ios::out | std::ios::app) : (std::ios::out | std::ios::trunc);
    try {
        if (volume_count_ > 0) {
            std::string full_path = FileUtil::path_cat(log_path_, log_file_name_);
            log_file_stream_.open(full_path.c_str(), mode);
        } else {
            std::string full_path = GetFileName();
            log_file_stream_.open(full_path.c_str(), mode);
        }
    } catch (...) {
        printf("[FATAL] Open log file failed\n");
    }
}

void LogAppenderFile::NewLog() {
    if (volume_count_ > 0) {
        RollingVolumes();
    } else {
        Close();
        Open(false);
    }
}

void LogAppenderFile::SetVolumeCount(int32_t volume_count) {
    volume_count_ = (volume_count <= 0) ? 0 : volume_count;
}

void LogAppenderFile::SetVolumeSize(int32_t volume_size) {
    volume_size_ = (volume_size > 0) ? volume_size * 1024 : volume_size_;
}

int32_t LogAppenderFile::GetMaxVolumeIndex(const std::string &dir, const std::string &name) {
    int32_t count = 0;
    std::vector<std::string> v;

    if (FileUtil::list_dir(dir, v)) {
        for (auto iter = v.begin(); iter != v.end(); ++iter) {
            int32_t k = GetIndexFromVolume(name, *iter);
            if (count < k) {
                count = k;
            }
        }
    }

    return count;
}

int32_t LogAppenderFile::GetIndexFromVolume(const std::string &name, const std::string &volume_name) {
    int32_t index = 0;

    // Name matching
    if (volume_name.size() >= name.size() && 
        name.compare(volume_name.substr(0, name.size())) == 0) {
        std::string idx = volume_name.substr(name.size(), volume_name.find(".", name.size(), volume_name.size() - name.size()));
        if (!idx.empty()) {
            index = atoi(idx.substr(1).c_str());
        }
    } else {
        index = -1;
    }

    return index;
}

void LogAppenderFile::RollingVolumes() {
    // Close the streamer first
    Close();

    if (volume_count_ > 1) {
        // Delete last volume(_index) if _index > _maxIndex
        std::string old_log = log_file_name_;

        if ((volume_index_ > 0) && (volume_index_ >= volume_count_ - 1)) {
            volume_index_ = volume_count_ - 1;
            old_log += ".";
            old_log += bee_itoa(volume_index_);

            FileUtil::delete_file(FileUtil::path_cat(log_path_, old_log));
        }

         // Rename [1, _index - 1] volumes to which+1
        std::vector<std::string> v;
        FileUtil::list_dir(log_path_, v);

        // Sort the list reversely
        std::sort(v.rbegin(), v.rend(), LogFilenameCompare);

        // Rename
        for (auto iter = v.begin(); iter != v.end(); ++iter) {
            int32_t k = GetIndexFromVolume(log_file_name_, *iter);
            if (k < (volume_count_ - 1) && k >= 0) {
                // do renaming
                std::string newLog(log_file_name_);
                newLog += ".";
                newLog += bee_itoa(k + 1);

                std::string strSrc = FileUtil::path_cat(log_path_, *iter);
                std::string strDst = FileUtil::path_cat(log_path_, newLog);
                FileUtil::rename_file(strSrc, strDst);
            }
        }
    }

    Open(false);
    volume_index_++;
}

bool LogAppenderFile::LogFilenameCompare(const std::string &s1, const std::string &s2) {
    int32_t index1 = 0;
    int32_t index2 = 0;

    if (s1.empty() || s2.empty()) {
        return false;
    }

    std::vector<std::string> v;
    Split(v, s1, ".");
    if (v.size() <= 0) {
        index1 = 0;
    } else {
        if (v[v.size() - 1] == "gz") {
            index1 = atoi(FileUtil::str_trim(v[v.size() - 2]).c_str());
        } else {
            index1 = atoi(FileUtil::str_trim(v[v.size() - 1]).c_str());
        }
    }

    Split(v, s2, ".");
    if (v.size() <= 0) {
        index2 = 0;
    } else {
        if (v[v.size() - 1] == "gz") {
            index2 = atoi(FileUtil::str_trim(v[v.size() - 2]).c_str());
        } else {
            index2 = atoi(FileUtil::str_trim(v[v.size() - 1]).c_str());
        }
    }

    if (index1 >= index2) {
        return false;
    } else {
        return true;
    }
}

std::string LogAppenderFile::GetFileName() {
    static std::locale loc(std::cout.getloc(), new boost::posix_time::time_facet("%Y.%m.%d.%H-%M-%S"));
    std::basic_stringstream<char> ss;
    ss.imbue(loc);
    ss << boost::posix_time::microsec_clock::local_time();

    std::string file_name ;
    file_name = log_file_full_path_tag_;
    file_name += ".";
    file_name += ss.str();
    file_name +=  ".log";
	
    return file_name;
}

////////////////////////////////////LogAppenderNet//////////////////////////////////////
LogAppenderNet::LogAppenderNet() {

}

LogAppenderNet::~LogAppenderNet() {

}

void LogAppenderNet::Open(const std::string &prefix) {
    //Do Nothing.
}

void LogAppenderNet::Close() {
    //Do Nothing.
}

void LogAppenderNet::Append(const std::string &log) {
    BeeEntrance::instance()->write_net_log(log);
}

////////////////////////////////////LoggerImpl//////////////////////////////////////
std::shared_ptr<IOService> LoggerImpl::io_service_;
bool LoggerImpl::opened_ = false;
BeeLogLevel LoggerImpl::log_level_ = kLogLevel_All;
std::vector<LogAppender::Ptr> LoggerImpl::appenders_;

void LoggerImpl::Open(
    const std::string &path, 
    BeeLogLevel level, 
    int32_t max_line, 
    int32_t volume_count,
    int32_t volume_size,
    const std::string &prefix, 
    BeeLogCallback callback) {
    if (opened_) {
        return;
    }

    if (io_service_ == NULL) {
        io_service_.reset(new IOService);
        io_service_->start();
    }

    std::shared_ptr<std::promise<bool> > promise(new std::promise<bool>);
    io_service_->ios()->post(boost::bind(&LoggerImpl::InnerOpen, path, level, max_line, volume_count, volume_size, prefix, callback, promise));
    std::shared_future<bool> future = promise->get_future();
    opened_ = future.get();
}

void LoggerImpl::Close() {
    if (!opened_) {
        return;
    }

    opened_ = false;

    std::shared_ptr<std::promise<bool> > promise(new std::promise<bool>);
    io_service_->ios()->post(boost::bind(&LoggerImpl::InnerClose, promise));
    std::shared_future<bool> future = promise->get_future();
    future.get();

    if (io_service_ != NULL) {
        io_service_->stop();
        io_service_.reset();
    }
}

void LoggerImpl::Append(const std::string& log) {
    if (!opened_) {
        return;
    }

    io_service_->ios()->post(boost::bind(&LoggerImpl::InnerAppend, log));
}

void LoggerImpl::InnerOpen(
    const std::string &path, 
    BeeLogLevel level, 
    int32_t max_line, 
    int32_t volume_count,
    int32_t volume_size,
    const std::string &prefix, 
    BeeLogCallback callback, 
    std::shared_ptr<std::promise<bool> > promise) {
    appenders_.clear();
    log_level_ = level;

#ifndef ANDROID
    LogAppenderConsole::Ptr lac(new LogAppenderConsole);
    lac->Open(prefix);
    appenders_.push_back(lac);
#else
    LogAppenderLogcat::Ptr lal(new LogAppenderLogcat);
    lal->Open(prefix);
    appenders_.push_back(lal);
#endif

#ifdef WIN32
    LogAppenderDebug::Ptr lad(new LogAppenderDebug);
    lad->Open(prefix);
    appenders_.push_back(lad);
#endif

    //Net Log
    LogAppenderNet::Ptr lan(new LogAppenderNet);
    lan->Open(prefix);
    appenders_.push_back(lan);

    //If callback is set, disable file.
    if (callback != NULL) {
        LogAppenderCallback::Ptr lacb(new LogAppenderCallback);
        lacb->SetLogCallback(callback);
        lacb->Open(prefix);
        appenders_.push_back(lacb);
    } else {
        LogAppenderFile::Ptr laf(new LogAppenderFile);
        laf->SetLogFilePath(path);
        laf->SetLogFileMaxLine(max_line);
        laf->SetVolumeCount(volume_count);
        laf->SetVolumeSize(volume_size);
        laf->Open(prefix);
        appenders_.push_back(laf);
    }

    promise->set_value(true);
}

void LoggerImpl::InnerClose(std::shared_ptr<std::promise<bool> > promise) {
    for (auto apender : appenders_) {
        apender->Close();
    }
    appenders_.clear();
    promise->set_value(true);
}

void LoggerImpl::InnerAppend(const std::string &log) {
    for (auto apender : appenders_) {
        apender->Append(log);
    }
}

} //namespace bee
