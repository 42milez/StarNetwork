#ifndef P2P_TECHDEMO_LIB_CORE_ASYNC_WORKER_H_
#define P2P_TECHDEMO_LIB_CORE_ASYNC_WORKER_H_

#include <atomic>
#include <functional>
#include <mutex>
#include <thread>

namespace core
{
    class AsyncWorker
    {
      public:
        AsyncWorker();

        inline void
        Run(std::function<void()> &&task)
        {
            thread_ = std::thread(task);
        };

        inline void
        Stop()
        {
            canceled_ = true;
            thread_.join();
        }

      private:
        std::atomic<bool> canceled_;
        std::mutex mutex_;
        std::thread thread_;
    };
} // namespace core

#endif // P2P_TECHDEMO_LIB_CORE_ASYNC_WORKER_H_
