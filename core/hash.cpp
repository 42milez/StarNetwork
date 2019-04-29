#include <string>

#include <uuid/uuid.h>

#include "hash.h"

uint32_t
hash32()
{
    uuid_t in;
    uuid_string_t out;

    uuid_generate_time(in);
    uuid_unparse(in, out);

    return std::hash<std::string>{}(std::string(out));
}
