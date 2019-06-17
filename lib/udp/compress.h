#ifndef P2P_TECHDEMO_LIB_UDP_COMPRESS_H
#define P2P_TECHDEMO_LIB_UDP_COMPRESS_H

#include <functional>

#include "Rudp.h"

int
udp_host_compress_with_range_coder(std::shared_ptr<UdpHost> &host);

#endif // P2P_TECHDEMO_LIB_UDP_COMPRESS_H
