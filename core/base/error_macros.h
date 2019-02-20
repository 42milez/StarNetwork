#ifndef P2P_TECHDEMO_CORE_BASE_ERROR_MACROS_H
#define P2P_TECHDEMO_CORE_BASE_ERROR_MACROS_H

#include "typedefs.h"

#define ERR_FAIL_COND(cond)  \
    {                        \
        if (unlikely(cond))  \
        {                    \
            // ToDo: logging \
            return;          \
        }                    \
    }


#define ERR_FAIL_COND_V(cond, retval) \
    {                                 \
        if (unlikely(cond)) {         \
            // ToDo: logging          \
            return retval;            \
        }                             \
    }

#define WARN_PRINT(str)                      \
    {                                        \
    // ToDo: // ToDo: Add the implementation \
    }

#endif // P2P_TECHDEMO_CORE_BASE_ERROR_MACROS_H
