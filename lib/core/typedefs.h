#ifndef STAR_NETWORK_LIB_CORE_TYPEDEFS_H_
#define STAR_NETWORK_LIB_CORE_TYPEDEFS_H_

#define likely(x) __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)

#endif // STAR_NETWORK_LIB_CORE_TYPEDEFS_H_
