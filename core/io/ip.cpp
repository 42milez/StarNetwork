#include <list>
#include <map>
#include <mutex>
#include <thread>
#include <vector>

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

        std::condition_variable cv;
        std::mutex mtx;
        std::thread thread;

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

//        static void _thread_function(void *self) {
//
//            IpResolver *ipr = (_IP_ResolverPrivate *)self;
//
//            while (!ipr->thread_abort) {
//
//                ipr->sem->wait();
//
//                ipr->mutex->lock();
//                ipr->resolve_queues();
//                ipr->mutex->unlock();
//            }
//        }

        std::map<std::string, IpAddress> cache;

        static std::string get_cache_key(const std::string &hostname, IP::Type type)
        {
            return std::to_string(static_cast<int>(type)) + hostname;
        }
    }; // struct IpResolver

    IpAddress
    IP::resolve_hostname(const std::string &hostname, IP::Type type)
    {
        auto lk = std::lock_guard<std::mutex>(_resolver->mtx);

        std::string key = IpResolver::get_cache_key(hostname, type);

        if (_resolver->cache->count(key) && _resolver->cache[key].is_valid())
        {
            IpAddress ip_addr = _resolver->cache[key];

            return ip_addr;
        }

        IpAddress ip_addr = _resolve_hostname(hostname, type);
        _resolver->cache[key] = ip_addr;

        return ip_addr;
    }

    IP::ResolverID
    IP::resolve_hostname_queue_item(const std::string &hostname, core::io::IP::Type type)
    {
        auto lk = std::lock_guard<std::mutex>(_resolver->mtx);

        auto id = _resolver->find_empty_id();

        if (id == IP::RESOLVER_INVALID_ID)
        {
            // ToDo: logging
            // ...

            return id;
        }

        std::string key = IpResolver::get_cache_key(hostname, type);

        _resolver->queue[id].hostname = hostname;
        _resolver->queue[id].type = type;

        if (_resolver->cache->count(key) && _resolver->cache[key].is_valid)
        {
            _resolver->queue[id].response = _resolver->cache[key];
            _resolver->queue[id].status = IP::ResolverStatus::NONE;
        }
        else
        {
            _resolver->queue[id].response = IpAddress();
            _resolver->queue[id].status = IP::ResolverStatus::WAITING;

            _resolver->cv.notify_all();
        }

        return id;
    }

    IP::ResolverStatus
    IP::get_resolve_item_status(core::io::IP::ResolverID id) const
    {
        // ToDo: bounds checking
        // ...

        auto lk = std::lock_guard<std::mutex>(_resolver->mtx);

        if (_resolver->queue[id].status == IP::ResolverStatus::NONE)
        {
            // ToDo: logging
            // ...

            return IP::ResolverStatus::NONE;
        }

        IP::ResolverStatus ret = _resolver->queue[id].status;

        return ret;
    }

    IpAddress
    IP::get_resolve_item_address(IP::ResolverID id) const
    {
        // ToDo: bounds checking
        // ...

        auto lk = std::lock_guard<std::mutex>(_resolver->mtx);

        if (_resolver->queue[id].status != IP::ResolverStatus::DONE)
        {
            // ToDo: logging
            // ...

            return IpAddress();
        }

        IpAddress ret = _resolver->queue[id].response;

        return ret;
    }

    void
    IP::erase_resolve_item(IP::ResolverID id)
    {
        // ToDo: bounds checking
        // ...

        auto lk = std::lock_guard<std::mutex>(_resolver->mtx);

        _resolver->queue[id].status = IP::ResolverStatus::NONE;
    }

    void
    IP::clear_cache(const std::string &hostname)
    {
        auto lk = std::lock_guard<std::mutex>(_resolver->mtx);

        if (hostname.empty())
        {
            _resolver->cache.clear();
        }
        else
        {
            _resolver->cache.erase(IpResolver::get_cache_key(hostname, Type::NONE));
            _resolver->cache.erase(IpResolver::get_cache_key(hostname, Type::IPV4));
            _resolver->cache.erase(IpResolver::get_cache_key(hostname, Type::IPV6));
            _resolver->cache.erase(IpResolver::get_cache_key(hostname, Type::ANY));
        }
    }

    std::vector<IpAddress>
    IP::_get_local_addresses() const
    {
        std::vector<IpAddress> addresses;
        std::list<IpAddress> ip_addresses;

        get_local_addresses(ip_addresses);

        for (auto const &ip_address : ip_addresses)
        {
            addresses.push_back(ip_address);
        }

        return addresses;
    }

    IP::IP()
    {
        _resolver = std::make_shared<IpResolver>();

        _resolver->thread = std::thread{
            [this]{
                auto lk = std::unique_lock<std::mutex>(this->_resolver->mtx);
                this->_resolver->cv.wait(lk, [this]{
                    this->_resolver->resolve_queues();
                });
            }
        };
    }
}} // namespace core / io
