#ifndef P2P_TECHDEMO_RUDPCOMPRESS_H
#define P2P_TECHDEMO_RUDPCOMPRESS_H

#include <functional>

#include "RUdpHost.h"

int
udp_host_compress_with_range_coder(std::shared_ptr<RUdpHost> &host);

#endif // P2P_TECHDEMO_RUDPCOMPRESS_H
