#include "buffer_pool.h"

namespace bee {

std::shared_ptr<BufferPool> BufferPool::pinst_;
std::once_flag BufferPool::once_flag_;

} // namespace bee
