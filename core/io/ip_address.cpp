#include <algorithm>
#include <cstdlib>
#include <sstream>

#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>

#include "core/string.h"
#include "ip_address.h"

namespace
{
    void
    _32_to_buf(uint8_t *dst, uint32_t n)
    {
        dst[0] = (n >> 24) & 0xff;
        dst[1] = (n >> 16) & 0xff;
        dst[2] = (n >> 8) & 0xff;
        dst[3] = (n >> 0) & 0xff;
    }

    void
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

                // ToDo: error handling
                // ...
            }

            ret = ret << 4;
            ret += n;
        }

        dst[0] = ret >> 8;
        dst[1] = ret & 0xff;
    }
}

IpAddress::operator std::string() const
{
    if (!_valid)
    {
        return "";
    }

    if (is_ipv4())
    {
        return std::to_string(_field8[12]) + "." +
               std::to_string(_field8[13]) + "." +
               std::to_string(_field8[14]) + "." +
               std::to_string(_field8[15]);
    }

    std::stringstream ret;

    for (auto i = 0; i < 8; i++)
    {
        if (i > 0)
        {
            ret << ":";
        }

        // ToDo: Try to use _field16
        uint16_t num = (_field8[i * 2] << 8) + _field8[i * 2 + 1];

        ret << std::hex << num;
    }

    return ret.str();
}

void
IpAddress::_parse_ipv4(const std::string &str, int start, uint8_t *ret)
{
    std::string ip;

    if (start != 0)
    {
        ip = str.substr(start, str.length() - start);
    }
    else
    {
        ip = str;
    }

    auto slices = std::count(ip.begin(), ip.end(), '.');

    if (slices != 3)
    {
        // ToDo: logging
        // ...

        // ToDo: error handling
        // ...
    }

    std::vector<std::string> octets;
    boost::algorithm::split(octets, ip, boost::is_any_of("."));

    for (auto i = 0; i < 4; i++)
    {
        ret[i] = std::stoi(octets[i]);
    }
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
                continue; // next must be a ':'
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
            for (auto j = 1; j < parts_extra; j++)
            {
                _field16[idx++] = 0;
            }
            continue;
        }

        if (part_ipv4 && i == parts_idx - 1)
        {
            _parse_ipv4(str, parts[i], reinterpret_cast<uint8_t *>(&_field16[idx])); // should be the last one
        }
        else
        {
            _parse_hex(str, parts[i], reinterpret_cast<uint8_t *>(&_field16[idx++]));
        }
    }
}

bool
IpAddress::operator!=(const IpAddress &ip_address) const
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

bool
IpAddress::operator==(const IpAddress &ip_address) const
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

void
IpAddress::clear()
{
    memset(&_field8[0], 0, sizeof(_field8));
    _valid = false;
    _wildcard = false;
}

bool
IpAddress::is_ipv4() const
{
    return (_field32[0] == 0 && _field32[1] == 0 && _field16[4] == 0 && _field16[5] == 0xffff);
}

bool
IpAddress::is_valid() const
{
    return _valid;
}

bool
IpAddress::is_wildcard() const
{
    return _wildcard;
}

const uint8_t *
IpAddress::GetIPv4() const
{
    // ToDo: bounds checking
    // ...

    return &(_field8[12]);
}

const uint8_t *
IpAddress::GetIPv6() const
{
    // ToDo: bounds checking
    // ...

    return _field8;
}

void
IpAddress::set_ipv4(const uint8_t (&ip)[4])
{
    clear();

    _valid = true;

    _field16[5] = 0xffff;
    _field32[3] = reinterpret_cast<const uint32_t &>(ip);
}

void
IpAddress::set_ipv6(const uint8_t (&ip)[16])
{
    clear();

    _valid = true;

    for (auto i = 0; i < 16; ++i)
    {
        _field8[i] = ip[i];
    }
}

void
IpAddress::set_ipv6(const std::array<uint8_t, 16> &host)
{
    clear();

    _valid = true;

    for (auto i = 0; i < 16; ++i)
    {
        _field8[i] = host.at(i);
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
    else if (std::count(str.begin(), str.end(), '.') == 3)
    {
        // IPv4 (mapped to IPv6 internally)
        _field16[5] = 0xffff;
        _parse_ipv4(str, 0, &_field8[12]);
        _valid = true;
    }
    else
    {
        // ToDo: logging
        // ...
    }
}

IpAddress::IpAddress(uint32_t a, uint32_t b, uint32_t c, uint32_t d, bool is_v6)
{
    clear();

    _valid = true;

    if (!is_v6)
    {
        _field16[5] = 0xffff;
        _field8[12] = a;
        _field8[13] = b;
        _field8[14] = c;
        _field8[15] = d;
    }
    else
    {
        _32_to_buf(&_field8[0], a);
        _32_to_buf(&_field8[4], b);
        _32_to_buf(&_field8[8], c);
        _32_to_buf(&_field8[12], d);
    }
}

IpAddress::IpAddress()
{
    clear();
}
