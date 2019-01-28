#include <map>
#include <mutex>

#include "core/base/singleton.h"

#include "ip.h"

namespace core { namespace io
{
  struct IpResolver
  {
    struct QueueItem
    {
      volatile IP::ResolverStatus status;

      IpAddress response;

      std::string hostname;

      IP::Type type;

      void
      clear()
      {
        status = IP::ResolverStatus::DONE;
        response = IpAddress();
        type = IP::Type::NONE;
        hostname = "";
      };

      QueueItem()
      {
        clear();
      };
    }; // struct QueueItem

    QueueItem queue[IP::RESOLVER_MAX_QUERIES];

    IP::ResolverID
    find_empty_id() const
    {
      for (int i = 0; i < IP::RESOLVER_MAX_QUERIES; i++)
      {
        if (queue[i].status == IP::ResolverStatus::DONE)
        {
          return i;
        }
      }
      return IP::RESOLVER_INVALID_ID;
    }

    std::mutex mtx;

    std::unique_ptr<std::thread> thread;

    bool thread_abort = false;

    void
    resolve_queues()
    {
      for (auto i = 0; i < IP::RESOLVER_MAX_QUERIES; i++)
      {
        if (queue[i].status != IP::ResolverStatus::WAITING)
        {
          continue;
        }

        queue[i].response = core::base::Singleton<IP>::Instance().resolve_hostname(queue[i].hostname, queue[i].type);

        if (!queue[i].response.is_valid())
        {
          queue[i].status = IP::ResolverStatus::ERROR;
        }
        else
        {
          queue[i].status = IP::ResolverStatus::DONE;
        }
      }
    }

//    static void thread_function(std::shared_ptr<IpResolver> &resolver)
//    {
//      while (!resolver->thread_abort)
//      {
//        resolver->mtx->lock();
//        resolver->resolve_queues();
//        resolver->mtx.unlock();
//      }
//    }

    std::map<std::string, IpAddress> cache;

    static std::string get_cache_key(const std::string &hostname, IP::Type type)
    {
      return std::to_string(type) + hostname;
    }
  }; // struct IpResolver

  IpAddress IP::resolve_hostname(const std::string &hostname, IP::Type type)
  {

  }

  IP::IP()
  {
    _resolver = std::make_shared<IpResolver>();
    _resolver->thread = std::make_unique<std::thread>([resolver = _resolver] {
      while (!resolver->thread_abort)
      {
        std::unique_lock(resolver->mtx);
        resolver->resolve_queues();
      }
    });
  }

  IP::~IP()
  {
    _resolver->thread->join();
  }
}} // namespace core / io
