#ifndef P2P_TECHDEMO_LIB_CORE_IO_IP_H_
#define P2P_TECHDEMO_LIB_CORE_IO_IP_H_

#include <list>
#include <memory>
#include <string>
#include <vector>

#include "ip_address.h"

#define SPLIT_IPV4_TO_OCTET_INIT_LIST(sin_addr)      \
    {                                                \
        static_cast<uint8_t>(sin_addr.s_addr >> 24), \
        static_cast<uint8_t>(sin_addr.s_addr >> 16), \
        static_cast<uint8_t>(sin_addr.s_addr >> 8),  \
        static_cast<uint8_t>(sin_addr.s_addr)        \
    }

struct IpResolver;

class IP
{
private:
    std::shared_ptr<IpResolver> _resolver;

public:
    using ResolverID = int;

    enum class ResolverStatus : int
    {
        NONE = 0,
        WAITING = 1,
        DONE = 2,
        ERROR = 3
    };

    enum class Type : int
    {
        NONE = 0,
        V4 = 1,
        V6 = 2,
        ANY = 3
    };

    static constexpr int RESOLVER_INVALID_ID = -1;

public:
    void clear_cache(const std::string &hostname = "");

    void erase_resolve_item(ResolverID id);

    IpAddress resolve_hostname(const std::string &hostname, Type type = Type::ANY);

    ResolverID resolve_hostname_queue_item(const std::string &hostname, Type type = Type::ANY);

    ResolverStatus get_resolve_item_status(ResolverID id) const;

    IpAddress get_resolve_item_address(ResolverID id) const;

    IP();

    ~IP();
};

#endif // P2P_TECHDEMO_LIB_CORE_IO_IP_H_
