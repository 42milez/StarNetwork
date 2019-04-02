#include <string>

#include "core/error_macros.h"
#include "lib/udp/compress.h"
#include "transporter.h"

size_t
Transporter::_udp_compress(void *context, const UdpBuffer *in_buffer, size_t in_buffer_count, size_t in_limit, uint8_t *out_data, size_t out_limit)
{

}

size_t
Transporter::_udp_decompress(void *context, const uint8_t *in_data, size_t in_limit, uint8_t *out_data, size_t out_limit)
{

}

void
Transporter::_udp_destroy(void *context)
{
    // Nothing to do
}

void
Transporter::_setup_compressor()
{
    if (_compression_mode == CompressionMode::NONE)
    {
        udp_host_compress(_host);
    }
    else if (_compression_mode == CompressionMode::RANGE_CODER)
    {
        udp_host_compress_with_range_coder(_host);
    }
    else // FASTLZ or ZLIB or ZSTD
    {
        udp_custom_compress(_host, _udp_compressor);
    }
}

Error Transporter::create_server(uint16_t port, size_t peer_count, uint32_t in_bandwidth, uint32_t out_bandwidth)
{
    ERR_FAIL_COND_V(_active, Error::ERR_ALREADY_IN_USE);
    ERR_FAIL_COND_V(port < 0 || port > 65535, Error::ERR_INVALID_PARAMETER);
    ERR_FAIL_COND_V(peer_count < 0, Error::ERR_INVALID_PARAMETER);
    ERR_FAIL_COND_V(in_bandwidth < 0, Error::ERR_INVALID_PARAMETER);
    ERR_FAIL_COND_V(out_bandwidth < 0, Error::ERR_INVALID_PARAMETER);

    std::unique_ptr<UdpAddress> address = std::make_unique<UdpAddress>();

#ifdef P2P_TECHDEMO_IPV6
    if (_bind_ip.is_wildcard())
    {
        address.wildcard = 1;
    }
    else
    {
        udp_address_set_ip(address, _bind_ip.get_ipv6(), 16);
    }
#else
    if (!_bind_ip.is_wildcard())
    {
        ERR_FAIL_COND_V(!_bind_ip.is_ipv4(), Error::ERR_INVALID_PARAMETER);
        udp_address_set_ip(address, _bind_ip.get_ipv4(), 8);
    }
#endif

    address->port = port;

    _host = udp_host_create(std::move(address), peer_count, _channel_limit, in_bandwidth, out_bandwidth);

    ERR_FAIL_COND_V(_host == nullptr, Error::CANT_CREATE);

    _setup_compressor();

    _active = true;
    _server = true;
    _unique_id = 1;
    _connection_status = ConnectionStatus::CONNECTED;

    return Error::OK;
}

Transporter::Transporter() : _bind_ip("*")
{
    _active = false;
    _always_ordered = false;
    _channel_limit = SysCh::MAX;
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
    _compressor.compress = _udp_compress;
    _compressor.decompress = _udp_decompress;
    _compressor.destroy = _udp_destroy;
}

Transporter::~Transporter()
{
    close_connection();
}
