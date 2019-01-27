#ifndef P2P_TECHDEMO_CORE_IO_IP_H
#define P2P_TECHDEMO_CORE_IO_IP_H

#include <list>
#include <string>
#include <vector>

namespace core { namespace io
{
  struct _IP_ResolverPrivate;

  class IP
  {
  public:
    enum class ResolverStatus : int
    {
      RESOLVER_STATUS_NONE = 0,
      RESOLVER_STATUS_WAITING = 1,
      RESOLVER_STATUS_DONE = 2,
      RESOLVER_STATUS_ERROR = 3
    };

    enum class Type : int
    {
      TYPE_NONE = 0,
      TYPE_IPV4 = 1,
      TYPE_IPV6 = 2,
      TYPE_ANY = 3
    };

    constexpr int RESOLVER_MAX_QUERIES = 32;

    constexpr int RESOLVER_INVALID_ID = -1;

    using ResolverID = int;

  private:
    _IP_ResolvePrivate *resolver;

  protected:
    static IP *singleton;

    static void _bind_methods();

    virtual IP_Address _resolve_hostname(const std::string &hostname, Type type = Type::TYPE_ANY) = 0;

    std::vector _get_local_addresses() const;

    static IP *(*_create)();

  public:
    static IP *get_singleton();

    static IP *create();

    IP_Address resolve_hostname(const std::string &hostname, Type type = Type::TYPE_ANY);

    ResolverID resolve_hostname_queue_item(const std::string &hostname, Type type = TYPE_ANY);

    ResolverStatus get_resolve_item_status(ResolverID id) const;

    IP_Address get_resolve_item_address(ResolverID id) const;

    virtual void get_local_addresses(std::list<IP_Address> &addresses) const = 0;

    void erase_resolve_item(ResolverID id);

    void clear_cache(const std::string &hostname = "");

    IP();

    ~IP();
  };
}} // namespace core / io

#endif // P2P_TECHDEMO_CORE_IO_IP_H
