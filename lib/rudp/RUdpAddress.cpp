#include "RUdpAddress.h"

#include <cstring>

UdpAddress::UdpAddress() : port(0), wildcard(0)
{
    memset(&host, 0, sizeof(host));
}
