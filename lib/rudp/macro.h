#ifndef P2P_TECHDEMO_RUDPCOMMON_H
#define P2P_TECHDEMO_RUDPCOMMON_H

#include <cstdint>

namespace rudp
{
#define UDP_TIME_OVERFLOW 86400000 // msec per day (60 sec * 60 sec * 24 h * 1000)

// TODO: change argument order as the expression would be "A is less than B"
#define UDP_TIME_LESS(a, b) ((a) - (b) >= UDP_TIME_OVERFLOW)
#define UDP_TIME_GREATER(a, b) ((b) - (a) >= UDP_TIME_OVERFLOW)
#define UDP_TIME_LESS_EQUAL(a, b) (!UDP_TIME_GREATER(a, b))
#define UDP_TIME_GREATER_EQUAL(a, b) (!UDP_TIME_LESS(a, b))
#define UDP_TIME_DIFFERENCE(a, b) ((a) - (b) >= UDP_TIME_OVERFLOW ? (b) - (a) : (a) - (b))
} // namespace rudp

#endif // P2P_TECHDEMO_RUDPCOMMON_H
