#ifndef P2P_TECHDEMO_LIB_CORE_NETWORK_SYSTEM_H_
#define P2P_TECHDEMO_LIB_CORE_NETWORK_SYSTEM_H_

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
} // core

#endif // P2P_TECHDEMO_LIB_CORE_NETWORK_SYSTEM_H_
