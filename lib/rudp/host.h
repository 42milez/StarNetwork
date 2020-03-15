#ifndef P2P_TECHDEMO_LIB_RUDP_HOST_H_
#define P2P_TECHDEMO_LIB_RUDP_HOST_H_

#include <functional>
#include <vector>

#include "lib/core/network/system.h"
#include "lib/rudp/peer/peer_pod.h"
#include "connection.h"
#include "network_config.h"

namespace rudp
{
    class Host
    {
      public:
        Host(const NetworkConfig &address, core::SysCh channel_count, size_t peer_count,
             uint32_t in_bandwidth,   // bytes per second
             uint32_t out_bandwidth); // bytes per second

        void
        Broadcast(core::SysCh ch, std::shared_ptr<Segment> &segment);

        Error
        Connect(const NetworkConfig &address, core::SysCh channel_count, uint32_t data);

        void
        RequestPeerRemoval(uint32_t peer_idx, const std::shared_ptr<Peer> &peer);

        Error
        Send(size_t peer_id, core::SysCh ch, std::shared_ptr<Segment> &segment);

        EventStatus
        Service(std::unique_ptr<Event> &event, uint32_t timeout);

        inline Error
        DisconnectNow(const std::shared_ptr<Peer> &peer, uint32_t data)
        {
            return peer_pod_->DisconnectNow(peer, data, checksum_);
        }

        inline Error
        DisconnectLater(const std::shared_ptr<Peer> &peer, uint32_t data)
        {
            return peer_pod_->DisconnectLater(peer, data, checksum_);
        }

        inline std::shared_ptr<Peer>
        PeerPtr(size_t idx)
        {
            return peer_pod_->GetPeer(idx);
        }

        inline RUdpPeerState
        PeerState(size_t idx)
        {
            return peer_pod_->GetPeer(idx)->net()->state();
        }

      private:
        inline int
        SocketWait(uint8_t wait_condition, uint32_t timeout)
        {
            return 0;
        };

      private:
        std::shared_ptr<Connection> conn_;
        std::unique_ptr<PeerPod> peer_pod_;

        ChecksumCallback checksum_;
        core::SysCh channel_count_;

        size_t maximum_segment_size_;

        uint32_t incoming_bandwidth_;
        uint32_t outgoing_bandwidth_;
        uint32_t mtu_;
    };
} // namespace rudp

#endif // P2P_TECHDEMO_LIB_RUDP_HOST_H_
