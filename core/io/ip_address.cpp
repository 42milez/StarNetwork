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
            return std::to_string(static_cast<integer>(std::get<IP128_8>(_field)[12])) + "." +
                   std::to_string(static_cast<integer>(std::get<IP128_8>(_field)[13])) + "." +
                   std::to_string(static_cast<integer>(std::get<IP128_8>(_field)[14])) + "." +
                   std::to_string(static_cast<integer>(std::get<IP128_8>(_field)[15]));
        }

        std::stringstream ret;

        for (auto i = 0; i < 8; i++)
        {
            if (i > 0)
            {
                ret << ":";
            }

            uint16_t num = (std::get<IP128_8>(_field)[i * 2] << 8) + std::get<IP128_8>(_field)[i * 2 + 1];

            ret << std::hex << num;
        }

        return ret.str();
    }

    static void
    _parse_hex(const std::string &str, int start, uint8_t dst)
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
}} // namespace core /io
