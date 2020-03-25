#ifndef P2P_TECHDEMO_LIB_CORE_ASYNC_WORKER_H_
#define P2P_TECHDEMO_LIB_CORE_ASYNC_WORKER_H_

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
            thread_ = std::thread([this]{
              auto id = std::this_thread::get_id();
              std::stringstream ss;
              ss << id;
              core::Singleton<core::Logger>::Instance().Debug("worker started (thread {0})", ss.str());
              while (!deactivate_) {
                task_();
              }
              core::Singleton<core::Logger>::Instance().Debug("worker stopped (thread {0})", ss.str());
            });
        };

        inline void
        Stop()
        {
            deactivate_ = true;

            if (thread_.joinable()) {
                try {
                    thread_.join();
                    core::Singleton<core::Logger>::Instance().Debug("worker joined");
                }
                catch (std::system_error& e) {
                    const std::error_code& ec = e.code();
                    core::Singleton<core::Logger>::Instance().Debug("join failed");
                    core::Singleton<core::Logger>::Instance().Debug("code: {0}", e.code().value());
                    core::Singleton<core::Logger>::Instance().Debug("reason: {0}", e.what());
                }
            }
        }

      private:
        std::atomic<bool> deactivate_;
        std::function<void()> task_;
        std::thread thread_;
    };
} // namespace core

#endif // P2P_TECHDEMO_LIB_CORE_ASYNC_WORKER_H_
