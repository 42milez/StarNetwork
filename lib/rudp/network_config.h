#ifndef P2P_TECHDEMO_NETWORK_H
#define P2P_TECHDEMO_NETWORK_H

#include <array>
#include <cstdint>
#include <cstring>

namespace rudp
{
    class NetworkConfig
    {
      private:
        static constexpr size_t HOST_LENGTH = 16;

      public:
        NetworkConfig();

        void
        Reset();

        void
        SetIP(const uint8_t *ip, size_t size);

      public:
        [[nodiscard]] inline const std::array<uint8_t, HOST_LENGTH> &
        host() const
        {
            return host_;
        }

        inline void
        host_v4(const uint8_t *ip_address)
        {
            memcpy(&host_, ip_address, 4);
        }

        inline void
        host_v6(const uint8_t *ip_address)
        {
            memcpy(&host_, ip_address, 16);
        }

        [[nodiscard]] inline uint16_t
        port() const
        {
            return port_;
        }

        inline void
        port(uint16_t val)
        {
            port_ = val;
        }

        [[nodiscard]] inline uint8_t
        wildcard() const
        {
            return wildcard_;
        }

      public:
        NetworkConfig &
        operator=(const NetworkConfig &address);

        bool
        operator==(const NetworkConfig &address) const;

        bool
        operator!=(const NetworkConfig &address) const;

      private:
        std::array<uint8_t, HOST_LENGTH> host_;
        uint16_t port_;
        uint8_t wildcard_;
    };
} // namespace rudp

#endif // P2P_TECHDEMO_NETWORK_H
