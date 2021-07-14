#ifndef STAR_NETWORK_LIB_CORE_ASYNC_WORKER_H_
#define STAR_NETWORK_LIB_CORE_ASYNC_WORKER_H_

#include <atomic>
#include <functional>
#include <sstream>
#include <string>
#include <thread>

#include "logger.h"
#include "singleton.h"

namespace core
{
    class AsyncWorker
    {
      public:
        explicit AsyncWorker(std::function<void()> &&task);

        inline void
        Run()
        {
            thread_ = std::thread([this] {
                auto id = std::this_thread::get_id();
                std::stringstream ss;
                ss << id;
                LOG_DEBUG_VA("worker started (thread {0})", ss.str());
                while (!deactivate_) {
                    task_();
                }
                LOG_DEBUG_VA("worker stopped (thread {0})", ss.str());
            });
        };

        inline void
        Stop()
        {
            deactivate_ = true;

            if (thread_.joinable()) {
                try {
                    thread_.join();
                    LOG_DEBUG("worker joined");
                }
                catch (std::system_error &e) {
                    LOG_DEBUG("join failed");
                    LOG_DEBUG_VA("code: {0}", e.code().value());
                    LOG_DEBUG_VA("reason: {0}", e.what());
                }
            }
        }

      private:
        std::atomic<bool> deactivate_;
        std::function<void()> task_;
        std::thread thread_;
    };
} // namespace core

#endif // STAR_NETWORK_LIB_CORE_ASYNC_WORKER_H_
