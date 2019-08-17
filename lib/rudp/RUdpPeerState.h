#ifndef P2P_TECHDEMO_RUDPPEERSTATE_H
#define P2P_TECHDEMO_RUDPPEERSTATE_H

enum class RUdpPeerState: uint8_t
{
    DISCONNECTED = 0,
    CONNECTING,
    ACKNOWLEDGING_CONNECT,
    CONNECTION_PENDING,
    CONNECTION_SUCCEEDED,
    CONNECTED,
    DISCONNECT_LATER,
    DISCONNECTING,
    ACKNOWLEDGING_DISCONNECT,
    ZOMBIE
};

#endif // P2P_TECHDEMO_RUDPPEERSTATE_H
