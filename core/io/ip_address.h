#ifndef P2P_TECHDEMO_CORE_IO_IP_ADDRESS_H
#define P2P_TECHDEMO_CORE_IO_IP_ADDRESS_H

namespace core { namespace io
{
    struct IpAddress
    {
    private:
        union
        {
            uint8_t _field8[16];
            uint16_t _field16[8];
            uint32_t _field32[4];
        };

        bool _valid;
        bool _wildcard;

    protected:
        void _parse_ipv6(const std::string &str);
        void _parse_ipv4(const std::string &str, int start, uint8_t *ret);

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

            for (auto i = 0; i < 4; i++)
            {
                if (ip_address._field32[i] != _field32[i])
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

            for (auto i = 0; i < 4; i++)
            {
                if (ip_address._field32[i] != _field32[i])
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

        const uint8_t *get_ipv4() const;

        const uint8_t *get_ipv6() const;

        void set_ipv4(const uint8_t (&ip)[4]);

        void set_ipv6(const uint8_t (&ip)[16]);

        std::string to_string() const;

        IpAddress(const std::string &str);

        IpAddress(uint32_t a, uint32_t b, uint32_t c, uint32_t d, bool is_v6 = false);

        IpAddress() { clear(); }
    };
}} // namespace core / io

#endif // P2P_TECHDEMO_CORE_IO_IP_ADDRESS_H
