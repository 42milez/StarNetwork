#ifndef P2P_TECHDEMO_RUDPCOMPRESS_H
#define P2P_TECHDEMO_RUDPCOMPRESS_H

#include <functional>

#include "RUdpCommon.h"

int
udp_host_compress_with_range_coder(std::shared_ptr<UdpHost> &host);

#endif // P2P_TECHDEMO_RUDPCOMPRESS_H
