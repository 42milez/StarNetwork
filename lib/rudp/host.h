#ifndef P2P_TECHDEMO_LIB_RUDP_HOST_H_
#define P2P_TECHDEMO_LIB_RUDP_HOST_H_

#include <functional>
#include <vector>

#include "connection.h"
#include "lib/core/network/system.h"
#include "lib/rudp/peer/peer_pod.h"
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

        core::Error
        Connect(const NetworkConfig &address, core::SysCh channel_count, uint32_t data);

        void
        RequestPeerRemoval(uint32_t peer_idx, const std::shared_ptr<Peer> &peer);

        core::Error
        Send(size_t peer_id, core::SysCh ch, std::shared_ptr<Segment> &segment);

        EventStatus
        Service(std::unique_ptr<Event> &event, uint32_t timeout);

        inline core::Error
        DisconnectNow(const std::shared_ptr<Peer> &peer, uint32_t data)
        {
            return peer_pod_->DisconnectNow(peer, data, checksum_);
        }

        inline core::Error
        DisconnectLater(const std::shared_ptr<Peer> &peer, uint32_t data)
        {
            return peer_pod_->DisconnectLater(peer, data, checksum_);
        }

        inline void
        Flush()
        {
            peer_pod_->Flush(nullptr);
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

        uint32_t incoming_bandwidth_;
        uint32_t outgoing_bandwidth_;
    };
} // namespace rudp

#endif // P2P_TECHDEMO_LIB_RUDP_HOST_H_
