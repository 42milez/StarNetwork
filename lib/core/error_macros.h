#ifndef STAR_NETWORK_LIB_CORE_ERROR_MACROS_H_
#define STAR_NETWORK_LIB_CORE_ERROR_MACROS_H_

#include "logger.h"
#include "singleton.h"
#include "typedefs.h"

#define ERR_CONTINUE(cond)                                                                                             \
    if (unlikely(cond)) {                                                                                              \
        core::Singleton<core::Logger>::Instance().Critical("{0} {1} {2}", __FUNCTION__, __FILE__, __LINE__);           \
        continue;                                                                                                      \
    }

#define ERR_FAIL_COND(cond)                                                                                            \
    {                                                                                                                  \
        if (unlikely(cond)) {                                                                                          \
            return;                                                                                                    \
        }                                                                                                              \
    }

#define ERR_FAIL_COND_V(cond, retval)                                                                                  \
    {                                                                                                                  \
        if (unlikely(cond)) {                                                                                          \
            return retval;                                                                                             \
        }                                                                                                              \
    }

#define ERR_FAIL_V(retval)                                                                                             \
    {                                                                                                                  \
        return retval;                                                                                                 \
    }

// TODO: add implementation
#define ERR_PRINT(str)                                                                                                 \
    {                                                                                                                  \
    }

// TODO: add implementation
#define WARN_PRINT(str)                                                                                                \
    {                                                                                                                  \
    }

#endif // STAR_NETWORK_LIB_CORE_ERROR_MACROS_H_
