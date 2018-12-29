#include "buffer_pool.h"

std::shared_ptr<BufferPool> BufferPool::pinst_;
std::once_flag BufferPool::once_flag_;
