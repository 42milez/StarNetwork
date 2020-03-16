#include <string>

#include "network/constants.h"
#include "hash.h"
#include "os.h"
#include "singleton.h"

namespace core
{
    uint64_t
    hash64(std::string &&str)
    {

        /* simple djb2 hashing */

        const char *chr = str.c_str();
        uint64_t hashv  = 5381;
        uint64_t c;

        while ((c = *chr++))
            hashv = ((hashv << 5) + hashv) + c; /* hash * 33 + c */

        return hashv;
    }

    uint32_t
    hash_djb2_one_32(uint32_t p_in, uint32_t p_prev = 5381)
    {
        return ((p_prev << 5u) + p_prev) + p_in;
    }

    uint32_t
    Hash::uniqueID()
    {
        uint32_t hash = 0;

        while (hash == BROADCAST_ID || hash == SERVER_ID) {
            hash = hash_djb2_one_32((uint32_t)core::Singleton<OS>::Instance().GetTicksMsec());
            hash = hash_djb2_one_32((uint32_t)OS::GetUnixTime(), hash);
            hash = hash_djb2_one_32((uint32_t)hash64(OS::GetUserDataDir()), hash);
            hash = hash_djb2_one_32((uint32_t)((uint64_t)this), hash);  // Rely on ASLR heap
            hash = hash_djb2_one_32((uint32_t)((uint64_t)&hash), hash); // Rely on ASLR stack

            hash = hash & 0x7FFFFFFFu; // Make it compatible with unsigned, since negative ID is used for exclusion
        }

        return hash;
    }
} // namespace core
