#ifndef P2P_TECHDEMO_CORE_BASE_ERROR_MACROS_H
#define P2P_TECHDEMO_CORE_BASE_ERROR_MACROS_H

#include "typedefs.h"

// ToDo: logging
#define ERR_FAIL_COND(cond)  \
    {                        \
        if (unlikely(cond))  \
        {                    \
            return;          \
        }                    \
    }

// ToDo: logging
#define ERR_FAIL_COND_V(cond, retval) \
    {                                 \
        if (unlikely(cond)) {         \
            return retval;            \
        }                             \
    }

// ToDo: // ToDo: Add the implementation
#define WARN_PRINT(str) \
    {}                  \

#endif // P2P_TECHDEMO_CORE_BASE_ERROR_MACROS_H
