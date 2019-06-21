#include <string>

#include "core/error_macros.h"
#include "core/hash.h"
#include "core/singleton.h"
#include "core/string.h"
#include "core/io/compression.h"

#include "lib/rudp/RUdpAddress.h"

#include "Network.h"

Network::Segment::Segment()
    : segment(nullptr), from(0), channel(0)
{}

size_t
Network::Compressor(const std::vector<RUdpBuffer> &in_buffers, size_t in_limit, uint8_t *out_data, size_t out_limit)
{
    if (src_compressor_mem_.size() < in_limit)
        src_compressor_mem_.resize(in_limit);

    auto total = in_limit;
    auto offset = 0;
    auto in_buffer_size = in_buffers.size();

    while (total) {
        for (auto i = 0; i < in_buffer_size; i++) {
            auto to_copy = std::min(total, in_buffers[i].data_length);
            memcpy(&src_compressor_mem_[offset], in_buffers[i].data, to_copy);
            offset += to_copy;
            total -= to_copy;
        }
    }

    Compression::Mode mode;

    if (compression_mode_ == CompressionMode::ZSTD) {
        mode = Compression::Mode::ZSTD;
    }
    else if (compression_mode_ == CompressionMode::ZLIB) {
        mode = Compression::Mode::DEFLATE;
    }
    else {
        ERR_FAIL_V(0)
    }

    auto req_size = Compression::get_max_compressed_buffer_size(offset, mode);

    if (dst_compressor_mem_.size() < req_size)
        dst_compressor_mem_.resize(req_size);

    auto ret = Compression::compress(dst_compressor_mem_, src_compressor_mem_, mode);

    if (ret < 0)
        return 0; // TODO: Is -1 better?

    if (ret > out_limit)
        return 0; // TODO: Is -1 better?

    out_data.resize(dst_compressor_mem_.size());
    out_data = dst_compressor_mem_;

    src_compressor_mem_.clear();
    dst_compressor_mem_.clear();

    return ret;
}

size_t
Network::Decompressor(const uint8_t *in_data, size_t in_limit, uint8_t *out_data, size_t out_limit)
{
    auto ret = -1;

    if (compression_mode_ == CompressionMode::ZLIB) {
        ret = Compression::decompress(out_data, out_limit, in_data, Compression::Mode::DEFLATE);
    }
    else if (compression_mode_ == CompressionMode::ZSTD) {
        // ...
    }

    if (ret < 0)
        return 0;

    return ret;
}

void
Network::_udp_destroy()
{
    // Nothing to do
}

void
Network::_setup_compressor()
{
    if (compression_mode_ == CompressionMode::NONE) {
        //udp_host_compress(_host);
    }
    else if (compression_mode_ == CompressionMode::RANGE_CODER) {
        //udp_host_compress_with_range_coder(_host);
    }
    else // FASTLZ or ZLIB or ZSTD
    {
        //udp_custom_compress(_host, _udp_compressor);
    }
}

Error
Network::create_server(uint16_t port, size_t peer_count, uint32_t in_bandwidth, uint32_t out_bandwidth)
{
    ERR_FAIL_COND_V(active_, Error::ERR_ALREADY_IN_USE)
    ERR_FAIL_COND_V(port < 0 || port > 65535, Error::ERR_INVALID_PARAMETER)
    ERR_FAIL_COND_V(peer_count < 0, Error::ERR_INVALID_PARAMETER)
    ERR_FAIL_COND_V(in_bandwidth < 0, Error::ERR_INVALID_PARAMETER)
    ERR_FAIL_COND_V(out_bandwidth < 0, Error::ERR_INVALID_PARAMETER)

    RUdpAddress address;

#ifdef P2P_TECHDEMO_IPV6
    if (bind_ip_.is_wildcard())
    {
        address->wildcard = 1;
    }
    else
    {
        address.set_ip(bind_ip_.get_ipv6(), 16);
    }
#else
    if (!bind_ip_.is_wildcard()) {
        ERR_FAIL_COND_V(!bind_ip_.is_ipv4(), Error::ERR_INVALID_PARAMETER)
        address.set_ip(bind_ip_.get_ipv4(), 8);
    }
#endif

    address.port = port;

    _host = std::make_unique<RUdpHost>(address, channel_count_, peer_count, in_bandwidth, out_bandwidth);

    ERR_FAIL_COND_V(_host == nullptr, Error::CANT_CREATE)

    _setup_compressor();

    active_ = true;
    server_ = true;
    _connection_status = ConnectionStatus::CONNECTED;

    unique_id_ = hash32();

    return Error::OK;
}

Error
Network::create_client(const std::string &address, int port, int in_bandwidth, int out_bandwidth, int client_port)
{
    ERR_FAIL_COND_V(active_, Error::ERR_ALREADY_IN_USE)
    ERR_FAIL_COND_V(port < 0 || port > 65535, Error::ERR_INVALID_PARAMETER)
    ERR_FAIL_COND_V(client_port < 0 || client_port > 65535, Error::ERR_INVALID_PARAMETER)
    ERR_FAIL_COND_V(in_bandwidth < 0, Error::ERR_INVALID_PARAMETER)
    ERR_FAIL_COND_V(out_bandwidth < 0, Error::ERR_INVALID_PARAMETER)

    if (client_port != 0) {
        RUdpAddress client_address;

#ifdef P2P_TECHDEMO_IPV6
        if (bind_ip_.is_wildcard())
        {
            client->wildcard = 1;
        }
        else
        {
            address.set_ip(client, bind_ip_.get_ipv6(), 16);
        }
#else
        if (!bind_ip_.is_wildcard()) {
            ERR_FAIL_COND_V(!bind_ip_.is_ipv4(), Error::ERR_INVALID_PARAMETER)
            client_address.set_ip(bind_ip_.get_ipv4(), 8);
        }
#endif
        client_address.port = client_port;

        _host = std::make_unique<RUdpHost>(client_address, channel_count_, 1, in_bandwidth, out_bandwidth);
    }
    else {
        // create a host with random assigned port
        // ...
    }

    ERR_FAIL_COND_V(!_host, Error::CANT_CREATE)

    _setup_compressor();

    IpAddress ip;

    if (is_valid_ip_address(address)) {
        ip = IpAddress(address);
    }
    else {
#ifdef P2P_TECHDEMO_IPV6
        ip = Singleton<IP>::Instance().resolve_hostname(address);
#else
        ip = Singleton<IP>::Instance().resolve_hostname(address, IP::Type::V4);
#endif
        ERR_FAIL_COND_V(!ip.is_valid(), Error::CANT_CREATE)
    }

    RUdpAddress udp_address;

#ifdef P2P_TECHDEMO_IPV6
    address.set_ip(ip.get_ipv6(), 16);
#else
    ERR_FAIL_COND_V(!ip.is_ipv4(), Error::ERR_INVALID_PARAMETER)
    memcpy(udp_address.host, ip.get_ipv4(), sizeof(udp_address.host));
#endif
    udp_address.port = port;

    unique_id_ = hash32();

    return _host->udp_host_connect(udp_address, channel_count_, unique_id_);
}

void
Network::close_connection(uint32_t wait_usec)
{
    // ...
}

void
Network::_pop_current_segment()
{
    // ...
}

void
Network::poll()
{
    ERR_FAIL_COND(!active_)

    _pop_current_segment();

    std::unique_ptr<RUdpEvent> event;

    while (true) {
        if (!_host || !active_)
            return;

        int ret = _host->udp_host_service(event, 0);

        if (ret <= 0)
            break;

        if (event->type == RUdpEventType::CONNECT) {
            // ...
        }
        else if (event->type == RUdpEventType::DISCONNECT) {
            // ...
        }
        else if (event->type == RUdpEventType::RECEIVE) {
            // ...
        }
        else if (event->type == RUdpEventType::NONE) {
            // ...
        }
    }
}

Network::Network()
    : active_(false),
      always_ordered_(false),
      bind_ip_("*"),
      channel_count_(SysCh::MAX),
      compression_mode_(CompressionMode::NONE),
      _connection_status(ConnectionStatus::DISCONNECTED),
      _current_segment(Segment{}),
      _refuse_connections(false),
      server_(false),
      _target_peer(0),
      _transfer_channel(-1),
      _transfer_mode(TransferMode::RELIABLE),
      unique_id_(0)
{
    _compressor->compress = [this](const std::vector<RUdpBuffer> &in_buffers,
                                   size_t in_limit,
                                   uint8_t *out_data,
                                   size_t out_limit) -> size_t
    {
        return Compressor(in_buffers, in_limit, out_data, out_limit);
    };

    _compressor->decompress =
        [this](const uint8_t *in_data, size_t in_limit, uint8_t *out_data, size_t out_limit) -> size_t
        {
            return Decompressor(in_data, in_limit, out_data, out_limit);
        };

    _compressor->destroy = [this]() -> void
    {
        _udp_destroy();
    };
}

Network::~Network()
{
    close_connection();
}
