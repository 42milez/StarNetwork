#ifdef __linux__
#include <arpa/inet.h>
#endif

#include "lib/rudp/command/command_size.h"
#include "lib/rudp/channel.h"
#include "lib/rudp/macro.h"
#include "lib/rudp/time.h"
#include "RUdpProtocol.h"

namespace rudp
{
    RUdpProtocol::RUdpProtocol()
            : bandwidth_throttle_epoch_(),
              chamber_(std::make_unique<Chamber>()),
              dispatch_hub_(std::make_unique<DispatchHub>()),
              maximum_waiting_data_(HOST_DEFAULT_MAXIMUM_WAITING_DATA)
    {}

#define IS_PEER_NOT_CONNECTED(peer) \
    peer->net()->StateIsNot(RUdpPeerState::CONNECTED) && peer->net()->StateIsNot(RUdpPeerState::DISCONNECT_LATER)

    void
    RUdpProtocol::BandwidthThrottle(uint32_t service_time,
            uint32_t incoming_bandwidth,
            uint32_t outgoing_bandwidth,
            const std::vector<std::shared_ptr<Peer>>& peers)
    {
        if (UDP_TIME_DIFFERENCE(service_time, bandwidth_throttle_epoch_) >= HOST_BANDWIDTH_THROTTLE_INTERVAL)
        {
            auto time_current = Time::Get();
            auto time_elapsed = time_current - bandwidth_throttle_epoch_;
            auto peers_remaining = dispatch_hub_->connected_peers();
            auto data_total = ~0u;
            auto bandwidth = ~0u;
            auto throttle = 0;
            auto bandwidth_limit = 0;
            auto needs_adjustment = dispatch_hub_->bandwidth_limited_peers() > 0 ? true : false;

            if (time_elapsed < HOST_BANDWIDTH_THROTTLE_INTERVAL)
                return;

            bandwidth_throttle_epoch_ = time_current;

            if (peers_remaining == 0)
                return;

            //  Throttle outgoing bandwidth
            // --------------------------------------------------

            if (outgoing_bandwidth != 0)
            {
                data_total = 0;
                bandwidth = outgoing_bandwidth * (time_elapsed / 1000);

                for (const auto& peer : peers)
                {
                    if (IS_PEER_NOT_CONNECTED(peer))
                        continue;

                    data_total += peer->command_pod()->outgoing_data_total();
                }
            }

            //  Throttle peer bandwidth : Case A ( adjustment is needed )
            // --------------------------------------------------

            while (peers_remaining > 0 && needs_adjustment)
            {
                needs_adjustment = false;

                if (data_total <= bandwidth)
                    throttle = PEER_SEGMENT_THROTTLE_SCALE;
                else
                    throttle = (bandwidth * PEER_SEGMENT_THROTTLE_SCALE) / data_total;

                for (auto& peer : peers)
                {
                    uint32_t peer_bandwidth;
                    auto& net = peer->net();
                    auto& cmd_pod = peer->command_pod();

                    if ((IS_PEER_NOT_CONNECTED(peer)) ||
                        net->incoming_bandwidth() == 0 ||
                        net->outgoing_bandwidth_throttle_epoch() == time_current)
                    {
                        continue;
                    }

                    peer_bandwidth = net->incoming_bandwidth() * (time_elapsed / 1000);
                    if ((throttle * cmd_pod->outgoing_data_total()) / PEER_SEGMENT_THROTTLE_SCALE <= peer_bandwidth)
                        continue;

                    net->segment_throttle_limit(
                            (peer_bandwidth * PEER_SEGMENT_THROTTLE_SCALE) / cmd_pod->outgoing_data_total());

                    if (net->segment_throttle_limit() == 0)
                        net->segment_throttle_limit(1);

                    if (net->segment_throttle() > net->segment_throttle_limit())
                        net->segment_throttle(net->segment_throttle_limit());

                    net->outgoing_bandwidth_throttle_epoch(time_current);
                    cmd_pod->IncrementIncomingDataTotal(0);
                    cmd_pod->IncrementOutgoingDataTotal(0);

                    needs_adjustment = true;

                    --peers_remaining;

                    bandwidth -= peer_bandwidth;
                    data_total -= peer_bandwidth;
                }
            }

            //  Throttle peer bandwidth : Case B ( adjustment is NOT needed )
            // --------------------------------------------------

            if (peers_remaining > 0)
            {
                if (data_total <= bandwidth)
                    throttle = PEER_SEGMENT_THROTTLE_SCALE;
                else
                    throttle = (bandwidth * PEER_SEGMENT_THROTTLE_SCALE) / data_total;

                for (auto& peer : peers)
                {
                    auto& net = peer->net();
                    auto& cmd_pod = peer->command_pod();

                    if ((IS_PEER_NOT_CONNECTED(peer)) || net->outgoing_bandwidth_throttle_epoch() == time_current)
                        continue;

                    net->segment_throttle_limit(throttle);

                    if (net->segment_throttle() > net->segment_throttle_limit())
                        net->segment_throttle(net->segment_throttle_limit());

                    cmd_pod->IncrementIncomingDataTotal(0);
                    cmd_pod->IncrementOutgoingDataTotal(0);
                }
            }

            //  Recalculate Bandwidth Limits
            // --------------------------------------------------

            if (dispatch_hub_->recalculate_bandwidth_limits())
            {
                dispatch_hub_->recalculate_bandwidth_limits(false);
                peers_remaining = dispatch_hub_->connected_peers();
                bandwidth = incoming_bandwidth;
                needs_adjustment = true;

                if (bandwidth == 0)
                {
                    bandwidth_limit = 0;
                }
                else
                {
                    while (peers_remaining > 0 && needs_adjustment)
                    {
                        needs_adjustment = false;
                        bandwidth_limit = bandwidth / peers_remaining;

                        for (auto& peer: peers)
                        {
                            auto& net = peer->net();

                            if ((IS_PEER_NOT_CONNECTED(peer)) ||
                                net->incoming_bandwidth_throttle_epoch() == time_current)
                                continue;

                            if (net->outgoing_bandwidth() > 0 &&
                                net->outgoing_bandwidth() >= bandwidth_limit)
                                continue;

                            net->incoming_bandwidth_throttle_epoch(time_current);

                            needs_adjustment = true;

                            --peers_remaining;

                            bandwidth -= net->outgoing_bandwidth();
                        }
                    }
                }

                for (auto& peer : peers)
                {
                    auto& net = peer->net();

                    if (IS_PEER_NOT_CONNECTED(peer))
                        continue;

                    auto cmd = std::make_shared<ProtocolType>();

                    cmd->header.command = static_cast<uint8_t>(RUdpProtocolCommand::BANDWIDTH_LIMIT) |
                                          static_cast<uint16_t>(RUdpProtocolFlag::COMMAND_ACKNOWLEDGE);
                    cmd->header.channel_id = 0xFF;
                    cmd->bandwidth_limit.outgoing_bandwidth = htonl(outgoing_bandwidth);

                    if (net->incoming_bandwidth_throttle_epoch() == time_current)
                        cmd->bandwidth_limit.incoming_bandwidth = htonl(net->outgoing_bandwidth());
                    else
                        cmd->bandwidth_limit.incoming_bandwidth = htonl(bandwidth_limit);

                    peer->QueueOutgoingCommand(cmd, nullptr, 0);
                }
            }
        }
    }

    void
    RUdpProtocol::NotifyDisconnect(std::shared_ptr<Peer>& peer, const std::unique_ptr<Event>& event)
    {
        auto& net = peer->net();

        if (net->StateIsGreaterThanOrEqual(RUdpPeerState::CONNECTION_PENDING))
            // ピアを切断するのでバンド幅を再計算する
            dispatch_hub_->recalculate_bandwidth_limits(true);

        // ピアのステートが以下の３つの内のいずれかである場合
        // 1. DISCONNECTED,
        // 2. ACKNOWLEDGING_CONNECT,
        // 3. CONNECTION_PENDING
        //if (peer->state != RUdpPeerState::CONNECTING && peer->state < RUdpPeerState::CONNECTION_SUCCEEDED)
        if (net->StateIsNot(RUdpPeerState::CONNECTING)
            && net->StateIsLessThanOrEqual(RUdpPeerState::CONNECTION_SUCCEEDED))
        {
            ResetPeer(peer);
        }
            // ピアが接続済みである場合
        else if (event != nullptr)
        {
            event->type(EventType::DISCONNECT);
            event->peer(peer);
            event->data(0);

            ResetPeer(peer);
        }
        else
        {
            peer->event_data(0);

            dispatch_hub_->DispatchState(peer, RUdpPeerState::ZOMBIE);
        }
    }

    EventStatus
    RUdpProtocol::DispatchIncomingCommands(std::unique_ptr<Event>& event)
    {
        while (dispatch_hub_->PeerExists())
        {
            auto peer = dispatch_hub_->Dequeue();
            auto& net = peer->net();

            peer->needs_dispatch(false);

            if (net->StateIs(RUdpPeerState::CONNECTION_PENDING) ||
                net->StateIs(RUdpPeerState::CONNECTION_SUCCEEDED))
            {
                // ピアが接続したら接続中ピアのカウンタを増やし、切断したら減らす
                dispatch_hub_->ChangeState(peer, RUdpPeerState::CONNECTED);

                event->type(EventType::CONNECT);
                event->peer(peer);
                event->data(peer->event_data());

                return EventStatus::AN_EVENT_OCCURRED;
            }
            else if (net->StateIs(RUdpPeerState::ZOMBIE))
            {
                dispatch_hub_->recalculate_bandwidth_limits(true);

                event->type(EventType::DISCONNECT);
                event->peer(peer);
                event->data(peer->event_data());

                // ゾンビ状態になったピアはリセットする
                ResetPeer(peer);

                return EventStatus::AN_EVENT_OCCURRED;
            }
            else if (net->StateIs(RUdpPeerState::CONNECTED))
            {
                if (!peer->DispatchedCommandExists())
                    continue;

                // 接続済みのピアからセグメントを取得
                auto[segment, channel_id] = peer->Receive();

                if (segment == nullptr)
                    continue;

                event->segment(segment);
                event->channel_id(channel_id);
                event->type(EventType::RECEIVE);
                event->peer(peer);

                // ディスパッチすべきピアが残っている場合は、ディスパッチ待ちキューにピアを投入する
                if (peer->DispatchedCommandExists())
                {
                    peer->needs_dispatch(true);

                    dispatch_hub_->Enqueue(peer);
                }

                return EventStatus::AN_EVENT_OCCURRED;
            }
        }

        return EventStatus::NO_EVENT_OCCURRED;
    }

    Error
    RUdpProtocol::DispatchIncomingReliableCommands(std::shared_ptr<Peer>& peer,
            const std::shared_ptr<ProtocolType>& cmd)
    {
        auto ch = peer->GetChannel(cmd->header.channel_id);

        auto reliable_commands = ch->NewIncomingReliableCommands();

        if (reliable_commands.empty())
            return Error::OK;

        peer->PushIncomingCommandsToDispatchQueue(reliable_commands);

        if (!peer->needs_dispatch())
        {
            dispatch_hub_->Enqueue(peer);
            peer->needs_dispatch(true);
        }

        if (ch->IncomingUnreliableCommandExists())
            DispatchIncomingUnreliableCommands(peer, cmd);

        return Error::OK;
    }

    Error
    RUdpProtocol::DispatchIncomingUnreliableCommands(const std::shared_ptr<Peer>& peer,
            const std::shared_ptr<ProtocolType>& cmd)
    {
        // TODO: add implementation
        // ...

        return Error::OK;
    }

    Error
    RUdpProtocol::HandleAcknowledge(const std::unique_ptr<Event>& event,
            std::shared_ptr<Peer>& peer,
            const std::shared_ptr<ProtocolType>& cmd,
            uint32_t service_time,
            std::function<void(std::shared_ptr<Peer>& peer)> disconnect)
    {
        auto& net = peer->net();
        auto& cmd_pod = peer->command_pod();

        if (net->StateIs(RUdpPeerState::DISCONNECTED) || net->StateIs(RUdpPeerState::ZOMBIE))
            return Error::OK;

        uint32_t received_sent_time = cmd->acknowledge.received_sent_time;
        received_sent_time |= service_time & 0xFFFF0000;

        if ((received_sent_time & 0x8000) > (service_time & 0x8000))
            received_sent_time -= 0x10000;

        if (UDP_TIME_LESS(service_time, received_sent_time))
            return Error::OK;

        peer->last_receive_time(service_time);
        cmd_pod->earliest_timeout(0);

        auto round_trip_time = UDP_TIME_DIFFERENCE(service_time, received_sent_time);

        net->segment_throttle(round_trip_time);
        peer->UpdateRoundTripTimeVariance(service_time, round_trip_time);

        auto received_reliable_sequence_number = cmd->acknowledge.received_reliable_sequence_number;

        core::Singleton<core::Logger>::Instance().Debug("acknowledge was received ({0})",
                received_reliable_sequence_number);

        auto command_number = peer->RemoveSentReliableCommand(received_reliable_sequence_number,
                cmd->header.channel_id);

        auto state = net->state();
        if (state == RUdpPeerState::ACKNOWLEDGING_CONNECT)
        {
            if (command_number != RUdpProtocolCommand::VERIFY_CONNECT)
                return Error::ERROR;

            dispatch_hub_->NotifyConnect(event, peer);
        }
        else if (state == RUdpPeerState::DISCONNECTING)
        {
            if (command_number != RUdpProtocolCommand::DISCONNECT)
                return Error::ERROR;

            dispatch_hub_->NotifyDisconnect(event, peer);
        }
        else if (state == RUdpPeerState::DISCONNECT_LATER)
        {
            if (!cmd_pod->OutgoingReliableCommandExists() &&
                !cmd_pod->OutgoingUnreliableCommandExists() &&
                !cmd_pod->SentReliableCommandExists())
            {
                disconnect(peer);
            }
        }

        return Error::OK;
    }

    Error
    RUdpProtocol::HandleBandwidthLimit(const std::shared_ptr<Peer>& peer,
            const std::shared_ptr<ProtocolType>& cmd,
                                       std::vector<uint8_t>::iterator & data)
    {
        // TODO
        // ...

        return Error::OK;
    }

    void
    RUdpProtocol::HandleConnect(std::shared_ptr<Peer>& peer,
            const ProtocolHeader* header,
            const std::shared_ptr<ProtocolType>& cmd,
            const NetworkConfig& received_address,
            uint32_t host_outgoing_bandwidth,
            uint32_t host_incoming_bandwidth)
    {
        auto channel_count = cmd->connect.channel_count;

        if (channel_count < PROTOCOL_MINIMUM_CHANNEL_COUNT || channel_count > PROTOCOL_MAXIMUM_CHANNEL_COUNT)
            return;

        peer->SetupConnectedPeer(cmd,
                received_address,
                host_incoming_bandwidth,
                host_outgoing_bandwidth,
                channel_count);
    }

    Error
    RUdpProtocol::HandlePing(const std::shared_ptr<Peer>& peer)
    {
        auto& net = peer->net();

        if (net->StateIs(RUdpPeerState::CONNECTED) && net->StateIs(RUdpPeerState::DISCONNECT_LATER))
            return Error::ERROR;

        return Error::OK;
    }

    Error
    RUdpProtocol::HandleSendFragment(std::shared_ptr<Peer>& peer,
            const std::shared_ptr<ProtocolType>& cmd,
                                     std::vector<uint8_t> & data, uint16_t flags)
    {
        if (!peer->StateIs(RUdpPeerState::CONNECTED) && !peer->StateIs(RUdpPeerState::DISCONNECT_LATER))
            return Error::ERROR;

        auto ch = peer->GetChannel(cmd->header.channel_id);
        auto start_sequence_number = cmd->send_fragment.start_sequence_number;
        auto start_window = start_sequence_number / PEER_RELIABLE_WINDOW_SIZE;
        auto current_window = ch->incoming_reliable_sequence_number() / PEER_RELIABLE_WINDOW_SIZE;

        if (start_sequence_number < ch->incoming_reliable_sequence_number())
            start_window += PEER_RELIABLE_WINDOWS;

        if (start_window < current_window || start_window >= current_window + PEER_FREE_RELIABLE_WINDOWS - 1)
            return Error::OK;

        auto fragment_length = cmd->send_fragment.data_length;
        auto fragment_number = cmd->send_fragment.fragment_number;
        auto fragment_count = cmd->send_fragment.fragment_count;
        auto fragment_offset = cmd->send_fragment.fragment_offset;
        auto total_length = cmd->send_fragment.total_length;

        if (fragment_count > PROTOCOL_MAXIMUM_FRAGMENT_COUNT ||
            fragment_number >= fragment_count ||
            total_length > HOST_DEFAULT_MAXIMUM_SEGMENT_SIZE ||
            fragment_offset >= total_length ||
            fragment_length > total_length - fragment_offset)
        {
            return Error::ERROR;
        }

        auto[firstCmd, err] = ch->ExtractFirstCommand(start_sequence_number, total_length, fragment_count);

        if (!firstCmd)
        {
            cmd->header.reliable_sequence_number = start_sequence_number;
            auto[in_cmd, error] = ch->QueueIncomingCommand(cmd, data, flags, fragment_count);

            in_cmd->MarkFragmentReceived(fragment_number);

            if (error != Error::OK)
            {
                return Error::ERROR;
            }
        }
            // copy a fragment into the buffer of the first command
        else if (!firstCmd->IsFragmentAlreadyReceived(fragment_number))
        {
            firstCmd->MarkFragmentReceived(fragment_number);

            auto data_length = firstCmd->segment()->DataLength();

            if (fragment_offset + fragment_length > data_length)
            {
                fragment_length = data_length - fragment_offset;
            }

            firstCmd->CopyFragmentedPayload(data);

            if (firstCmd->IsAllFragmentsReceived())
            {
                DispatchIncomingReliableCommands(peer, cmd);
            }
        }

        return Error::OK;
    }

    Error
    RUdpProtocol::HandleSendReliable(std::shared_ptr<Peer>& peer,
            const std::shared_ptr<ProtocolType>& cmd,
                                     std::vector<uint8_t>& data,
            uint16_t data_length,
            uint16_t flags,
            uint32_t fragment_count)
    {
        auto ret = peer->QueueIncomingCommand(cmd, data, data_length, flags, fragment_count, maximum_waiting_data_);

        if (ret != Error::OK)
            return ret;

        auto cmd_type = static_cast<RUdpProtocolCommand>(cmd->header.command & PROTOCOL_COMMAND_MASK);

        if (cmd_type == RUdpProtocolCommand::SEND_FRAGMENT || cmd_type == RUdpProtocolCommand::SEND_RELIABLE)
        {
            auto commands_dispatched = DispatchIncomingReliableCommands(peer, cmd);

            if (ret != Error::OK)
                return ret;
        }
        else
        {
            ret = DispatchIncomingUnreliableCommands(peer, cmd);

            if (ret != Error::OK)
                return ret;
        }

        return Error::OK;
    }

    Error
    RUdpProtocol::HandleVerifyConnect(const std::unique_ptr<Event>& event,
            std::shared_ptr<Peer>& peer,
            const std::shared_ptr<ProtocolType>& cmd)
    {
        auto& net = peer->net();

        if (net->StateIsNot(RUdpPeerState::CONNECTING))
            return Error::OK;

        auto channel_count = cmd->verify_connect.channel_count;

        if (channel_count < PROTOCOL_MINIMUM_CHANNEL_COUNT ||
            channel_count > PROTOCOL_MAXIMUM_CHANNEL_COUNT ||
            cmd->verify_connect.segment_throttle_interval != net->segment_throttle_interval() ||
            cmd->verify_connect.segment_throttle_acceleration != net->segment_throttle_acceleration() ||
            cmd->verify_connect.segment_throttle_deceleration != net->segment_throttle_deceleration() ||
            cmd->verify_connect.connect_id != peer->connect_id())
        {
            peer->event_data(0);

            dispatch_hub_->DispatchState(peer, RUdpPeerState::ZOMBIE);

            return Error::ERROR;
        }

        peer->RemoveSentReliableCommand(1, 0xFF);
        peer->outgoing_peer_id(cmd->verify_connect.outgoing_peer_id);
        peer->incoming_session_id(cmd->verify_connect.incoming_session_id);
        peer->outgoing_session_id(cmd->verify_connect.outgoing_session_id);

        auto mtu = cmd->verify_connect.mtu;

        if (mtu < PROTOCOL_MINIMUM_MTU)
            mtu = PROTOCOL_MINIMUM_MTU;
        else if (mtu > PROTOCOL_MAXIMUM_MTU)
            mtu = PROTOCOL_MAXIMUM_MTU;

        if (mtu < net->mtu())
            net->mtu(mtu);

        auto window_size = cmd->verify_connect.window_size;

        if (window_size < PROTOCOL_MINIMUM_WINDOW_SIZE)
            window_size = PROTOCOL_MINIMUM_WINDOW_SIZE;

        if (window_size > PROTOCOL_MAXIMUM_WINDOW_SIZE)
            window_size = PROTOCOL_MAXIMUM_WINDOW_SIZE;

        if (window_size < net->window_size())
            net->window_size(window_size);

        net->incoming_bandwidth(cmd->verify_connect.incoming_bandwidth);
        net->outgoing_bandwidth(cmd->verify_connect.outgoing_bandwidth);

        dispatch_hub_->NotifyConnect(event, peer);

        return Error::OK;
    }

    Error
    RUdpProtocol::HandleDisconnect(std::shared_ptr<Peer>& peer, const std::shared_ptr<ProtocolType>& cmd)
    {
        auto& net = peer->net();

        if (net->StateIs(RUdpPeerState::DISCONNECTED) ||
            net->StateIs(RUdpPeerState::ZOMBIE) ||
            net->StateIs(RUdpPeerState::ACKNOWLEDGING_DISCONNECT))
        {
            return Error::OK;
        }

        peer->ResetPeerQueues();

        if (net->StateIs(RUdpPeerState::CONNECTION_SUCCEEDED) ||
            net->StateIs(RUdpPeerState::DISCONNECTING) ||
            net->StateIs(RUdpPeerState::CONNECTING))
        {
            dispatch_hub_->DispatchState(peer, RUdpPeerState::ZOMBIE);
        }
        else if (net->StateIsNot(RUdpPeerState::CONNECTED) && net->StateIsNot(RUdpPeerState::DISCONNECT_LATER))
        {
            if (net->StateIs(RUdpPeerState::CONNECTION_PENDING))
                dispatch_hub_->recalculate_bandwidth_limits(true);

            peer->Reset();
        }
        else if (cmd->header.command & static_cast<uint16_t>(RUdpProtocolFlag::COMMAND_ACKNOWLEDGE))
        {
            dispatch_hub_->ChangeState(peer, RUdpPeerState::ACKNOWLEDGING_DISCONNECT);
        }
        else
        {
            dispatch_hub_->DispatchState(peer, RUdpPeerState::ZOMBIE);
        }

        if (net->StateIsNot(RUdpPeerState::DISCONNECTED))
            peer->event_data(cmd->disconnect.data);

        return Error::OK;
    }

// void enet_peer_reset (ENetPeer * peer)
    void
    RUdpProtocol::ResetPeer(const std::shared_ptr<Peer>& peer)
    {
        dispatch_hub_->PurgePeer(peer);

        peer->Reset();
        peer->ResetPeerQueues();
    }

    void
    RUdpProtocol::SendAcknowledgements(std::shared_ptr<Peer>& peer)
    {
        while (peer->AcknowledgementExists())
        {
            auto command = chamber_->EmptyCommandBuffer();
            auto buffer = chamber_->EmptyDataBuffer();
            auto ack = peer->PopAcknowledgement();

            // 送信継続
            // - コマンドバッファに空きがない
            // - データバッファに空きがない
            // - ピアの MTU とパケットサイズの差が ProtocolAcknowledge のサイズ未満
            if (command == nullptr ||
                buffer == nullptr ||
                peer->net()->mtu() - chamber_->segment_size() < sizeof(ProtocolAcknowledge))
            {
                chamber_->continue_sending(true);

                break;
            }

            chamber_->IncrementSegmentSize(sizeof(ProtocolAcknowledge));

            auto reliable_sequence_number = htons(ack->command().header.reliable_sequence_number);

            (*command)->header.command = static_cast<uint8_t>(RUdpProtocolCommand::ACKNOWLEDGE);
            (*command)->header.channel_id = ack->command().header.channel_id;
            (*command)->header.reliable_sequence_number = reliable_sequence_number;
            (*command)->acknowledge.received_reliable_sequence_number = reliable_sequence_number;
            (*command)->acknowledge.received_sent_time = htons(ack->sent_time());

            // REVIEW: Why is Acknowledge directly moved to buffer?
            //         Other commands are through outgoing_reliable_commands before moved to buffer.
            (*buffer)->Add(*command);

            if ((ack->command().header.command & PROTOCOL_COMMAND_MASK) == static_cast<uint8_t>(RUdpProtocolCommand::DISCONNECT))
                dispatch_hub_->DispatchState(peer, RUdpPeerState::ZOMBIE);
        }
    }

    bool
    RUdpProtocol::SendReliableOutgoingCommands(const std::shared_ptr<Peer>& peer, uint32_t service_time)
    {
        auto can_ping = peer->LoadReliableCommandsIntoChamber(chamber_, service_time);

        return can_ping;
    }

    void
    RUdpProtocol::SendUnreliableOutgoingCommands(std::shared_ptr<Peer>& peer, uint32_t service_time)
    {
        auto can_disconnect = peer->LoadUnreliableCommandsIntoChamber(chamber_);

        if (can_disconnect)
            dispatch_hub_->PurgePeer(peer);
    }
} // namespace rudp
