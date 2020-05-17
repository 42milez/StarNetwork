#ifndef P2P_TECHDEMO_LIB_CORE_IO_IP_H_
#define P2P_TECHDEMO_LIB_CORE_IO_IP_H_

#include <list>
#include <memory>
#include <string>
#include <vector>

#include "ip_address.h"

namespace core
{
    struct IpResolver;

    class IP
    {
      public:
        enum class ResolverStatus : int
        {
            NONE    = 0,
            WAITING = 1,
            DONE    = 2,
            ERROR   = 3
        };

        enum class Type : int
        {
            NONE = 0,
            V4   = 1,
            V6   = 2,
            ANY  = 3
        };

        using ResolverID = int;

      public:
        IP();
        ~IP();

        void
        ClearCache(const std::string &hostname = "");

        void
        EraseResolveItem(ResolverID id);

        IpAddress
        ResolveHostname(const std::string &hostname, Type type = Type::ANY);

        ResolverID
        ResolveHostnameQueueItem(const std::string &hostname, Type type = Type::ANY);

        IpAddress
        ResolveItemAddress(ResolverID id) const;

        ResolverStatus
        ResolveItemStatus(ResolverID id) const;

      private:
        std::shared_ptr<IpResolver> resolver_;
    };
} // namespace core

#endif // P2P_TECHDEMO_LIB_CORE_IO_IP_H_
