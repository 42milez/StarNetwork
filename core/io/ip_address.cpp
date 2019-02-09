#include <algorithm>
#include <sstream>

#include "ip_address.h"

namespace core { namespace io
{
    std::string
    IpAddress::to_string() const
    {
        if (!_valid)
        {
            return "";
        }

        if (is_ipv4())
        {
            return std::to_string(static_cast<int>(std::get<IP128_8>(_field)[12])) + "." +
                   std::to_string(static_cast<int>(std::get<IP128_8>(_field)[13])) + "." +
                   std::to_string(static_cast<int>(std::get<IP128_8>(_field)[14])) + "." +
                   std::to_string(static_cast<int>(std::get<IP128_8>(_field)[15]));
        }

        std::stringstream ret;

        for (auto i = 0; i < 8; i++)
        {
            if (i > 0)
            {
                ret << ":";
            }

            // ToDo: Try to use IP128_16
            uint16_t num = (std::get<IP128_8>(_field)[i * 2] << 8) + std::get<IP128_8>(_field)[i * 2 + 1];

            ret << std::hex << num;
        }

        return ret.str();
    }

    static void
    _parse_hex(const std::string &str, int start, uint8_t *dst)
    {
        uint16_t ret = 0;

        for (auto i = start; i < start + 4; i++)
        {
            if (i >= str.length())
            {
                break;
            }

            auto n = 0;
            auto c = str[i];

            if (c >= '0' && c <= '9')
            {
                n = c - '0';
            }
            else if (c >= 'a' && c <= 'f')
            {
                n = 10 + (c - 'a');
            }
            else if (c >= 'A' && c <= 'F')
            {
                n = 10 + (c - 'A');
            }
            else if (c == ':')
            {
                break;
            }
            else
            {
                // ToDo: logging
                // ...
            }

            ret = ret << 4;
            ret += n;
        }

        dst[0] = ret >> 8;
        dst[1] = ret & 0xff;
    }

    void
    IpAddress::_parse_ipv6(const std::string &str)
    {
        static const int parts_total = 8;
        int parts[parts_total] = {0};
        int parts_count = 0;
        bool part_found = false;
        bool part_skip = false;
        bool part_ipv4 = false;
        int parts_idx = 0;

        for (auto i = 0; i < str.length(); i++)
        {
            auto c = str[i];

            if (c == ':')
            {
                if (i == 0)
                {
                    continue;
                }

                if (!part_found)
                {
                    part_skip = true;
                    parts[parts_idx++] = -1;
                }

                part_found = false;
            }
            else if (c == '.')
            {
                part_ipv4 = true;
            }
            else if ((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F'))
            {
                if (!part_found)
                {
                    parts[parts_idx++] = i;
                    part_found = true;
                    ++parts_count;
                }
            }
            else
            {
                // ToDo: logging
                // ...

                // ToDo: error handling
                // ...

                return;
            }
        }

        int parts_extra = 0;

        if (part_skip)
        {
            parts_extra = parts_total - parts_count;
        }

        int idx = 0;

        for (auto i = 0; i < parts_idx; i++)
        {
            if (parts[i] == -1)
            {
                for (auto j = 0; j < parts_extra; j++)
                {
                    std::get<IP128_16>(_field)[idx++] = 0;
                }
                continue;
            }

            if (part_ipv4 && i == parts_idx - 1)
            {
                _parse_ipv4(str, parts[i], (uint8_t *)&std::get<IP128_16>(_field)[idx]);
            }
            else
            {
                _parse_hex(str, parts[i], (uint8_t *)&std::get<IP128_16>(_field)[idx++]);
            }
        }
    }

    IpAddress::IpAddress(const std::string &str)
    {
        clear();

        if (str == "*")
        {
            _wildcard = true;
        }
        else if (str.find(':') >= 0)
        {
            // IPv6
            _parse_ipv6(str);
            _valid = true;
        }
        else if (std::count(str.begin(), str.end(), '.') == 4)
        {
            // IPv4 (mapped to IPv6 internally)
            std::get<IP128_16>(_field)[5] = 0xffff;
            _valid = true;
        }
        else
        {
            // ToDo: logging
            // ...
        }
    }
}} // namespace core /io
