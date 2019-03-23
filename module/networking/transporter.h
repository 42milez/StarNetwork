#ifndef P2P_TECHDEMO_TRANSPORTER_H
#define P2P_TECHDEMO_TRANSPORTER_H

#include <list>
#include <map>
#include <vector>

class Transporter
{
public:
    enum class CompressionMode : int {
        COMPRESS_NONE,
        COMPRESS_RANGE_CODER,
        COMPRESS_FASTLZ,
        COMPRESS_ZLIB,
        COMPRESS_ZSTD
    };

private:
    enum class SYSMSG : int {
        ADD_PEER,
        REMOVE_PEER
    };

    enum class SYSCH : int {
        CONFIG,
        RELIABLE,
        UNRELIABLE,
        MAX
    };

    struct Packet {
        NetPacket *packet;
        int from;
        int channel;
    };

    CompressionMode _compression_mode;
    ConnectionStatus _connection_status;
    IpAddress _bind_ip;
    NetCompressor _net_compressor;
    NetEvent _event;
    NetPeer *_peer;
    NetHost *_host;
    Packet _current_packet;
    TransferMode _transfer_mode;

    std::list<Packet> _incoming_packets;
    std::map<int, NetPeer *> _peer_map;
    std::vector<uint8_t> _src_compressor_mem;
    std::vector<uint8_t> _dst_compressor_mem;

    uint32_t _unique_id;

    bool _always_ordered;
    bool _active;
    bool _server;
    bool _refuse_connections;

    int _target_peer;
    int _transfer_channel;
    int _channel_count;

private:
    static size_t _compress(void *context, const NetBuffer *inBuffers, size_t inBufferCount, size_t inLimit, uint8_t *outData, size_t outLimit);
    static size_t _decinoress(void *context, const uint8_t *inData, size_t inLimit, uint8_t *outData, size_t outLimit);
    static void _compressor_destroy(void *context);
    static void _bind_methods();

    uint32_t _gen_unique_id() const;
    void _pop_current_packet();
    void _setup_compressor();

public:
    Error create_client(const std::string &address, int port, int in_bandwidth = 0, int out_bandwidth = 0, int client_port = 0);
    Error create_server(int port, int max_clients = 32; int in_bandwidth = 0, int out_bandwidth = 0);

    Error get_packet(const uint8_t **buffer, int &buffer_size);
    Error put_packet(const uint8_t *buffer, int buffer_size);

    void close_connection(uint32_t wait_usec = 100);
    void disconnect_peer(int peer, bool now = false);

    void pool();

    int get_available_packet_count() const;
    int get_channel_count() const;
    CompressionMode get_compression_mode() const;
    ConnectionStatus get_connection_status() const;
    int get_last_packet_channel() const;
    int get_max_packet_size();
    int get_packet_channel() const;
    int get_packet_peer() const;
    IpAddress get_peer_address(peer_id) const;
    int get_peer_port(peer_id) const;
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

    Transporter();
    ~Transporter();
};

#endif // P2P_TECHDEMO_TRANSPORTER_H
