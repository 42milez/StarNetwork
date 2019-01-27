#include <mutex>

#include "ip.h"

namespace core { namespace io
{
  struct _IP_ResolverPrivate
  {
    struct QueueItem
    {
      volatile IP::ResolverStatus status;

      IP_Address response;

      std::string hostname;

      IP::Type type;

      void
      clear()
      {
        status = IP::ResolverStatus::RESOLVER_STATUS_DONE;
        response = IP_Address();
        type = IP::Type::TYPE_NONE;
        hostname = "";
      };

      QueueItem()
      {
        clear();
      };
    };

    QueueItem queue[IP::RESOLVER_MAX_QUERIES];

    IP::ResolverID
    find_empty_id() const
    {
      for (int i = 0; i < IP::RESOLVER_MAX_QUERIES; i++)
      {
        if (queue[i].status == IP::ResolverStatus::RESOLVER_STATUS_DONE)
        {
          return i;
        }
      }
      return IP::RESOLVER_INVALID_ID;
    }

    std::mutex *mutex;


  };
}} // namespace core / io
