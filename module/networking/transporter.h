#ifndef P2P_TECHDEMO_MODULE_TRANSPORTER_H
#define P2P_TECHDEMO_MODULE_TRANSPORTER_H

#include <list>
#include <map>
#include <vector>

#include "core/io/ip_address.h"
#include "core/errors.h"
#include "lib/udp/udp.h"

class Transporter
{
public:
    enum class CompressionMode : int
    {
        NONE,
        RANGE_CODER,
        FASTLZ,
        ZLIB,
        ZSTD
    };

    enum class TargetPeer : int
    {
        BROADCAST,
        SERVER
    };

    enum class TransferMode : int
    {
        UNRELIABLE,
        UNRELIABLE_ORDERED,
        RELIABLE
    };

    enum class ConnectionStatus : int
    {
        DISCONNECTED,
        CONNECTING,
        CONNECTED
    };

private:
    enum class SysMsg : int
    {
        ADD_PEER,
        REMOVE_PEER
    };

    struct Packet
    {
        UdpPacket *packet;
        int from;
        int channel;
    };

    IpAddress _bind_ip;

    CompressionMode _compression_mode;

    ConnectionStatus _connection_status;

    UdpCompressor _compressor;

    UdpEvent _event;

    UdpPeer *_peer;

    std::shared_ptr<UdpHost> _host;

    Packet _current_packet;

    TransferMode _transfer_mode;

    std::list<Packet> _incoming_packets;

    std::map<int, UdpPeer *> _peer_map;

    std::vector<uint8_t> _src_compressor_mem;

    std::vector<uint8_t> _dst_compressor_mem;

    uint32_t _unique_id;

    bool _active;

    bool _always_ordered;

    bool _refuse_connections;

    bool _server;

    SysCh _channel_count;

    int _target_peer;

    int _transfer_channel;

    std::shared_ptr<UdpCompressor> _udp_compressor;

private:
    size_t _udp_compress(const std::vector<UdpBuffer> &in_buffers,
                         size_t in_limit,
                         std::vector<uint8_t> &out_data,
                         size_t out_limit);

    size_t _udp_decompress(std::vector<uint8_t> &in_data,
                           size_t in_limit,
                           std::vector<uint8_t> &out_data,
                           size_t out_limit);

    void _udp_destroy();

    uint32_t _gen_unique_id() const;

    void _pop_current_packet();

    void _setup_compressor();

public:
    Error
    create_client(const std::string &address, int port, int in_bandwidth = 0, int out_bandwidth = 0, int client_port = 0);

    Error create_server(uint16_t port, size_t peer_count = 32, uint32_t in_bandwidth = 0, uint32_t out_bandwidth = 0);

    Error get_packet(const uint8_t **buffer, int &buffer_size);

    Error put_packet(const uint8_t *buffer, int buffer_size);

    void close_connection(uint32_t wait_usec = 100);

    void disconnect(int peer, bool now = false);

    void poll();

    int get_available_packet_count() const;

    int get_channel_count() const;

    CompressionMode get_compression_mode() const;

    ConnectionStatus get_connection_status() const;

    int get_last_packet_channel() const;

    int get_max_packet_size();

    int get_packet_channel() const;

    int get_packet_peer() const;

    IpAddress get_peer_address(int peer_id) const;

    int get_peer_port(int peer_id) const;

    int get_transfer_channel() const;

    TransferMode get_transfer_mode() const;

    int get_unique_id() const;

    void set_always_ordered(bool ordered);

    void set_bind_ip(const IpAddress &ip);

    void set_channel_count(int channel);

    void set_compression_mode(CompressionMode mode);

    void set_refuse_new_connections(bool enable);

    void set_target_peer(int peer);

    void set_transfer_channel(int channel);

    void set_transfer_mode(TransferMode mode);

    bool is_always_ordered() const;

    bool is_refusing_new_connections() const;

    bool is_server() const;

    void udp_host_compress(std::shared_ptr<UdpHost> &host);

    void udp_custom_compress(std::shared_ptr<UdpHost> &host, std::shared_ptr<UdpCompressor> &compressor);

    Transporter();

    ~Transporter();
};

#endif // P2P_TECHDEMO_MODULE_TRANSPORTER_H
