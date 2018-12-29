#ifndef __BUFFERPOOL_H__
#define __BUFFERPOOL_H__

#include "common.h"

namespace bee {

const int PACKET_LEN = 1500;

class BufferPool : private boost::noncopyable {
public:
    typedef std::shared_ptr<BufferPool> p;
    static std::shared_ptr<BufferPool> inst() {
        std::call_once(once_flag_, &BufferPool::init_it);
        return pinst_;
    }

    ~BufferPool() {  
        auto iter = buffer_map_.begin();
        for (; iter != buffer_map_.end(); iter++) {
            if ((*iter).second) {
                delete []((*iter).first);
            }
        }
        buffer_map_.clear();
        free_buffer_.clear();
    }  

public:
    char *alloc() {
        std::lock_guard<std::mutex> lock(allocate_mutex_);

        char *buffer = NULL;
        if (free_buffer_.empty())
        {   
            buffer = new char[PACKET_LEN];
            buffer_map_[buffer] = true;
        } else {   
            buffer = free_buffer_.front();
            free_buffer_.pop_front();
            buffer_map_[buffer] = true;
        }   
        return buffer; 
    }

    void free(char *buffer) {
        std::lock_guard<std::mutex> lock(allocate_mutex_);
        buffer_map_[buffer] = false;
        free_buffer_.push_back(buffer);
    }

    void reduce_space() {
        std::lock_guard<std::mutex> lock(allocate_mutex_);
        std::deque<char *>::iterator iter = free_buffer_.begin();
        for (;iter != free_buffer_.end(); iter++) {
            buffer_map_.erase(*iter);
            delete [](*iter);
        }
        free_buffer_.clear();
    }

    void get_pool_info(std::pair<std::size_t,std::size_t> &info) {
        std::lock_guard<std::mutex> lock(allocate_mutex_);
        info.first = buffer_map_.size();
        info.second = free_buffer_.size();
    }

    template< typename T > struct buffer_deleter {   
        void operator ()( T const  p) {   
            BufferPool::inst()->free(p); 
        }   
    };  
private:
    BufferPool() {

    }

    static void init_it() {
        pinst_.reset(new BufferPool);
    }
private:
    std::mutex allocate_mutex_;    
    std::unordered_map<char *, bool> buffer_map_;
    std::deque<char *> free_buffer_;
    static std::shared_ptr<BufferPool> pinst_;
    static std::once_flag once_flag_;
};

} // namespace bee

#endif
