#ifdef __linux__
#include <arpa/inet.h>
#endif

#include "lib/rudp/const.h"
#include "lib/rudp/enum.h"
#include "RUdpProtocolType.h"

namespace rudp
{
    RUdpProtocolHeader *
    ConvertNetworkByteOrderToHostByteOrder(RUdpProtocolHeader *header) {
        header->peer_id = ntohs(header->peer_id);

        uint16_t flags = header->peer_id & static_cast<uint16_t>(RUdpProtocolFlag::HEADER_MASK);

        if (flags & static_cast<uint16_t>(RUdpProtocolFlag::HEADER_SENT_TIME)) {
            header->sent_time = ntohs(header->sent_time);
        }

        return header;
    }

    RUdpProtocolType *
    ConvertNetworkByteOrderToHostByteOrder(RUdpProtocolType *cmd) {
        if (cmd == nullptr) {
            return nullptr;
        }

        cmd->header.reliable_sequence_number = ntohs(cmd->header.reliable_sequence_number);

        auto cmd_type = static_cast<RUdpProtocolCommand>(cmd->header.command & PROTOCOL_COMMAND_MASK);

        if (cmd_type == RUdpProtocolCommand::ACKNOWLEDGE) {
            cmd->acknowledge.received_reliable_sequence_number = ntohs(cmd->acknowledge.received_reliable_sequence_number);
            cmd->acknowledge.received_sent_time = ntohs(cmd->acknowledge.received_sent_time);
        }
        else if (cmd_type == RUdpProtocolCommand::CONNECT) {
            cmd->connect.outgoing_peer_id = ntohs(cmd->connect.outgoing_peer_id);
            cmd->connect.mtu = ntohl(cmd->connect.mtu);
            cmd->connect.window_size = ntohl(cmd->connect.window_size);
            cmd->connect.channel_count = ntohl(cmd->connect.channel_count);
            cmd->connect.incoming_bandwidth = ntohl(cmd->connect.incoming_bandwidth);
            cmd->connect.outgoing_bandwidth = ntohl(cmd->connect.outgoing_bandwidth);
            cmd->connect.segment_throttle_interval =  ntohl(cmd->connect.segment_throttle_interval);
            cmd->connect.segment_throttle_acceleration = ntohl(cmd->connect.segment_throttle_acceleration);
            cmd->connect.segment_throttle_deceleration = ntohl(cmd->connect.segment_throttle_deceleration);
            cmd->connect.connect_id = ntohl(cmd->connect.connect_id);
            cmd->connect.data = ntohl(cmd->connect.data);
        }
        else if (cmd_type == RUdpProtocolCommand::VERIFY_CONNECT) {
            cmd->verify_connect.outgoing_peer_id = ntohs(cmd->verify_connect.outgoing_peer_id);
            cmd->verify_connect.mtu = ntohl(cmd->verify_connect.mtu);
            cmd->verify_connect.window_size = ntohl(cmd->verify_connect.window_size);
            cmd->verify_connect.channel_count = ntohl(cmd->verify_connect.channel_count);
            cmd->verify_connect.incoming_bandwidth = ntohl(cmd->verify_connect.incoming_bandwidth);
            cmd->verify_connect.outgoing_bandwidth = ntohl(cmd->verify_connect.outgoing_bandwidth);
            cmd->verify_connect.segment_throttle_interval = ntohl(cmd->verify_connect.segment_throttle_interval);
            cmd->verify_connect.segment_throttle_acceleration = ntohl(cmd->verify_connect.segment_throttle_acceleration);
            cmd->verify_connect.segment_throttle_deceleration = ntohl(cmd->verify_connect.segment_throttle_deceleration);
            cmd->verify_connect.connect_id = ntohl(cmd->verify_connect.connect_id);
        }
        else if (cmd_type == RUdpProtocolCommand::DISCONNECT) {
            cmd->disconnect.data = ntohl(cmd->disconnect.data);
        }
        else if (cmd_type == RUdpProtocolCommand::PING) {
            // do nothing
        }
        else if (cmd_type == RUdpProtocolCommand::SEND_RELIABLE) {
            cmd->send_reliable.data_length = ntohs(cmd->send_reliable.data_length);
        }
        else if (cmd_type == RUdpProtocolCommand::SEND_UNRELIABLE) {
            cmd->send_unreliable.unreliable_sequence_number = ntohs(cmd->send_unreliable.unreliable_sequence_number);
            cmd->send_unreliable.data_length = ntohs(cmd->send_unreliable.data_length);
        }
        else if (cmd_type == RUdpProtocolCommand::SEND_FRAGMENT) {
            cmd->send_fragment.start_sequence_number = ntohs(cmd->send_fragment.start_sequence_number);
            cmd->send_fragment.data_length = ntohs(cmd->send_fragment.data_length);
            cmd->send_fragment.fragment_count = ntohl(cmd->send_fragment.fragment_count);
            cmd->send_fragment.fragment_number = ntohl(cmd->send_fragment.fragment_number);
            cmd->send_fragment.total_length = ntohl(cmd->send_fragment.total_length);
            cmd->send_fragment.fragment_offset = ntohl(cmd->send_fragment.fragment_offset);
        }
        else if (cmd_type == RUdpProtocolCommand::SEND_UNSEQUENCED) {
            cmd->send_unsequenced.unsequenced_group = ntohs(cmd->send_unsequenced.unsequenced_group);
            cmd->send_unsequenced.data_length = ntohs(cmd->send_unsequenced.data_length);
        }
        else if (cmd_type == RUdpProtocolCommand::BANDWIDTH_LIMIT) {
            cmd->bandwidth_limit.incoming_bandwidth = ntohl(cmd->bandwidth_limit.incoming_bandwidth);
            cmd->bandwidth_limit.outgoing_bandwidth = ntohl(cmd->bandwidth_limit.outgoing_bandwidth);
        }
        else if (cmd_type == RUdpProtocolCommand::THROTTLE_CONFIGURE) {
            cmd->throttle_configure.segment_throttle_interval = ntohl(cmd->throttle_configure.segment_throttle_interval);
            cmd->throttle_configure.segment_throttle_acceleration = ntohl(cmd->throttle_configure.segment_throttle_acceleration);
            cmd->throttle_configure.segment_throttle_deceleration = ntohl(cmd->throttle_configure.segment_throttle_deceleration);
        }

        return cmd;
    }
} // namespace rudp
