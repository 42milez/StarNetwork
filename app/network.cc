#include <string>

#include "lib/core/io/compression.h"
#include "lib/core/network/constants.h"
#include "lib/core/encode.h"
#include "lib/core/error_macros.h"
#include "lib/core/hash.h"
#include "lib/core/singleton.h"
#include "lib/core/string.h"

#include "lib/rudp/network_config.h"
#include "lib/rudp/peer/peer.h"

#include "network.h"

app::Network::Payload::Payload()
    : segment(nullptr)
    , from()
    , channel()
{
}

app::Network::Network()
    : active_()
    , always_ordered_()
    , bind_ip_("*")
    , channel_count_(core::SysCh::MAX)
    , connection_status_(ConnectionStatus::DISCONNECTED)
    , current_segment_(Payload{})
    , refuse_connections_()
    , server_()
    , server_relay_()
    , target_peer_()
    , transfer_channel_(-1)
    , transfer_mode_(TransferMode::RELIABLE)
    , unique_id_()
{
}

app::Network::~Network()
{
    CloseConnection();
}

Error
app::Network::CreateClient(const std::string &server_address, uint16_t server_port, uint16_t client_port,
                           int in_bandwidth, int out_bandwidth)
{
    ERR_FAIL_COND_V(active_, Error::ERR_ALREADY_IN_USE)
    ERR_FAIL_COND_V(server_port < 49152 || server_port > 65535, Error::ERR_INVALID_PARAMETER)
    ERR_FAIL_COND_V(client_port < 49152 || client_port > 65535, Error::ERR_INVALID_PARAMETER)
    ERR_FAIL_COND_V(in_bandwidth < 0, Error::ERR_INVALID_PARAMETER)
    ERR_FAIL_COND_V(out_bandwidth < 0, Error::ERR_INVALID_PARAMETER)

    if (client_port != 0) {
        rudp::NetworkConfig client_address;

#ifdef P2P_TECHDEMO_IPV6
        if (bind_ip_.is_wildcard()) {
            client->wildcard = true;
        }
        else {
            server_address.SetIP(client, bind_ip_.get_ipv6(), 16);
        }
#else
        if (!bind_ip_.is_wildcard()) {
            ERR_FAIL_COND_V(!bind_ip_.is_ipv4(), Error::ERR_INVALID_PARAMETER)
            client_address.SetIP(bind_ip_.GetIPv4(), 8);
        }
#endif
        client_address.port(client_port);

        host_ = std::make_unique<rudp::Host>(client_address, channel_count_, 1, in_bandwidth, out_bandwidth);
    }
    else {
        // create a host with randomly assigned port
        // ...
    }

    ERR_FAIL_COND_V(!host_, Error::CANT_CREATE)

    IpAddress ip;

    if (is_valid_ip_address(server_address)) {
        ip = IpAddress(server_address);
    }
    else {
#ifdef P2P_TECHDEMO_IPV6
        ip = Singleton<IP>::Instance().resolve_hostname(server_address);
#else
        ip = core::Singleton<IP>::Instance().resolve_hostname(server_address, IP::Type::V4);
#endif
        ERR_FAIL_COND_V(!ip.is_valid(), Error::CANT_CREATE)
    }

    rudp::NetworkConfig dst_address;

#ifdef P2P_TECHDEMO_IPV6
    dst_address.SetIP(ip.get_ipv6(), 16);
#else
    ERR_FAIL_COND_V(!ip.is_ipv4(), Error::ERR_INVALID_PARAMETER)
    dst_address.host_v4(ip.GetIPv4());
#endif
    dst_address.port(server_port);

    unique_id_ = core::Singleton<core::Hash>::Instance().uniqueID();

    return host_->Connect(dst_address, channel_count_, unique_id_);
}

Error
app::Network::CreateServer(uint16_t port, size_t peer_count, uint32_t in_bandwidth, uint32_t out_bandwidth)
{
    ERR_FAIL_COND_V(active_, Error::ERR_ALREADY_IN_USE)
    ERR_FAIL_COND_V(port < 0 || port > 65535, Error::ERR_INVALID_PARAMETER)
    ERR_FAIL_COND_V(peer_count < 0, Error::ERR_INVALID_PARAMETER)
    ERR_FAIL_COND_V(in_bandwidth < 0, Error::ERR_INVALID_PARAMETER)
    ERR_FAIL_COND_V(out_bandwidth < 0, Error::ERR_INVALID_PARAMETER)

    rudp::NetworkConfig address;

#ifdef P2P_TECHDEMO_IPV6
    if (bind_ip_.is_wildcard()) {
        address->wildcard = 1;
    }
    else {
        address.SetIP(bind_ip_.get_ipv6(), 16);
    }
#else
    if (!bind_ip_.is_wildcard()) {
        ERR_FAIL_COND_V(!bind_ip_.is_ipv4(), Error::ERR_INVALID_PARAMETER)
        address.SetIP(bind_ip_.GetIPv4(), 8);
    }
#endif

    address.port(port);

    host_ = std::make_unique<rudp::Host>(address, channel_count_, peer_count, in_bandwidth, out_bandwidth);

    ERR_FAIL_COND_V(host_ == nullptr, Error::CANT_CREATE)

    active_            = true;
    server_            = true;
    connection_status_ = ConnectionStatus::CONNECTED;

    unique_id_ = core::Singleton<core::Hash>::Instance().uniqueID();

    return Error::OK;
}

void
app::Network::Poll()
{
    ERR_FAIL_COND(!active_)

    payloads_.pop_front();

    std::unique_ptr<rudp::Event> event;

    while (true) {
        if (!host_ || !active_)
            return;

        rudp::EventStatus ret = host_->Service(event, 0);

        if (ret == rudp::EventStatus::NO_EVENT_OCCURRED) {
            continue;
        }

        if (ret == rudp::EventStatus::ERROR) {
            break;
        }

        if (event->TypeIs(rudp::EventType::CONNECT)) {
            if (server_ && refuse_connections_) {
                event->peer()->Reset();
                break;
            }

            if (server_ && ((int)event->data() < 2 || peers_.find(event->data()) != peers_.end())) {
                event->peer()->Reset();
                ERR_CONTINUE(true);
            }

            auto new_id = event->data();

            if (new_id == core::BROADCAST_ID) {
                new_id = core::SERVER_ID;
            }

            event->peer()->data(new_id);

            peers_.insert(std::make_pair(new_id, event->peer()));

            connection_status_ = ConnectionStatus::CONNECTED;

            if (server_) {
                for (auto const &peer : peers_) {
                    if (peer.first == new_id) {
                        continue;
                    }

                    // send existing peers to new peer
                    auto segment =
                        std::make_shared<rudp::Segment>(nullptr, static_cast<uint32_t>(rudp::SegmentFlag::RELIABLE));
                    auto msg = core::EncodeUint32(static_cast<uint32_t>(core::SysMsg::ADD_PEER));
                    auto id  = core::EncodeUint32(peer.first);
                    segment->AppendData(msg);
                    segment->AppendData(id);
                    event->peer()->Send(core::SysCh::CONFIG, segment, nullptr);

                    // send the new peer to existing peers
                    segment =
                        std::make_shared<rudp::Segment>(nullptr, static_cast<uint32_t>(rudp::SegmentFlag::RELIABLE));
                    msg = core::EncodeUint32(static_cast<uint32_t>(core::SysMsg::ADD_PEER));
                    id  = core::EncodeUint32(event->peer()->data());
                    segment->AppendData(msg);
                    segment->AppendData(id);
                    event->peer()->Send(core::SysCh::CONFIG, segment, nullptr);
                }
            }
        }
        else if (event->TypeIs(rudp::EventType::DISCONNECT)) {
            auto sender_id = event->peer()->data();

            if (sender_id == core::BROADCAST_ID) {
                if (!server_) {
                    // do something...?
                }
                break;
            }

            if (!server_) {
                CloseConnection();
                return;
            } else if (server_relay_) {
                for (const auto &[id, peer] : peers_) {
                    if (id == sender_id) {
                        continue;
                    }

                    auto segment = std::make_shared<rudp::Segment>(nullptr, static_cast<uint32_t>(rudp::SegmentFlag::RELIABLE));
                    auto msg = core::EncodeUint32(static_cast<uint32_t>(core::SysMsg::REMOVE_PEER));
                    auto receiver_id = core::EncodeUint32(id);
                    peer->Send(core::SysCh::CONFIG, segment, nullptr);
                }
            }
        }
        else if (event->TypeIs(rudp::EventType::RECEIVE)) {
            if (event->channel_id() == static_cast<uint8_t>(core::SysCh::CONFIG)) {
                ERR_CONTINUE(event->PayloadLength() < core::PAYLOAD_MINIMUM_LENGTH)
                ERR_CONTINUE(server_)

                auto msg = event->Message();
                auto id  = event->Id();

                switch (msg) {
                case core::SysMsg::ADD_PEER:
                    // ...
                    break;
                case core::SysMsg::REMOVE_PEER:
                    peers_.erase(id);
                    break;
                }
            }
            else if (static_cast<core::SysCh>(event->channel_id()) < channel_count_) {
                Payload payload;
                payload.segment = event->segment();

                ERR_CONTINUE(event->PayloadLength() < core::PAYLOAD_MINIMUM_LENGTH);

                auto id = event->Id();
                auto sender_id = event->SenderId();
                auto receiver_id = event->ReceiverId();

                payload.from = sender_id;
                payload.channel = event->channel_id();

                if (server_) {
                    ERR_CONTINUE(sender_id != id)

                    payload.from = id;

                    if (receiver_id == 1) {
                        payloads_.push_back(payload);
                    }
                    else if (!server_relay_) {
                        continue;
                    }
                    else if (receiver_id == 0) {
                        payloads_.push_back(payload);

                        for (const auto &[id, peer] : peers_) {
                            if (id == sender_id) {
                                continue;
                            }

                            peer->Send(event->Channel(), payload.segment, nullptr);
                        }
                    }
                    else if (receiver_id < 0) {
                        for (const auto &[id, peer] : peers_) {
                            if (id == sender_id || id == -receiver_id) {
                                continue;
                            }

                            peer->Send(event->Channel(), payload.segment, nullptr);
                        }

                        if (-receiver_id != 1) {
                            payloads_.push_back(payload);
                        }
                    }
                    else {
                        ERR_CONTINUE(peers_.find(receiver_id) == peers_.end())
                        auto peer = peers_.find(receiver_id)->second;
                        peer->Send(static_cast<core::SysCh>(event->channel_id()), payload.segment, nullptr);
                    }
                }
                else {
                    payloads_.push_back(payload);
                }
            }
            else {
                ERR_CONTINUE(true)
            }
        }
        else if (event->TypeIs(rudp::EventType::NONE)) {
            // do nothing
        }
    }
}

// TODO: add implementation
void
app::Network::CloseConnection(uint32_t wait_usec)
{
    // ...
}

void
app::Network::Disconnect(int peer_idx, bool now)
{
    ERR_FAIL_COND(!active_)
    ERR_FAIL_COND(!server_)

    auto peer = peers_.at(peer_idx);

    if (now) {
        host_->DisconnectNow(peer, 0);
        host_->RequestPeerRemoval(peer_idx, peer);

        peers_.erase(peer_idx);
    }
    else {
        host_->DisconnectLater(peer, 0);
    }
}
