#ifndef LIFE_EXIT_HANDLER_H
#define LIFE_EXIT_HANDLER_H

#include <csignal>

namespace engine
{
  namespace util
  {
    class ExitHandler {
    public:
      static void init();

      static void exit();

      bool should_exit() const;

    private:
      template<typename Func>
      static void register_handler(int signum, Func handler);

      static bool should_exit_;
    };
  }
}

#endif // LIFE_EXIT_HANDLER_H
