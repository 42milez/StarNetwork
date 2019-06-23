#include <algorithm>
#include <string>

#include "core/error_macros.h"
#include "core/hash.h"
#include "core/singleton.h"
#include "core/string.h"
#include "core/io/compression.h"

#include "lib/rudp/RUdpAddress.h"

#include "Network.h"

Network::Segment::Segment()
    : segment(nullptr), from(), channel()
{}

Network::Network()
    : active_(false),
      always_ordered_(false),
      bind_ip_("*"),
      channel_count_(SysCh::MAX),
      compression_mode_(CompressionMode::NONE),
      connection_status_(ConnectionStatus::DISCONNECTED),
      current_segment_(Segment{}),
      refuse_connections_(false),
      server_(false),
      target_peer_(),
      transfer_channel_(-1),
      transfer_mode_(TransferMode::RELIABLE),
      unique_id_()
{
    compressor_ = std::make_shared<RUdpCompressor>();

    compressor_->compress = [this](const std::vector<RUdpBuffer> &in_buffers,
                                   size_t in_limit,
                                   std::vector<uint8_t> &out_data,
                                   size_t out_limit) -> size_t
    {
        return Compressor(in_buffers, in_limit, out_data, out_limit);
    };

    compressor_->decompress =
        [this](std::vector<uint8_t> &in_data, size_t in_limit, std::vector<uint8_t> &out_data, size_t out_limit) -> size_t
        {
            return Decompressor(in_data, in_limit, out_data, out_limit);
        };

    compressor_->destroy = [this]() -> void
    {
        Destroy();
    };
}

Network::~Network()
{
    CloseConnection();
}

Error
Network::CreateClient(const std::string &server_address, uint16_t server_port, uint16_t client_port, int in_bandwidth, int out_bandwidth)
{
    ERR_FAIL_COND_V(active_, Error::ERR_ALREADY_IN_USE)
    ERR_FAIL_COND_V(server_port < 0 || server_port > 65535, Error::ERR_INVALID_PARAMETER)
    ERR_FAIL_COND_V(client_port < 0 || client_port > 65535, Error::ERR_INVALID_PARAMETER)
    ERR_FAIL_COND_V(in_bandwidth < 0, Error::ERR_INVALID_PARAMETER)
    ERR_FAIL_COND_V(out_bandwidth < 0, Error::ERR_INVALID_PARAMETER)

    if (client_port != 0) {
        RUdpAddress client_address;

#ifdef P2P_TECHDEMO_IPV6
        if (bind_ip_.is_wildcard())
        {
            client->wildcard = true;
        }
        else
        {
            server_address.SetIP(client, bind_ip_.get_ipv6(), 16);
        }
#else
        if (!bind_ip_.is_wildcard()) {
            ERR_FAIL_COND_V(!bind_ip_.is_ipv4(), Error::ERR_INVALID_PARAMETER)
            client_address.SetIP(bind_ip_.get_ipv4(), 8);
        }
#endif
        client_address.port = client_port;

        host_ = std::make_unique<RUdpHost>(client_address, channel_count_, 1, in_bandwidth, out_bandwidth);
    }
    else {
        // create a host with random assigned port
        // ...
    }

    ERR_FAIL_COND_V(!host_, Error::CANT_CREATE)

    SetupCompressor();

    IpAddress ip;

    if (is_valid_ip_address(server_address)) {
        ip = IpAddress(server_address);
    }
    else {
#ifdef P2P_TECHDEMO_IPV6
        ip = Singleton<IP>::Instance().resolve_hostname(server_address);
#else
        ip = Singleton<IP>::Instance().resolve_hostname(server_address, IP::Type::V4);
#endif
        ERR_FAIL_COND_V(!ip.is_valid(), Error::CANT_CREATE)
    }

    RUdpAddress dst_address;

#ifdef P2P_TECHDEMO_IPV6
    dst_address.SetIP(ip.get_ipv6(), 16);
#else
    ERR_FAIL_COND_V(!ip.is_ipv4(), Error::ERR_INVALID_PARAMETER)
    memcpy(dst_address.host, ip.get_ipv4(), sizeof(dst_address.host));
#endif
    dst_address.port = server_port;

    unique_id_ = hash32();

    return host_->Connect(dst_address, channel_count_, unique_id_);
}

Error
Network::CreateServer(uint16_t port, size_t peer_count, uint32_t in_bandwidth, uint32_t out_bandwidth)
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
        address.SetIP(bind_ip_.get_ipv6(), 16);
    }
#else
    if (!bind_ip_.is_wildcard()) {
        ERR_FAIL_COND_V(!bind_ip_.is_ipv4(), Error::ERR_INVALID_PARAMETER)
        address.SetIP(bind_ip_.get_ipv4(), 8);
    }
#endif

    address.port = port;

    host_ = std::make_unique<RUdpHost>(address, channel_count_, peer_count, in_bandwidth, out_bandwidth);

    ERR_FAIL_COND_V(host_ == nullptr, Error::CANT_CREATE)

    SetupCompressor();

    active_ = true;
    server_ = true;
    connection_status_ = ConnectionStatus::CONNECTED;

    unique_id_ = hash32();

    return Error::OK;
}

void
Network::Poll()
{
    ERR_FAIL_COND(!active_)

    PopCurrentSegment();

    std::unique_ptr<RUdpEvent> event;

    while (true) {
        if (!host_ || !active_)
            return;

        int ret = host_->Service(event, 0);

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

void
Network::CloseConnection(uint32_t wait_usec)
{
    // ...
}

size_t
Network::Compressor(const std::vector<RUdpBuffer> &in_buffers,
                    size_t in_limit,
                    std::vector<uint8_t> &out_data,
                    size_t out_limit)
{
    if (src_compressor_mem_.size() < in_limit)
        src_compressor_mem_.resize(in_limit);

    auto total = in_limit;
    auto offset = 0;
    auto in_buffer_size = in_buffers.size();

    while (total) {
        for (auto i = 0; i < in_buffer_size; i++) {
            auto to_copy = std::min(total, in_buffers.at(i).data_length);
            memcpy(&src_compressor_mem_.at(offset), in_buffers.at(i).data, to_copy);
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

    if (out_data.size() < dst_compressor_mem_.size())
        out_data.resize(dst_compressor_mem_.size());

    std::copy(dst_compressor_mem_.begin(), dst_compressor_mem_.end(), out_data.begin());

    src_compressor_mem_.clear();
    dst_compressor_mem_.clear();

    return ret;
}

size_t
Network::Decompressor(std::vector<uint8_t> &in_data,
                      size_t in_limit,
                      std::vector<uint8_t> &out_data,
                      size_t out_limit)
{
    auto ret = -1;

    if (compression_mode_ == CompressionMode::ZLIB) {
        ret = Compression::decompress(out_data, out_limit, in_data, in_limit, Compression::Mode::DEFLATE);
    }
    else if (compression_mode_ == CompressionMode::ZSTD) {
        // ...
    }

    if (ret < 0)
        return 0;

    return ret;
}

void
Network::Destroy()
{
    // Nothing to do
}

void
Network::PopCurrentSegment()
{
    // ...
}

void
Network::SetupCompressor()
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
