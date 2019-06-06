#include "udp.h"

UdpChamber::UdpChamber() : _continue_sending(false),
                           _header_flags(0),
                           _packets_sent(0)
{}

UdpBuffer *
UdpChamber::buffer_insert_pos()
{
    return &_buffers[_buffer_count];
}

UdpProtocolType *
UdpChamber::command_insert_pos()
{
    return &_commands[_command_count];
}

bool
UdpChamber::sending_continues(UdpProtocolType *command,
                              UdpBuffer *buffer,
                              uint32_t mtu,
                              const std::shared_ptr<UdpOutgoingCommand> &outgoing_command)
{
    // MEMO: [誤] _udp_protocol_send_reliable_outgoing_commands() では
    //            buffer に command が挿入されたら同時にインクリメントされるので、
    //            command か buffer どちらかでよいのでは？
    //       [正] コマンドがパケットを持っている際に buffer がインクリメントされる（コマンドに続くデータがバッファに投入される）ので、
    //            それぞれで判定する必要がある

    // unsent command exists
    if (command >= &_commands[sizeof(_commands) / sizeof(UdpProtocol)])
        return true;

    // unsent data exists
    if (buffer + 1 >= &_buffers[sizeof(_buffers) / sizeof(UdpBuffer)])
        return true;

    auto command_size = command_sizes[outgoing_command->command->header.command & PROTOCOL_COMMAND_MASK];

    // has not enough space for command（コマンド分のスペースがなければ続くデータも送信できないので先にチェック）
    if (mtu - _packet_size < command_size)
        return true;

    if (outgoing_command->packet != nullptr)
        return false;

    // has not enough space for command with payload
    if (static_cast<uint16_t>(mtu - _packet_size) <
        static_cast<uint16_t>(command_size + outgoing_command->fragment_length))
    {
        return true;
    }

    return false;
}

bool
UdpChamber::sent_reliable_command_exists()
{
    return !_sent_reliable_commands.empty();
}

bool
UdpChamber::sent_unreliable_command_exists()
{
    return !_sent_unreliable_commands.empty();
}

void
UdpChamber::add_header_flags(uint16_t flag)
{
    _header_flags |= flag;
}

void
UdpChamber::increase_packet_size(size_t val)
{
    _packet_size += val;
}

void
UdpChamber::increse_reliable_data_in_transit(uint32_t val)
{
    _reliable_data_in_transit += val;
}

void
UdpChamber::push_sent_reliable_command(std::shared_ptr<UdpOutgoingCommand> &command)
{
    _sent_reliable_commands.push_back(command);

    ++_packets_sent;
}

void
UdpChamber::push_sent_unreliable_command(std::shared_ptr<UdpOutgoingCommand> &command)
{
    _sent_unreliable_commands.push_back(command);
}

void
UdpChamber::update_command_count(const UdpProtocolType *command)
{
    _command_count = command - _commands;
}

void
UdpChamber::update_buffer_count(const UdpBuffer *buffer)
{
    _buffer_count = buffer - _buffers;
}

uint32_t
UdpChamber::reliable_data_in_transit()
{
    return _reliable_data_in_transit;
}

bool
UdpChamber::continue_sending()
{
    return _continue_sending;
}

void
UdpChamber::continue_sending(bool val)
{
    _continue_sending = val;
}

bool
UdpChamber::command_buffer_have_enough_space(UdpProtocolType *command)
{
    return command < &_commands[PROTOCOL_MAXIMUM_PACKET_COMMANDS];
}

bool
UdpChamber::data_buffer_have_enough_space(UdpBuffer *buffer)
{
    return buffer < &_buffers[BUFFER_MAXIMUM];
}

void
UdpChamber::remove_sent_unreliable_commands()
{
    auto outgoing_command = _sent_unreliable_commands.begin();


    if (outgoing_command->packet != nullptr)
    {
        if (outgoing_command->packet.use_count() == 0)
            outgoing_command->packet->flags |= static_cast<uint32_t >(UdpPacketFlag::SENT);
    }

    _sent_unreliable_commands.pop_front();
}
