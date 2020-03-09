#ifndef P2P_TECHDEMO_RUDPBUFFER_H
#define P2P_TECHDEMO_RUDPBUFFER_H

#include <cstddef>
#include <memory>
#include <string>
#include <variant>

#include "lib/rudp/protocol/protocol_type.h"
#include "segment.h"
#include "type.h"

namespace rudp
{
    class Buffer
    {
      public:
        Buffer();

        void
        Add(const ProtocolTypeSP &data);

        void
        CopyHeaderFrom(const std::vector<uint8_t> &data, size_t offset, size_t size);

        void
        CopySegmentFrom(const std::shared_ptr<Segment> &segment, size_t offset, size_t size = 0);

        std::string
        ProtocolCommandAsString();

        std::vector<uint8_t>::iterator
        CopyTo(std::vector<uint8_t>::iterator it);

        inline size_t
        Size()
        {
            return size_;
        };

        inline void
        Size(size_t val)
        {
            size_ = val;
        }

      private:
        using VariantBuffer = std::variant<ProtocolTypeSP, std::vector<uint8_t>>;

        VariantBuffer buffer_;
        size_t offset_;
        size_t size_;
    };
} // namespace rudp

#endif // P2P_TECHDEMO_RUDPBUFFER_H
