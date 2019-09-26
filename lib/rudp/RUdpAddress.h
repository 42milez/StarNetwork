#ifndef P2P_TECHDEMO_RUDPADDRESS_H
#define P2P_TECHDEMO_RUDPADDRESS_H

#include <array>
#include <cstdint>
#include <cstring>

using RUdpAddress = struct RUdpAddress
{
private:
    static constexpr size_t HOST_LENGTH = 16;

public:
    RUdpAddress();
    void Reset();
    void SetIP(const uint8_t *ip, size_t size);

public:
    [[nodiscard]]
    inline const std::array<uint8_t, HOST_LENGTH> &
    host() const { return host_; }

    inline void
    host(const uint8_t *ip_address) { memcpy(&host_, ip_address, host_.size()); }

    [[nodiscard]]
    inline uint16_t
    port() const { return port_; }

    inline void
    port(uint16_t val) { port_ = val; }

    [[nodiscard]]
    inline uint8_t
    wildcard() const { return wildcard_; }

public:
    RUdpAddress & operator =(const RUdpAddress &address);
    bool operator ==(const RUdpAddress &address) const;
    bool operator !=(const RUdpAddress &address) const;

private:
    std::array<uint8_t, HOST_LENGTH> host_;
    uint16_t port_;
    uint8_t wildcard_;
};

#endif // P2P_TECHDEMO_RUDPADDRESS_H
