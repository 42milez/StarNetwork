#ifndef P2P_TECHDEMO_APP_NETWORK_H_
#define P2P_TECHDEMO_APP_NETWORK_H_

#include <list>
#include <map>
#include <vector>

#include "lib/core/errors.h"
#include "lib/core/io/ip_address.h"
#include "lib/core/network/system.h"
#include "lib/core/network/types.h"

#include "lib/rudp/buffer.h"
#include "lib/rudp/compress.h"
#include "lib/rudp/event.h"
#include "lib/rudp/host.h"
#include "lib/rudp/peer/peer.h"
#include "lib/rudp/segment.h"

namespace app
{
    class Network
    {
      public:
        Network();
        ~Network();

        core::Error
        CreateClient(const std::string &server_address, uint16_t server_port, uint16_t client_port,
                     int in_bandwidth = 0, int out_bandwidth = 0);

        core::Error
        CreateServer(uint16_t port, size_t peer_count = 32, uint32_t in_bandwidth = 0, uint32_t out_bandwidth = 0);

        void
        CloseConnection(uint32_t wait_usec = 100);

        void
        Poll();

        std::tuple<core::Error, std::shared_ptr<rudp::Segment>>
        Receive();

        core::Error
        Send(const std::string &str);

        inline size_t
        Peek()
        {
            return payloads_.size();
        }

      private:
        enum class ConnectionStatus : uint8_t
        {
            DISCONNECTED,
            CONNECTING,
            CONNECTED
        };

        enum class TargetPeer : uint8_t
        {
            BROADCAST,
            SERVER
        };

        enum class TransferMode : uint8_t
        {
            UNRELIABLE,
            UNRELIABLE_ORDERED,
            RELIABLE
        };

        std::list<core::Payload> payloads_;
        std::map<uint32_t, std::shared_ptr<rudp::Peer>> peers_;

        std::unique_ptr<rudp::Host> host_;

        ConnectionStatus connection_status_;
        core::IpAddress bind_ip_;
        core::Payload current_payload_;
        core::SysCh channel_count_;
        core::SysCh transfer_channel_;
        TransferMode transfer_mode_;

        uint32_t unique_id_;

        int target_peer_id_;

        bool active_;
        bool refuse_connections_;
        bool server_;
        bool server_relay_;
    };
} // namespace app

#endif // P2P_TECHDEMO_APP_NETWORK_H_
