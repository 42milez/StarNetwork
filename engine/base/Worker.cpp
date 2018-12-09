#include "Worker.h"

namespace base {

  void Worker::assign(std::function<void()> fn) {
    fn_ = move(fn);
  }

  void Worker::run() {
    if (fn_) {
      thread_ = std::make_unique<std::thread>([&fn = fn_, &is_abort = is_abort_] {
        for (;;) {
          if (is_abort) break;
          fn();
        }
      });
    }
  }

  void Worker::terminate() {
    if (thread_ && thread_->joinable()) {
      is_abort_ = true;
      thread_->join();
    }
  }

} // namespace base
