#ifndef TD_MODULE_NETWORK_H_
#define TD_MODULE_NETWORK_H_

#include <list>
#include <map>
#include <vector>

#include "core/io/ip_address.h"
#include "core/errors.h"

#include "lib/rudp/buffer.h"
#include "lib/rudp/RUdpCompress.h"
#include "lib/rudp/RUdpEvent.h"
#include "lib/rudp/RUdpHost.h"
#include "lib/rudp/RUdpSegment.h"
#include "lib/rudp/peer/RUdpPeer.h"

class Network
{
public:
    Network();
    ~Network();

    Error CreateClient(const std::string &server_address,
                       uint16_t server_port,
                       uint16_t client_port,
                       int in_bandwidth,
                       int out_bandwidth);
    Error CreateServer(uint16_t port, size_t peer_count = 32, uint32_t in_bandwidth = 0, uint32_t out_bandwidth = 0);

    void Poll();

    void CloseConnection(uint32_t wait_usec = 100);
    void Disconnect(int peer_idx, bool now);

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
    public:
        Segment();

    public:
        std::shared_ptr<rudp::RUdpSegment> segment;

        int channel;
        int from;
    };

    std::list<Segment> incoming_segments_;
    std::map<int, std::shared_ptr<rudp::RUdpPeer>> peers_;

    std::vector<uint8_t> dst_compressor_mem_;
    std::vector<uint8_t> src_compressor_mem_;

    std::shared_ptr<rudp::RUdpCompress> compress_;
    std::shared_ptr<rudp::RUdpHost> host_;

    CompressionMode compression_mode_;
    ConnectionStatus connection_status_;
    IpAddress bind_ip_;
    Segment current_segment_;
    rudp::SysCh channel_count_;
    TransferMode transfer_mode_;

    uint32_t unique_id_;

    int target_peer_;
    int transfer_channel_;

    bool active_;
    bool always_ordered_;
    bool refuse_connections_;
    bool server_;

private:
    size_t Compressor(const std::vector<rudp::Buffer> &in_buffers,
                      size_t in_limit,
                      std::vector<uint8_t> &out_data,
                      size_t out_limit);
    size_t Decompressor(std::vector<uint8_t> &in_data,
                        size_t in_limit,
                        std::vector<uint8_t> &out_data,
                        size_t out_limit);
    void Cleaner();

    void PopCurrentSegment();

    void SetupCompressor();
};

#endif // TD_MODULE_NETWORK_H_
