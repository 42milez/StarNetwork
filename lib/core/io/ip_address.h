#ifndef P2P_TECHDEMO_LIB_CORE_IO_IP_ADDRESS_H_
#define P2P_TECHDEMO_LIB_CORE_IO_IP_ADDRESS_H_

#include <array>

struct IpAddress
{
  public:
    IpAddress(uint32_t a, uint32_t b, uint32_t c, uint32_t d, bool is_v6 = false);
    explicit IpAddress(const std::string &str);

    bool
    operator!=(const IpAddress &ip_address) const;

    bool
    operator==(const IpAddress &ip_address) const;

    explicit operator std::string() const;

    void
    Clear();

    const uint8_t *
    GetIPv4() const;

    const uint8_t *
    GetIPv6() const;

    inline IpAddress()
    {
        Clear();
    }

    [[nodiscard]] inline bool
    IsIpv4() const
    {
        return (field32_[0] == 0 && field32_[1] == 0 && field16_[4] == 0 && field16_[5] == 0xffff);
    }

    [[nodiscard]] inline bool
    IsValid() const
    {
        return valid_;
    }

    [[nodiscard]] inline bool
    IsWildcard() const
    {
        return wildcard_;
    }

    inline void
    SetIpv4(const uint8_t (&ip)[4])
    {
        Clear();
        valid_ = true;
        field16_[5] = 0xffff;
        field32_[3] = reinterpret_cast<const uint32_t &>(ip);
    }

    inline void
    SetIpv6(const uint8_t (&ip)[16])
    {
        Clear();
        valid_ = true;
        for (auto i = 0; i < 16; ++i) {
            field8_[i] = ip[i];
        }
    }

    inline void
    SetIpv6(const std::array<uint8_t, 16> &host)
    {
        Clear();
        valid_ = true;
        for (auto i = 0; i < 16; ++i) {
            field8_[i] = host.at(i);
        }
    }

  private:
    union {
        uint32_t field32_[4];
        uint16_t field16_[8];
        uint8_t field8_[16];
    };
    bool valid_;
    bool wildcard_;
};

#endif // P2P_TECHDEMO_LIB_CORE_IO_IP_ADDRESS_H_
