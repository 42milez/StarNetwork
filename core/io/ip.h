#ifndef P2P_TECHDEMO_CORE_IO_IP_H
#define P2P_TECHDEMO_CORE_IO_IP_H

#include <list>
#include <memory>
#include <string>
#include <vector>

#include "ip_address.h"

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

#endif // P2P_TECHDEMO_CORE_IO_IP_H
