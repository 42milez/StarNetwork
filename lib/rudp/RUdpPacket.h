#ifndef P2P_TECHDEMO_RUDPPACKET_H
#define P2P_TECHDEMO_RUDPPACKET_H

#include <functional>

#include <cstddef>
#include <cstdint>

class RUdpPacket
{
private:
    std::function<void(RUdpPacket *)> _free_callback;

    uint32_t _flags;

    uint8_t *_data;

    size_t _data_length;

    void *_user_data;

public:
    uint32_t add_flag(uint32_t flag);

    size_t data_length();

    uint8_t * move_data_pointer(uint32_t val);

    void destroy();
};

#endif // P2P_TECHDEMO_RUDPPACKET_H
