#include "RUdpEvent.h"

RUdpEvent::RUdpEvent()
    : type_(RUdpEventType::NONE),
      channel_id_(-1),
      data_()
{}

void RUdpEvent::Reset()
{
    peer_ = nullptr;
    segment_ = nullptr;
    type_ = RUdpEventType::NONE;
    data_ = 0;
    channel_id_ = -1;
}
