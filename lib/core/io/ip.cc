#include <list>

#include <cstring>
#include <map>
#include <mutex>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <thread>
#include <vector>
#include <condition_variable>

#include "lib/core/singleton.h"

#include "ip.h"

namespace
{
    constexpr int GETADDRINFO_SUCCESS = 0;
    constexpr int RESOLVER_MAX_QUERIES = 32;
}

namespace
{
    IpAddress
    _sockaddr2ip(struct sockaddr &addr)
    {
        IpAddress ip;

        if (addr.sa_family == AF_INET)
        {
            auto &addr_in = reinterpret_cast<struct sockaddr_in &>(addr);

            auto octet1 = static_cast<uint8_t>(addr_in.sin_addr.s_addr >> 24);
            auto octet2 = static_cast<uint8_t>(addr_in.sin_addr.s_addr >> 16);
            auto octet3 = static_cast<uint8_t>(addr_in.sin_addr.s_addr >> 8);
            auto octet4 = static_cast<uint8_t>(addr_in.sin_addr.s_addr);

            ip.set_ipv4({octet1, octet2, octet3, octet4});
        }
        else if (addr.sa_family == AF_INET6)
        {
            auto &addr_in6 = reinterpret_cast<struct sockaddr_in6 &>(addr);

            ip.set_ipv6(addr_in6.sin6_addr.s6_addr);
        }

        return ip;
    }

    IpAddress
    _resolve_hostname(const std::string &hostname, IP::Type type)
    {
        struct addrinfo hints;
        struct addrinfo *results;

        memset(&hints, 0, sizeof(hints));

        if (type == IP::Type::V4)
        {
            hints.ai_family = AF_INET;
        }
        else if (type == IP::Type::V6)
        {
            hints.ai_family = AF_INET6;
            hints.ai_flags = 0;
        }
        else
        {
            hints.ai_family = AF_UNSPEC;
            hints.ai_flags = AI_ADDRCONFIG;
        }

        hints.ai_flags &= ~AI_NUMERICHOST;

        auto s = getaddrinfo(hostname.c_str(), nullptr, &hints, &results);

        if (s != GETADDRINFO_SUCCESS)
        {
            // TODO: logging
            // ...

            return IpAddress();
        }

        if (results == nullptr || results->ai_addr == nullptr)
        {
            // TODO: logging
            // ...

            if (results)
            {
                freeaddrinfo(results);
            }

            return IpAddress();
        }

        IpAddress ip = _sockaddr2ip(*(results->ai_addr));

        freeaddrinfo(results);

        return ip;
    }
}

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

    std::vector<QueueItem> queue;
    bool is_queued;

    IP::ResolverID
    find_empty_id() const
    {
        for (int i = 0; i < RESOLVER_MAX_QUERIES; i++)
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
        for (auto &q : queue)
        {
            if (q.status != IP::ResolverStatus::WAITING)
            {
                continue;
            }

            q.response = core::Singleton<IP>::Instance().resolve_hostname(q.hostname, q.type);

            if (!q.response.is_valid())
            {
                q.status = IP::ResolverStatus::ERROR;
            }
            else
            {
                q.status = IP::ResolverStatus::DONE;
            }
        }

        is_queued = false;
    }

    std::map<std::string, IpAddress> cache;

    static std::string get_cache_key(const std::string &hostname, IP::Type type)
    {
        return std::to_string(static_cast<int>(type)) + hostname;
    }

    IpResolver() : queue(RESOLVER_MAX_QUERIES), is_queued(false) {}
}; // struct IpResolver

IpAddress
IP::resolve_hostname(const std::string &hostname, IP::Type type)
{
    auto lk = std::lock_guard<std::mutex>(_resolver->mtx);

    std::string key = IpResolver::get_cache_key(hostname, type);

    if (_resolver->cache.count(key) && _resolver->cache[key].is_valid())
    {
        IpAddress ip_addr = _resolver->cache[key];

        return ip_addr;
    }

    IpAddress ip_addr = _resolve_hostname(hostname, type);
    _resolver->cache[key] = ip_addr;

    return ip_addr;
}

IP::ResolverID
IP::resolve_hostname_queue_item(const std::string &hostname, IP::Type type)
{
    auto lk = std::lock_guard<std::mutex>(_resolver->mtx);

    auto id = _resolver->find_empty_id();

    if (id == IP::RESOLVER_INVALID_ID)
    {
        // TODO: logging
        // ...

        return id;
    }

    std::string key = IpResolver::get_cache_key(hostname, type);

    _resolver->queue[id].hostname = hostname;
    _resolver->queue[id].type = type;

    if (_resolver->cache.count(key) && _resolver->cache[key].is_valid())
    {
        _resolver->queue[id].response = _resolver->cache[key];
        _resolver->queue[id].status = IP::ResolverStatus::NONE;
    }
    else
    {
        _resolver->queue[id].response = IpAddress();
        _resolver->queue[id].status = IP::ResolverStatus::WAITING;

        _resolver->is_queued = true;

        _resolver->cv.notify_all();
    }

    return id;
}

IP::ResolverStatus
IP::get_resolve_item_status(IP::ResolverID id) const
{
    // TODO: bounds checking
    // ...

    auto lk = std::lock_guard<std::mutex>(_resolver->mtx);

    if (_resolver->queue[id].status == IP::ResolverStatus::NONE)
    {
        // TODO: logging
        // ...

        return IP::ResolverStatus::NONE;
    }

    IP::ResolverStatus ret = _resolver->queue[id].status;

    return ret;
}

IpAddress
IP::get_resolve_item_address(IP::ResolverID id) const
{
    // TODO: bounds checking
    // ...

    auto lk = std::lock_guard<std::mutex>(_resolver->mtx);

    if (_resolver->queue[id].status != IP::ResolverStatus::DONE)
    {
        // TODO: logging
        // ...

        return IpAddress();
    }

    IpAddress ret = _resolver->queue[id].response;

    return ret;
}

void
IP::erase_resolve_item(IP::ResolverID id)
{
    // TODO: bounds checking
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
        _resolver->cache.erase(IpResolver::get_cache_key(hostname, Type::V4));
        _resolver->cache.erase(IpResolver::get_cache_key(hostname, Type::V6));
        _resolver->cache.erase(IpResolver::get_cache_key(hostname, Type::ANY));
    }
}

IP::IP()
{
    _resolver = std::make_shared<IpResolver>();

    _resolver->thread = std::thread{
        [this] {
            auto lk = std::unique_lock<std::mutex>(this->_resolver->mtx);
            this->_resolver->cv.wait(lk, [this] {
                return this->_resolver->is_queued;
            });
            this->_resolver->resolve_queues();
            this->_resolver->is_queued = false;
        }
    };
}

IP::~IP()
{
    if (_resolver->thread.joinable())
    {
        _resolver->thread.join();
    }
}
