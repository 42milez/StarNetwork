#ifndef P2P_TECHDEMO_ENGINE_BASE_EXIT_HANDLER_H
#define P2P_TECHDEMO_ENGINE_BASE_EXIT_HANDLER_H

#include <csignal>

namespace engine
{
  namespace base
  {
    enum class REGISTER_HANDLER_STATUS {
      SUCCESS = 0,
      FAIL = -1
    };

    class ExitHandler {
    public:
      ExitHandler();

      bool init();

      void exit();

      bool should_exit() const;

    private:
      template<typename Func>
      REGISTER_HANDLER_STATUS register_handler(int signum, Func handler);

      bool should_exit_;
    };
  } // namespace base
} // namespace engine

#endif // P2P_TECHDEMO_ENGINE_BASE_EXIT_HANDLER_H
