#ifndef P2P_TECHDEMO_CORE_IO_IP_ADDRESS_H
#define P2P_TECHDEMO_CORE_IO_IP_ADDRESS_H

#include <variant>

namespace core { namespace io
{
    struct IpAddress
    {
    private:
        using IP128_8 = uint8_t[16];
        using IP128_16 = uint16_t[8];
        using IP128_32 = uint32_t[4];

        std::variant<IP128_8, IP128_16, IP128_32> _field;
        bool _valid;
        bool _wildcard;

    protected:
        void _parse_ipv6(const std::string &str);
        void _parse_ipv4(const std::string &str);

    public:
        bool operator==(const IpAddress &ip_address) const
        {
            if (_valid != ip_address._valid)
            {
                return false;
            }

            if (!_valid)
            {
                return false;
            }

            auto field_l = std::get<IP128_32>(_field);
            auto field_r = std::get<IP128_32>(ip_address._field);

            for (auto i = 0; i < 4; i++)
            {
                if (field_l[i] != field_r[i])
                {
                    return false;
                }
            }

            return true;
        }

        bool operator!=(const IpAddress &ip_address) const
        {
            if (_valid != ip_address._valid)
            {
                return true;
            }

            if (!_valid)
            {
                return true;
            }

            auto field_l = std::get<IP128_32>(_field);
            auto field_r = std::get<IP128_32>(ip_address._field);

            for (auto i = 0; i < 4; i++)
            {
                if (field_l[i] != field_r[i])
                {
                    return true;
                }
            }

            return false;
        }

        void clear();

        bool is_wildcard() const { return _wildcard; }

        bool is_valid() const { return _valid; }

        bool is_ipv4() const;

        const uint8_t &get_ipv4() const;

        void set_ipv4(const uint8_t *ip);

        const uint16_t &get_ipv6() const;

        void set_ipv6(const uint8_t &buf);

        std::string to_string() const;

        IpAddress(const std::string &str);

        IpAddress(uint32_t a, uint32_t b, uint32_t c, uint32_t d, bool is_v6 = false);

        IpAddress() { clear(); }
    };
}} // namespace core / io

#endif // P2P_TECHDEMO_CORE_IO_IP_ADDRESS_H
