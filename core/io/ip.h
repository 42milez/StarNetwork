#ifndef P2P_TECHDEMO_CORE_IO_IP_H
#define P2P_TECHDEMO_CORE_IO_IP_H

#include <list>
#include <memory>
#include <string>
#include <vector>

#include "ip_address.h"

namespace core { namespace io
{
  struct IpResolver;

  class IP
  {
  public:
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
      IPV4 = 1,
      IPV6 = 2,
      ANY = 3
    };

    static constexpr int RESOLVER_MAX_QUERIES = 32;
    static constexpr int RESOLVER_INVALID_ID = -1;

    using ResolverID = int;

  public:
    IpAddress resolve_hostname(const std::string &hostname, Type type = Type::ANY);

    ResolverID resolve_hostname_queue_item(const std::string &hostname, Type type = Type::ANY);

    ResolverStatus get_resolve_item_status(ResolverID id) const;

    IpAddress get_resolve_item_address(ResolverID id) const;

    virtual void get_local_addresses(std::list<IpAddress> &addresses) const = 0;

    void erase_resolve_item(ResolverID id);

    void clear_cache(const std::string &hostname = "");

    IP();

    ~IP();

  protected:
    virtual IpAddress _resolve_hostname(const std::string &hostname, Type type = Type::ANY) = 0;

  private:
    std::shared_ptr<IpResolver> _resolver;
  };
}} // namespace core / io

#endif // P2P_TECHDEMO_CORE_IO_IP_H
