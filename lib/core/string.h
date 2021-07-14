#ifndef STAR_NETWORK_LIB_CORE_STRING_H_
#define STAR_NETWORK_LIB_CORE_STRING_H_

#include <string>

namespace core
{
    bool
    IsValidHexNumber(const std::string &str, bool with_prefix);

    bool
    IsValidInteger(const std::string &str);

    bool
    IsValidIpAddress(const std::string &str);
} // namespace core

#endif // STAR_NETWORK_LIB_CORE_STRING_H_
