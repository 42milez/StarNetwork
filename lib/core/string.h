#ifndef P2P_TECHDEMO_LIB_CORE_STRING_H_
#define P2P_TECHDEMO_LIB_CORE_STRING_H_

#include <string>

bool is_valid_hex_number(const std::string &str, bool with_prefix);
bool is_valid_integer(const std::string &str);
bool is_valid_ip_address(const std::string &str);

#endif // P2P_TECHDEMO_LIB_CORE_STRING_H_
