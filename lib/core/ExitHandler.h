#ifndef P2P_TECHDEMO_LIB_CORE_EXIT_HANDLER_H_
#define P2P_TECHDEMO_LIB_CORE_EXIT_HANDLER_H_

#include <csignal>

namespace core
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
}

#endif // P2P_TECHDEMO_LIB_CORE_EXIT_HANDLER_H_
