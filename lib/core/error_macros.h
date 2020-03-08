#ifndef P2P_TECHDEMO_CORE_ERROR_MACROS_H
#define P2P_TECHDEMO_CORE_ERROR_MACROS_H

#include "logger.h"
#include "singleton.h"
#include "typedefs.h"

#define ERR_CONTINUE()                                                                      \
    core::Singleton<core::Logger>::Instance().Critical("__FUNCTION__, __FILE__, __LINE__"); \
	continue;

#define ERR_FAIL_COND(cond) { \
    if (unlikely(cond)) {     \
        return;               \
    }                         \
}

#define ERR_FAIL_COND_V(cond, retval) { \
    if (unlikely(cond)) {               \
        return retval;                  \
    }                                   \
}

#define ERR_FAIL_V(retval) { \
    return retval;           \
}

// TODO: add implementation
#define ERR_PRINT(str) {}

// TODO: add implementation
#define WARN_PRINT(str) {}

#endif // P2P_TECHDEMO_CORE_ERROR_MACROS_H
