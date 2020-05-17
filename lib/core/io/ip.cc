#include <condition_variable>
#include <cstring>
#include <map>
#include <mutex>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <thread>
#include <vector>

#include "lib/core/singleton.h"

#include "ip.h"

namespace
{
    constexpr int GETADDRINFO_SUCCESS  = 0;
    constexpr int RESOLVER_INVALID_ID  = -1;
    constexpr int RESOLVER_MAX_QUERIES = 32;

    core::IpAddress
    _sockaddr2ip(struct sockaddr &addr)
    {
        core::IpAddress ip;

        if (addr.sa_family == AF_INET) {
            auto &addr_in = reinterpret_cast<struct sockaddr_in &>(addr);

            auto octet1 = static_cast<uint8_t>(addr_in.sin_addr.s_addr >> 24);
            auto octet2 = static_cast<uint8_t>(addr_in.sin_addr.s_addr >> 16);
            auto octet3 = static_cast<uint8_t>(addr_in.sin_addr.s_addr >> 8);
            auto octet4 = static_cast<uint8_t>(addr_in.sin_addr.s_addr);

            ip.SetIpv4({octet1, octet2, octet3, octet4});
        }
        else if (addr.sa_family == AF_INET6) {
            auto &addr_in6 = reinterpret_cast<struct sockaddr_in6 &>(addr);

            ip.SetIpv6(addr_in6.sin6_addr.s6_addr);
        }

        return ip;
    }

    core::IpAddress
    _resolve_hostname(const std::string &hostname, core::IP::Type type)
    {
        struct addrinfo hints;
        struct addrinfo *results;

        memset(&hints, 0, sizeof(hints));

        if (type == core::IP::Type::V4) {
            hints.ai_family = AF_INET;
        }
        else if (type == core::IP::Type::V6) {
            hints.ai_family = AF_INET6;
            hints.ai_flags  = 0;
        }
        else {
            hints.ai_family = AF_UNSPEC;
            hints.ai_flags  = AI_ADDRCONFIG;
        }

        hints.ai_flags &= ~AI_NUMERICHOST;

        auto s = getaddrinfo(hostname.c_str(), nullptr, &hints, &results);

        if (s != GETADDRINFO_SUCCESS) {
            // TODO: logging
            // ...

            return core::IpAddress();
        }

        if (results == nullptr || results->ai_addr == nullptr) {
            // TODO: logging
            // ...

            if (results) {
                freeaddrinfo(results);
            }

            return core::IpAddress();
        }

        auto ip = _sockaddr2ip(*(results->ai_addr));

        freeaddrinfo(results);

        return ip;
    }
} // namespace

namespace core
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
                status   = IP::ResolverStatus::DONE;
                response = IpAddress();
                type     = IP::Type::NONE;
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
            for (int i = 0; i < RESOLVER_MAX_QUERIES; i++) {
                if (queue[i].status == IP::ResolverStatus::DONE) {
                    return i;
                }
            }
            return RESOLVER_INVALID_ID;
        }

        std::condition_variable cv;
        std::mutex mtx;
        std::thread thread;

        void
        resolve_queues()
        {
            for (auto &q : queue) {
                if (q.status != IP::ResolverStatus::WAITING) {
                    continue;
                }

                q.response = core::Singleton<IP>::Instance().ResolveHostname(q.hostname, q.type);

                if (!q.response.IsValid()) {
                    q.status = IP::ResolverStatus::ERROR;
                }
                else {
                    q.status = IP::ResolverStatus::DONE;
                }
            }

            is_queued = false;
        }

        std::map<std::string, IpAddress> cache;

        static std::string
        get_cache_key(const std::string &hostname, IP::Type type)
        {
            return std::to_string(static_cast<int>(type)) + hostname;
        }

        IpResolver()
            : queue(RESOLVER_MAX_QUERIES)
            , is_queued(false)
        {
        }
    }; // struct IpResolver

    IP::IP()
    {
        resolver_ = std::make_shared<IpResolver>();

        resolver_->thread = std::thread{[this] {
            auto lk = std::unique_lock<std::mutex>(this->resolver_->mtx);
            this->resolver_->cv.wait(lk, [this] { return this->resolver_->is_queued; });
            this->resolver_->resolve_queues();
            this->resolver_->is_queued = false;
        }};
    }

    IP::~IP()
    {
        if (resolver_->thread.joinable()) {
            resolver_->thread.join();
        }
    }

    IpAddress
    IP::ResolveHostname(const std::string &hostname, IP::Type type)
    {
        auto lk = std::lock_guard<std::mutex>(resolver_->mtx);

        std::string key = IpResolver::get_cache_key(hostname, type);

        if (resolver_->cache.count(key) && resolver_->cache[key].IsValid()) {
            IpAddress ip_addr = resolver_->cache[key];

            return ip_addr;
        }

        IpAddress ip_addr     = _resolve_hostname(hostname, type);
        resolver_->cache[key] = ip_addr;

        return ip_addr;
    }

    IP::ResolverID
    IP::ResolveHostnameQueueItem(const std::string &hostname, IP::Type type)
    {
        auto lk = std::lock_guard<std::mutex>(resolver_->mtx);

        auto id = resolver_->find_empty_id();

        if (id == RESOLVER_INVALID_ID) {
            // TODO: logging
            // ...

            return id;
        }

        std::string key = IpResolver::get_cache_key(hostname, type);

        resolver_->queue[id].hostname = hostname;
        resolver_->queue[id].type     = type;

        if (resolver_->cache.count(key) && resolver_->cache[key].IsValid()) {
            resolver_->queue[id].response = resolver_->cache[key];
            resolver_->queue[id].status   = IP::ResolverStatus::NONE;
        }
        else {
            resolver_->queue[id].response = IpAddress();
            resolver_->queue[id].status   = IP::ResolverStatus::WAITING;

            resolver_->is_queued = true;

            resolver_->cv.notify_all();
        }

        return id;
    }

    IP::ResolverStatus
    IP::ResolveItemStatus(IP::ResolverID id) const
    {
        // TODO: bounds checking
        // ...

        auto lk = std::lock_guard<std::mutex>(resolver_->mtx);

        if (resolver_->queue[id].status == IP::ResolverStatus::NONE) {
            // TODO: logging
            // ...

            return IP::ResolverStatus::NONE;
        }

        IP::ResolverStatus ret = resolver_->queue[id].status;

        return ret;
    }

    IpAddress
    IP::ResolveItemAddress(IP::ResolverID id) const
    {
        // TODO: bounds checking
        // ...

        auto lk = std::lock_guard<std::mutex>(resolver_->mtx);

        if (resolver_->queue[id].status != IP::ResolverStatus::DONE) {
            // TODO: logging
            // ...

            return IpAddress();
        }

        IpAddress ret = resolver_->queue[id].response;

        return ret;
    }

    void
    IP::EraseResolveItem(IP::ResolverID id)
    {
        // TODO: bounds checking
        // ...

        auto lk = std::lock_guard<std::mutex>(resolver_->mtx);

        resolver_->queue[id].status = IP::ResolverStatus::NONE;
    }

    void
    IP::ClearCache(const std::string &hostname)
    {
        auto lk = std::lock_guard<std::mutex>(resolver_->mtx);

        if (hostname.empty()) {
            resolver_->cache.clear();
        }
        else {
            resolver_->cache.erase(IpResolver::get_cache_key(hostname, Type::NONE));
            resolver_->cache.erase(IpResolver::get_cache_key(hostname, Type::V4));
            resolver_->cache.erase(IpResolver::get_cache_key(hostname, Type::V6));
            resolver_->cache.erase(IpResolver::get_cache_key(hostname, Type::ANY));
        }
    }
} // namespace core
