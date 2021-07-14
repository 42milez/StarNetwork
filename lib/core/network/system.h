#ifndef STAR_NETWORK_LIB_CORE_NETWORK_SYSTEM_H_
#define STAR_NETWORK_LIB_CORE_NETWORK_SYSTEM_H_

namespace core
{
    enum class SysCh : uint8_t
    {
        CONFIG,
        RELIABLE,
        UNRELIABLE,
        MAX
    };

    enum class SysMsg : uint8_t
    {
        ADD_PEER,
        REMOVE_PEER
    };
} // namespace core

#endif // STAR_NETWORK_LIB_CORE_NETWORK_SYSTEM_H_
