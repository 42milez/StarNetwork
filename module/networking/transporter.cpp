#include <string>

#include "core/error_macros.h"
#include "transporter.h"

Error Transporter::create_server(int port, int max_clients, int in_bandwidth, int out_bandwidth)
{
    ERR_FAIL_COND_V(_active, Error::ERR_ALREADY_IN_USE);
    ERR_FAIL_COND_V(port < 0 || port > 65535, Error::ERR_INVALID_PARAMETER);
    ERR_FAIL_COND_V(max_clients < 0, Error::ERR_INVALID_PARAMETER);
    ERR_FAIL_COND_V(in_bandwidth < 0, Error::ERR_INVALID_PARAMETER);
    ERR_FAIL_COND_V(out_bandwidth < 0, Error::ERR_INVALID_PARAMETER);

    UdpAddress address;

#ifdef P2P_TECHDEMO_IPV6
    if (_bind_ip.is_wildcard()) {
        address.wildcard = 1;
    } else {
        udp_address_set_ip(&address, bind_ip.get_ipv6(), 16);
    }
#else
    if (_bind_ip.is_wildcard()) {
        address.host = 0;
    } else {
        ERR_FAIL_COND_V(!_bind_ip.is_ipv4(), Error::ERR_INVALID_PARAMETER);
        address.host = *(uint32_t *)_bind_ip.get_ipv4();
    }
#endif
}

Transporter::Transporter()
{
    _active = false;
    _always_ordered = false;
    _bind_ip = IpAddress("*");
    _channel_count = SysCh::MAX;
    _compression_mode = CompressionMode::NONE;
    _connection_status = ConnectionStatus::DISCONNECTED;
    _current_packet.packet = nullptr;
    _refuse_connections = false;
    _server = false;
    _target_peer = 0;
    _transfer_channel = -1;
    _transfer_mode = TransferMode::RELIABLE;
    _unique_id = 0;

    _compressor.context = this;
    _compressor.compress = compress;
    _compressor.decompress = decompress;
    _compressor.destroy = compressor_destroy;
}

Transporter::~Transporter()
{
    close_connection();
}
