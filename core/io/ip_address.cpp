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

            return std::to_string(static_cast<integer>(std::get<IP128_8>(_field)[12])) +
                   std::to_string(static_cast<integer>(std::get<IP128_8>(_field)[13])) +
                   std::to_string(static_cast<integer>(std::get<IP128_8>(_field)[14])) +
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
}} // namespace core /io
