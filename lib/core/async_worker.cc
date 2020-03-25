#include "async_worker.h"

namespace core
{
    AsyncWorker::AsyncWorker(std::function<void()> &&task)
        : stopped_()
        , task_(task)
    {
    }
} // namespace core
