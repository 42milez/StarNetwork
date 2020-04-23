#include <algorithm>
#include <cstdlib>
#include <sstream>

#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>

#include "ip_address.h"
#include "lib/core/string.h"

namespace
{
    void
    ParseHex(const std::string &str, int start, uint8_t *dst)
    {
        uint16_t ret = 0;

        for (auto i = start; i < start + 4; i++) {
            if (i >= str.length()) {
                break;
            }

            auto n = 0;
            auto c = str[i];

            if (c >= '0' && c <= '9') {
                n = c - '0';
            }
            else if (c >= 'a' && c <= 'f') {
                n = 10 + (c - 'a');
            }
            else if (c >= 'A' && c <= 'F') {
                n = 10 + (c - 'A');
            }
            else if (c == ':') {
                break;
            }
            else {
                // TODO: logging
                // ...

                // TODO: handle error
                // ...
            }

            ret = ret << 4;
            ret += n;
        }

        dst[0] = ret >> 8;
        dst[1] = ret & 0xff;
    }

    void
    ParseIpv4(const std::string &str, int start, uint8_t *ret)
    {
        std::string ip;

        if (start != 0) {
            ip = str.substr(start, str.length() - start);
        }
        else {
            ip = str;
        }

        auto slices = std::count(ip.begin(), ip.end(), '.');

        if (slices != 3) {
            // TODO: logging
            // ...

            // TODO: handle error
            // ...
        }

        std::vector<std::string> octets;
        boost::algorithm::split(octets, ip, boost::is_any_of("."));

        for (auto i = 0; i < 4; i++) {
            ret[i] = std::stoi(octets[i]);
        }
    }

    void
    ParseIpv6(const std::string &str, uint16_t (&field16)[8])
    {
        static const int parts_total = 8;
        int parts[parts_total]       = {0};
        int parts_count              = 0;
        bool part_found              = false;
        bool part_skip               = false;
        bool part_ipv4               = false;
        int parts_idx                = 0;

        for (auto i = 0; i < str.length(); i++) {
            auto c = str[i];

            if (c == ':') {
                if (i == 0) {
                    continue; // next must be a ':'
                }

                if (!part_found) {
                    part_skip          = true;
                    parts[parts_idx++] = -1;
                }

                part_found = false;
            }
            else if (c == '.') {
                part_ipv4 = true;
            }
            else if ((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F')) {
                if (!part_found) {
                    parts[parts_idx++] = i;
                    part_found         = true;
                    ++parts_count;
                }
            }
            else {
                // TODO: logging
                // ...

                // TODO: handle error
                // ...

                return;
            }
        }

        int parts_extra = 0;

        if (part_skip) {
            parts_extra = parts_total - parts_count;
        }

        int idx = 0;

        for (auto i = 0; i < parts_idx; i++) {
            if (parts[i] == -1) {
                for (auto j = 1; j < parts_extra; j++) {
                    field16[idx++] = 0;
                }
                continue;
            }

            if (part_ipv4 && i == parts_idx - 1) {
                ParseIpv4(str, parts[i], reinterpret_cast<uint8_t *>(&field16[idx])); // should be the last one
            }
            else {
                ParseHex(str, parts[i], reinterpret_cast<uint8_t *>(&field16[idx++]));
            }
        }
    }

    void
    SplitUint32IntoUint8(uint8_t *dst, uint32_t n)
    {
        dst[0] = (n >> 24) & 0xff;
        dst[1] = (n >> 16) & 0xff;
        dst[2] = (n >> 8) & 0xff;
        dst[3] = (n >> 0) & 0xff;
    }
} // namespace

IpAddress::operator std::string() const
{
    if (!valid_) {
        return "";
    }

    if (IsIpv4()) {
        return std::to_string(field8_[12]) + "." + std::to_string(field8_[13]) + "." + std::to_string(field8_[14]) +
               "." + std::to_string(field8_[15]);
    }

    std::stringstream ret;

    for (auto i = 0; i < 8; i++) {
        if (i > 0) {
            ret << ":";
        }

        // TODO: try to use field16_
        uint16_t num = (field8_[i * 2] << 8) + field8_[i * 2 + 1];

        ret << std::hex << num;
    }

    return ret.str();
}

bool
IpAddress::operator!=(const IpAddress &ip_address) const
{
    if (valid_ != ip_address.valid_) {
        return true;
    }

    if (!valid_) {
        return true;
    }

    for (auto i = 0; i < 4; i++) {
        if (ip_address.field32_[i] != field32_[i]) {
            return true;
        }
    }

    return false;
}

bool
IpAddress::operator==(const IpAddress &ip_address) const
{
    if (valid_ != ip_address.valid_) {
        return false;
    }

    if (!valid_) {
        return false;
    }

    for (auto i = 0; i < 4; i++) {
        if (ip_address.field32_[i] != field32_[i]) {
            return false;
        }
    }

    return true;
}

void
IpAddress::Clear()
{
    memset(&field8_[0], 0, sizeof(field8_));
    valid_    = false;
    wildcard_ = false;
}

const uint8_t *
IpAddress::GetIPv4() const
{
    // TODO: bounds checking
    // ...

    return &(field8_[12]);
}

const uint8_t *
IpAddress::GetIPv6() const
{
    // TODO: bounds checking
    // ...

    return field8_;
}

IpAddress::IpAddress(const std::string &str)
{
    Clear();

    if (str == "*") {
        wildcard_ = true;
    }
    else if (str.find(':') != std::string::npos) {
        // IPv6
        ParseIpv6(str, field16_);
        valid_ = true;
    }
    else if (std::count(str.begin(), str.end(), '.') == 3) {
        // IPv4 (mapped to IPv6 internally)
        field16_[5] = 0xffff;
        ParseIpv4(str, 0, &field8_[12]);
        valid_ = true;
    }
    else {
        // TODO: logging
        // ...
    }
}

IpAddress::IpAddress(uint32_t a, uint32_t b, uint32_t c, uint32_t d, bool is_v6)
{
    Clear();

    valid_ = true;

    if (!is_v6) {
        field16_[5] = 0xffff;
        field8_[12] = a;
        field8_[13] = b;
        field8_[14] = c;
        field8_[15] = d;
    }
    else {
        SplitUint32IntoUint8(&field8_[0], a);
        SplitUint32IntoUint8(&field8_[4], b);
        SplitUint32IntoUint8(&field8_[8], c);
        SplitUint32IntoUint8(&field8_[12], d);
    }
}
