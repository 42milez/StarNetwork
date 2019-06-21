#ifndef P2P_TECHDEMO_NETWORK_H
#define P2P_TECHDEMO_NETWORK_H

#include <list>
#include <map>
#include <vector>

#include "core/io/ip_address.h"
#include "core/errors.h"

#include "lib/rudp/RUdpBuffer.h"
#include "lib/rudp/RUdpCompressor.h"
#include "lib/rudp/RUdpEvent.h"
#include "lib/rudp/RUdpHost.h"
#include "lib/rudp/RUdpSegment.h"
#include "lib/rudp/RUdpPeer.h"

class Network
{
public:
    Network();

    ~Network();

    Error create_client(const std::string &address, int port, int in_bandwidth = 0, int out_bandwidth = 0,
                        int client_port = 0);

    Error create_server(uint16_t port, size_t peer_count = 32, uint32_t in_bandwidth = 0, uint32_t out_bandwidth = 0);

    void poll();

    void close_connection(uint32_t wait_usec = 100);

    void disconnect(int peer, bool now = false);

    Error get_segment(const uint8_t **buffer, int &buffer_size);

    Error put_segment(const uint8_t *buffer, int buffer_size);

private:
    enum class CompressionMode: uint8_t
    {
        NONE,
        RANGE_CODER,
        FASTLZ,
        ZLIB,
        ZSTD
    };

    enum class ConnectionStatus: uint8_t
    {
        DISCONNECTED,
        CONNECTING,
        CONNECTED
    };

    enum class SysMsg: uint8_t
    {
        ADD_PEER,
        REMOVE_PEER
    };

    enum class TargetPeer: uint8_t
    {
        BROADCAST,
        SERVER
    };

    enum class TransferMode: uint8_t
    {
        UNRELIABLE,
        UNRELIABLE_ORDERED,
        RELIABLE
    };

    struct Segment
    {
        std::shared_ptr<RUdpSegment> segment;
        int from;
        int channel;

        Segment();
    };

    std::list<Segment> incoming_segments_;
    std::map<int, std::shared_ptr<RUdpPeer>> peer_map_;
    std::vector<uint8_t> dst_compressor_mem_;
    std::vector<uint8_t> src_compressor_mem_;

    std::shared_ptr<RUdpCompressor> compressor_;
    std::shared_ptr<RUdpHost> host_;

    CompressionMode compression_mode_;

    ConnectionStatus connection_status_;

    IpAddress bind_ip_;

    Segment current_segment_;

    SysCh channel_count_;

    TransferMode transfer_mode_;

    uint32_t unique_id_;

    int target_peer_;

    int transfer_channel_;

    bool active_;

    bool always_ordered_;

    bool refuse_connections_;

    bool server_;

private:
    size_t Compressor(const std::vector<RUdpBuffer> &in_buffers, size_t in_limit, uint8_t *out_data, size_t out_limit);

    size_t Decompressor(const uint8_t *in_data, size_t in_limit, uint8_t *out_data, size_t out_limit);

    void _udp_destroy();

    void _pop_current_segment();

    void _setup_compressor();
};

#endif // P2P_TECHDEMO_NETWORK_H
