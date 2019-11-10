#define CATCH_CONFIG_MAIN

#include <memory>
#include <unistd.h>

#include <catch2/catch.hpp>

#include "core/logger.h"
#include "core/singleton.h"
#include "lib/rudp/RUdpHost.h"

namespace
{
    constexpr auto SLEEP_DURATION = 10 * 1000; // microsecond

    void
    LOG(const std::string &message)
    { core::Singleton<core::Logger>::Instance().Debug(message); }

    void
    DELAY()
    { usleep(SLEEP_DURATION); }
}

class HostFixture
{
public:
    HostFixture()
    {
        RUdpAddress address;
        address.port(8888);
        host_ = std::make_unique<RUdpHost>(address, SysCh::MAX, 32, 100, 100);
        core::Singleton<core::Logger>::Instance().Init("Broadcast");
    }

    void
    Broadcast(SysCh ch, std::shared_ptr<RUdpSegment> &segment)
    { host_->Broadcast(ch, segment); }

    RUdpPeerState
    PeerState(size_t idx)
    { return host_->PeerState(0); }

    EventStatus
    Service(std::unique_ptr<RUdpEvent> &event, uint32_t timeout)
    { return host_->Service(event, timeout); }

private:
    std::shared_ptr<RUdpHost> host_;
};

TEST_CASE_METHOD(HostFixture, "Broadcast", "[broadcast]")
{
    // テスト内容
    // 1. Guest1をHostに接続
    // 2. Guest2をHostに接続
    // 3. Guest1からHostにデータ送信（Host側で正常に受信できたことを検証）
    // 4. Guest2からHostにデータ送信（Host側で正常に受信できたことを検証）
}
