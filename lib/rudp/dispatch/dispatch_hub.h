#ifndef P2P_TECHDEMO_LIB_RUDP_DISPATCH_DISPATCH_HUB_H_
#define P2P_TECHDEMO_LIB_RUDP_DISPATCH_DISPATCH_HUB_H_

#include "lib/rudp/dispatch/dispatch_queue.h"
#include "lib/rudp/event.h"

namespace rudp
{
    class DispatchHub
    {
      public:
        DispatchHub();

        void
        MergePeer(const std::shared_ptr<Peer> &peer);

        void
        PurgePeer(const std::shared_ptr<Peer> &peer);

        void
        ChangeState(const std::shared_ptr<Peer> &peer, const RUdpPeerState &state);

        void
        NotifyConnect(const std::unique_ptr<Event> &event, std::shared_ptr<Peer> &peer);

        void
        NotifyDisconnect(const std::unique_ptr<Event> &event, std::shared_ptr<Peer> &peer);

      public:
        void
        DispatchState(std::shared_ptr<Peer> &peer, RUdpPeerState state);

        inline void
        Enqueue(std::shared_ptr<Peer> &peer)
        {
            queue_->Enqueue(peer);
        }

        inline std::shared_ptr<Peer>
        Dequeue()
        {
            return queue_->Dequeue();
        }

        inline bool
        PeerExists()
        {
            return queue_->PeerExists();
        }

      public:
        inline size_t
        bandwidth_limited_peers()
        {
            return bandwidth_limited_peers_;
        }

        inline size_t
        connected_peers()
        {
            return connected_peers_;
        }

        inline bool
        recalculate_bandwidth_limits()
        {
            return recalculate_bandwidth_limits_;
        }

        inline void
        recalculate_bandwidth_limits(bool val)
        {
            core::Singleton<core::Logger>::Instance().Debug("recalculate bandwidth limit: {0}", val);
            recalculate_bandwidth_limits_ = val;
        }

      private:
        std::unique_ptr<DispatchQueue> queue_;

        size_t bandwidth_limited_peers_;
        size_t connected_peers_;

        bool recalculate_bandwidth_limits_;
    };
} // namespace rudp

#endif // P2P_TECHDEMO_LIB_RUDP_DISPATCH_DISPATCH_HUB_H_
