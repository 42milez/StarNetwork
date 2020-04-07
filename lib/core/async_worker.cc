#include "async_worker.h"

namespace core
{
    AsyncWorker::AsyncWorker(std::function<void()> &&task)
        : deactivate_()
        , task_(task)
    {
    }
} // namespace core
