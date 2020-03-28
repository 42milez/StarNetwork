#include <complex>
#include <string>

#include "lib/core/encode.h"
#include "lib/core/error_macros.h"
#include "lib/core/exit_handler.h"
#include "lib/core/hash.h"
#include "lib/core/io/compression.h"
#include "lib/core/network/constants.h"
#include "lib/core/singleton.h"
#include "lib/core/string.h"

#include "lib/rudp/network_config.h"
#include "lib/rudp/peer/peer.h"

#include "network.h"

app::Network::Network()
    : active_()
    , always_ordered_()
    , bind_ip_("*")
    , channel_count_(core::SysCh::MAX)
    , connection_status_(ConnectionStatus::DISCONNECTED)
    , current_payload_()
    , refuse_connections_()
    , server_()
    , server_relay_()
    , target_peer_id_()
    , transfer_channel_(core::SysCh::CONFIG)
    , transfer_mode_(TransferMode::RELIABLE)
    , unique_id_()
{
}

app::Network::~Network()
{
    CloseConnection();
}

void
app::Network::CloseConnection(uint32_t wait_usec)
{
    ERR_FAIL_COND(!active_)

    auto peers_disconnected = false;

    for (const auto &[id, peer] : peers_) {
        host_->DisconnectNow(peer, unique_id_);
        peers_disconnected = true;
    }

    if (peers_disconnected) {
        host_->Flush();

        if (wait_usec > 0) {
            // TODO: wait for disconnection packets to send
            // ...
        }
    }

    active_ = false;
    payloads_.clear();
    peers_.clear();
    unique_id_         = 1;
    connection_status_ = ConnectionStatus::DISCONNECTED;
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

    rudp::NetworkConfig config;

    if (!bind_ip_.is_wildcard()) {
        ERR_FAIL_COND_V(!bind_ip_.is_ipv4(), Error::ERR_INVALID_PARAMETER)
        config.SetIP(bind_ip_.GetIPv6(), 16);
    }

    config.port(client_port);

    host_ = std::make_unique<rudp::Host>(config, channel_count_, 32, in_bandwidth, out_bandwidth);

    ERR_FAIL_COND_V(!host_, Error::CANT_CREATE)

    IpAddress ip;

    if (is_valid_ip_address(server_address)) {
        ip = IpAddress(server_address);
    }
    else {
        ip = core::Singleton<IP>::Instance().resolve_hostname(server_address, IP::Type::V4);

        ERR_FAIL_COND_V(!ip.is_valid(), Error::CANT_CREATE)
    }

    ERR_FAIL_COND_V(!ip.is_ipv4(), Error::ERR_INVALID_PARAMETER)

    rudp::NetworkConfig server_config;

    server_config.host_v6(ip.GetIPv6());
    server_config.port(server_port);

    active_            = true;
    connection_status_ = ConnectionStatus::CONNECTING;
    unique_id_         = core::Singleton<core::Hash>::Instance().uniqueID();

    return host_->Connect(server_config, channel_count_, unique_id_);
}

Error
app::Network::CreateServer(uint16_t port, size_t peer_count, uint32_t in_bandwidth, uint32_t out_bandwidth)
{
    ERR_FAIL_COND_V(active_, Error::ERR_ALREADY_IN_USE)
    ERR_FAIL_COND_V(port < 49152 || port > 65535, Error::ERR_INVALID_PARAMETER)
    ERR_FAIL_COND_V(peer_count < 0, Error::ERR_INVALID_PARAMETER)
    ERR_FAIL_COND_V(in_bandwidth < 0, Error::ERR_INVALID_PARAMETER)
    ERR_FAIL_COND_V(out_bandwidth < 0, Error::ERR_INVALID_PARAMETER)

    rudp::NetworkConfig config;

    if (!bind_ip_.is_wildcard()) {
        ERR_FAIL_COND_V(!bind_ip_.is_ipv4(), Error::ERR_INVALID_PARAMETER)
        config.SetIP(bind_ip_.GetIPv6(), 16);
    }

    IpAddress host_ip{"::FFFF:127.0.0.1"};

    config.host_v6(host_ip.GetIPv6());
    config.port(port);

    host_ = std::make_unique<rudp::Host>(config, channel_count_, peer_count, in_bandwidth, out_bandwidth);

    ERR_FAIL_COND_V(host_ == nullptr, Error::CANT_CREATE)

    active_            = true;
    connection_status_ = ConnectionStatus::CONNECTED;
    server_            = true;
    unique_id_         = 1;

    return Error::OK;
}

void
app::Network::Poll()
{
    ERR_FAIL_COND(!active_)

    auto event = std::make_unique<rudp::Event>();

    while (!core::Singleton<core::ExitHandler>::Instance().ShouldExit()) {
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
            }
            else if (server_relay_) {
                for (const auto &[id, peer] : peers_) {
                    if (id == sender_id) {
                        continue;
                    }

                    auto segment =
                        std::make_shared<rudp::Segment>(nullptr, static_cast<uint32_t>(rudp::SegmentFlag::RELIABLE));
                    auto msg         = core::EncodeUint32(static_cast<uint32_t>(core::SysMsg::REMOVE_PEER));
                    auto receiver_id = core::EncodeUint32(id);
                    segment->AppendData(msg);
                    segment->AppendData(receiver_id);
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
                core::Payload payload;
                payload.segment = event->segment();

                ERR_CONTINUE(event->PayloadLength() < core::PAYLOAD_MINIMUM_LENGTH);

                auto id          = event->Id();
                auto sender_id   = event->SenderId();
                auto receiver_id = event->ReceiverId();

                payload.from    = sender_id;
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
                            // do not resend to self, also do not send to excluded
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

std::tuple<Error, std::shared_ptr<rudp::Segment>>
app::Network::Receive()
{
    ERR_FAIL_COND_V(payloads_.empty(), std::make_tuple(Error::ERR_UNAVAILABLE, nullptr))

    current_payload_ = payloads_.front();

    payloads_.pop_front();

    return std::make_tuple(Error::OK, current_payload_.segment);
}

Error
app::Network::Send(const std::string &str)
{
    ERR_FAIL_COND_V(!active_, Error::ERR_UNCONFIGURED)
    ERR_FAIL_COND_V(connection_status_ != ConnectionStatus::CONNECTED, Error::ERR_UNCONFIGURED)

    rudp::SegmentFlag segment_flag = rudp::SegmentFlag::RELIABLE;
    core::SysCh channel            = core::SysCh::RELIABLE;

    switch (transfer_mode_) {
        case TransferMode::UNRELIABLE: {
            // ...
        } break;
        case TransferMode::UNRELIABLE_ORDERED: {
            // ...
        } break;
        case TransferMode::RELIABLE: {
            // do nothing (use defaults)
        } break;
    }

    if (transfer_channel_ > core::SysCh::CONFIG) {
        channel = transfer_channel_;
    }

    auto peer = peers_.end();

    if (target_peer_id_ != 0) {
        peer = peers_.find(std::abs(target_peer_id_));

        ERR_FAIL_COND_V(peer == peers_.end(), Error::ERR_INVALID_PARAMETER)
    }

    auto segment     = std::make_shared<rudp::Segment>(nullptr, static_cast<uint32_t>(segment_flag));
    auto msg         = core::EncodeUint32(static_cast<uint32_t>(core::SysMsg::REMOVE_PEER));
    auto receiver_id = core::EncodeUint32(target_peer_id_);

    segment->AppendData(msg);
    segment->AppendData(receiver_id);

    auto data = std::vector<uint8_t>{str.begin(), str.end()};

    segment->AppendData(data);

    if (server_) {
        if (target_peer_id_ == core::BROADCAST_ID) {
            host_->Broadcast(channel, segment);
        }
        else if (target_peer_id_ < core::BROADCAST_ID) {
            auto exclude = std::abs(target_peer_id_);

            for (const auto &[id, peer] : peers_) {
                if (id == exclude) {
                    continue;
                }

                peer->Send(channel, segment, nullptr);
            }
        }
        else {
            peer->second->Send(channel, segment, nullptr);
        }
    }
    else {
        peers_[core::SERVER_ID]->Send(channel, segment, nullptr);
    }

    host_->Flush();

    return Error::OK;
}
