#include "channel.h"

namespace rudp
{
    Channel::Channel()
        : incoming_reliable_commands_()
        , incoming_unreliable_commands_()
        , reliable_windows_()
        , incoming_reliable_sequence_number_()
        , incoming_unreliable_sequence_number_()
        , outgoing_reliable_sequence_number_()
        , outgoing_unreliable_sequence_number_()
        , used_reliable_windows_()
    {
    }

    std::vector<std::shared_ptr<IncomingCommand>>
    Channel::NewIncomingReliableCommands()
    {
        std::vector<std::shared_ptr<IncomingCommand>> commands;
        auto is_new_command_detected = false;

        for (auto &cmd : incoming_reliable_commands_) {
            auto reliable_sequence_number = cmd->reliable_sequence_number();

            if (cmd->fragments_remaining() > 0 || reliable_sequence_number != incoming_reliable_sequence_number_ + 1)
                break;
            else
                is_new_command_detected = true;

            incoming_reliable_sequence_number_ = reliable_sequence_number;

            if (cmd->fragment_count() > 0)
                incoming_reliable_sequence_number_ += cmd->fragment_count() - 1;

            commands.push_back(cmd);
        }

        if (is_new_command_detected)
            incoming_unreliable_sequence_number_ = 0;

        return commands;
    }

    void
    Channel::Reset()
    {
        incoming_reliable_commands_.clear();
        incoming_unreliable_commands_.clear();
        std::fill(reliable_windows_.begin(), reliable_windows_.end(), 0);
        incoming_reliable_sequence_number_   = 0;
        incoming_unreliable_sequence_number_ = 0;
        outgoing_reliable_sequence_number_   = 0;
        outgoing_unreliable_sequence_number_ = 0;
        used_reliable_windows_               = 0;
    }

    namespace
    {
        Error
        DiscardCommand(uint32_t fragment_count)
        {
            if (fragment_count > 0)
                return Error::ERROR;

            return Error::OK;
        }
    } // namespace

    std::tuple<std::shared_ptr<IncomingCommand>, Error>
    Channel::ExtractFirstCommand(uint16_t start_sequence_number, int total_length, uint32_t fragment_count)
    {
        if (incoming_reliable_commands_.empty()) {
            return {nullptr, Error::DOES_NOT_EXIST};
        }

        auto cmd = --(incoming_reliable_commands_.end());

        for (; cmd != incoming_reliable_commands_.begin(); --cmd) {
            //
            if (start_sequence_number >= incoming_reliable_sequence_number_) {
                if ((*cmd)->reliable_sequence_number() < incoming_reliable_sequence_number_) {
                    continue;
                }
                else if ((*cmd)->reliable_sequence_number() >= incoming_reliable_sequence_number_) {
                    return std::make_tuple(nullptr, Error::OK);
                }
            }

            //
            if ((*cmd)->reliable_sequence_number() <= start_sequence_number) {
                if ((*cmd)->reliable_sequence_number() < start_sequence_number) {
                    return std::make_tuple(nullptr, Error::OK);
                }

                if (((*cmd)->command()->header.command & PROTOCOL_COMMAND_MASK) !=
                        static_cast<uint8_t>(RUdpProtocolCommand::SEND_FRAGMENT) ||
                    total_length != (*cmd)->segment()->DataLength() || fragment_count != (*cmd)->fragment_count()) {
                    return std::make_tuple(nullptr, Error::ERROR);
                }

                break;
            }
        }

        return std::make_tuple((*cmd), Error::OK);
    }

    std::tuple<std::shared_ptr<IncomingCommand>, Error>
    Channel::QueueIncomingCommand(const std::shared_ptr<ProtocolType> &cmd, std::vector<uint8_t> &data, uint16_t flags,
                                  uint32_t fragment_count)
    {
        uint16_t reliable_sequence_number{};
        uint16_t unreliable_sequence_number{};

        if ((cmd->header.command & PROTOCOL_COMMAND_MASK) !=
            static_cast<uint8_t>(RUdpProtocolCommand::SEND_UNSEQUENCED)) {
            reliable_sequence_number = cmd->header.reliable_sequence_number;
            auto reliable_window     = reliable_sequence_number / PEER_RELIABLE_WINDOW_SIZE;
            auto current_window      = incoming_reliable_sequence_number_ / PEER_RELIABLE_WINDOW_SIZE;

            if (reliable_sequence_number < incoming_reliable_sequence_number_)
                reliable_window += PEER_RELIABLE_WINDOWS;

            if (reliable_window < current_window || reliable_window >= current_window + PEER_FREE_RELIABLE_WINDOWS - 1)
                return {nullptr, DiscardCommand(fragment_count)};
        }

        auto cmd_type = static_cast<RUdpProtocolCommand>(cmd->header.command & PROTOCOL_COMMAND_MASK);
        std::list<std::shared_ptr<IncomingCommand>>::iterator insert_pos;

        if (cmd_type == RUdpProtocolCommand::SEND_FRAGMENT || cmd_type == RUdpProtocolCommand::SEND_RELIABLE) {
            if (reliable_sequence_number == incoming_reliable_sequence_number_)
                return {nullptr, DiscardCommand(fragment_count)};

            if (!incoming_reliable_commands_.empty()) {
                auto end_of_list = false;
                for (insert_pos = std::prev(incoming_reliable_commands_.end(), 1); !end_of_list; --insert_pos) {
                    if (insert_pos == incoming_reliable_commands_.end())
                        end_of_list = true;

                    if (reliable_sequence_number >= incoming_reliable_sequence_number_) {
                        if ((*insert_pos)->reliable_sequence_number() < incoming_reliable_sequence_number_)
                            continue;
                    }
                    else if ((*insert_pos)->reliable_sequence_number() >= incoming_reliable_sequence_number_) {
                        break;
                    }

                    if ((*insert_pos)->reliable_sequence_number() <= reliable_sequence_number) {
                        if ((*insert_pos)->reliable_sequence_number() < reliable_sequence_number)
                            break;

                        return {nullptr, DiscardCommand(fragment_count)};
                    }
                }
            }
            else {
                insert_pos = incoming_reliable_commands_.end();
            }
        }
        else if (cmd_type == RUdpProtocolCommand::SEND_UNRELIABLE ||
                 cmd_type == RUdpProtocolCommand::SEND_UNRELIABLE_FRAGMENT) {
            unreliable_sequence_number = cmd->send_unreliable.unreliable_sequence_number;

            if (reliable_sequence_number == incoming_reliable_sequence_number_ &&
                unreliable_sequence_number <= incoming_unreliable_sequence_number_) {
                return {nullptr, DiscardCommand(fragment_count)};
            }

            if (!incoming_unreliable_commands_.empty()) {
                auto end_of_list = false;
                for (insert_pos = std::prev(incoming_unreliable_commands_.end(), 1); !end_of_list; --insert_pos) {
                    if (reliable_sequence_number >= incoming_reliable_sequence_number_) {
                        if ((*insert_pos)->reliable_sequence_number() < incoming_reliable_sequence_number_)
                            continue;
                    }
                    else if ((*insert_pos)->reliable_sequence_number() >= incoming_reliable_sequence_number_) {
                        break;
                    }

                    if ((*insert_pos)->reliable_sequence_number() < reliable_sequence_number)
                        break;

                    if ((*insert_pos)->reliable_sequence_number() > reliable_sequence_number)
                        continue;

                    if ((*insert_pos)->unreliable_sequence_number() <= unreliable_sequence_number) {
                        if ((*insert_pos)->unreliable_sequence_number() < unreliable_sequence_number)
                            break;

                        return {nullptr, DiscardCommand(fragment_count)};
                    }
                }
            }
            else {
                insert_pos = incoming_unreliable_commands_.end();
            }
        }
        else if (cmd_type == RUdpProtocolCommand::SEND_UNSEQUENCED) {
            insert_pos = incoming_unreliable_commands_.end();
        }
        else {
            return {nullptr, DiscardCommand(fragment_count)};
        }

        std::shared_ptr<Segment> segment = nullptr;

        if (fragment_count > 0) {
            segment = std::make_shared<Segment>(&data, flags, cmd->send_fragment.total_length);
        }
        else {
            segment = std::make_shared<Segment>(&data, flags);
        }

        if (segment == nullptr)
            return std::tuple(nullptr, Error::CANT_ALLOCATE);

        auto in_cmd = std::make_shared<IncomingCommand>();

        if (in_cmd == nullptr)
            return std::tuple(nullptr, Error::CANT_ALLOCATE);

        in_cmd->reliable_sequence_number(cmd->header.reliable_sequence_number);
        in_cmd->unreliable_sequence_number(unreliable_sequence_number & 0xFFFF);
        in_cmd->command(cmd);
        in_cmd->fragment_count(fragment_count);
        in_cmd->fragments_remaining(fragment_count);
        in_cmd->segment(segment);

        if (fragment_count > 0) {
            Error is_memory_allocated{};

            if (fragment_count <= PROTOCOL_MAXIMUM_FRAGMENT_COUNT)
                // TODO: handle std::bad_alloc
                is_memory_allocated = in_cmd->ResizeFragmentBuffer((fragment_count + 31) / 32 * sizeof(uint32_t));

            if (is_memory_allocated == Error::CANT_ALLOCATE)
                return {nullptr, Error::ERROR};
        }

        if (cmd_type == RUdpProtocolCommand::SEND_FRAGMENT || cmd_type == RUdpProtocolCommand::SEND_RELIABLE) {
            if (insert_pos == incoming_reliable_commands_.end())
                incoming_reliable_commands_.push_back(in_cmd);
            else
                incoming_reliable_commands_.insert(std::next(insert_pos), in_cmd);
        }
        else {
            if (insert_pos == incoming_unreliable_commands_.end())
                incoming_unreliable_commands_.push_back(in_cmd);
            else
                incoming_unreliable_commands_.insert(std::next(insert_pos), in_cmd);
        }

        return {in_cmd, Error::OK};
    }
} // namespace rudp
