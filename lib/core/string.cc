#include <boost/algorithm/string.hpp>

#include "string.h"

namespace core
{
    bool
    IsValidHexNumber(const std::string &str, bool with_prefix)
    {
        int len = str.size();

        if (len == 0) {
            return false;
        }

        int from = 0;

        if (len != 1 && (str[0] == '+' || str[0] == '-')) {
            from++;
        }

        if (with_prefix) {
            if (len < 3) {
                return false;
            }

            if (str[from] != '0' || str[from + 1] != 'x') {
                return false;
            }

            from += 2;
        }

        for (auto i = from; i < len; i++) {
            if ((str[i] >= '0' && str[i] <= '9') || (str[i] >= 'a' && str[i] <= 'f') || (str[i] >= 'A' && str[i] <= 'F'))
                continue;
            else {
                return false;
            }
        }

        return true;
    }

    bool
    IsValidInteger(const std::string &str)
    {
        int len = str.size();

        if (len == 0) {
            return false;
        }

        int from = 0;

        if (len != 1 && (str[0] == '+' || str[0] == '-')) {
            from++;
        }

        for (auto i = from; i < len; i++) {
            if (str[i] < '0' || str[i] > '9') {
                return false;
            }
        }

        return true;
    }

    bool
    IsValidIpAddress(const std::string &str)
    {
        if (str.find(':') != std::string::npos) {
            std::vector<std::string> octets;
            boost::algorithm::split(octets, str, boost::is_any_of(":"));

            for (auto &octet : octets) {
                if (octet.empty()) {
                    continue;
                }

                if (IsValidHexNumber(octet, false)) {
                    int nint = std::strtoul(octet.c_str(), nullptr, 16);
                    if (nint < 0 || nint > 0xffff) {
                        return false;
                    }
                    continue;
                }
                if (!IsValidIpAddress(octet)) {
                    return false;
                }
            }
        }
        else {
            std::vector<std::string> octets;
            boost::algorithm::split(octets, str, boost::is_any_of("."));

            if (octets.size() != 4) {
                return false;
            }

            for (auto &octet : octets) {
                if (!IsValidInteger(octet)) {
                    return false;
                }

                int val = std::stoi(octet);

                if (val < 0 || val > 255) {
                    return false;
                }
            }
        }

        return true;
    }
} // namespace core
