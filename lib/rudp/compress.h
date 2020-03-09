#ifndef P2P_TECHDEMO_RUDPCOMPRESS_H
#define P2P_TECHDEMO_RUDPCOMPRESS_H

#include <functional>

#include "buffer.h"

namespace rudp
{
    using CompressorType = std::function<size_t(const std::vector<Buffer> &in_buffers, size_t in_limit,
                                                std::vector<uint8_t> &out_data, size_t out_limit)>;

    using DecompressorType = std::function<size_t(std::vector<uint8_t> &in_data, size_t in_limit,
                                                  std::vector<uint8_t> &out_data, size_t out_limit)>;

    using CleanerType = std::function<void()>;

    class Compress
    {
      public:
        inline bool
        CanCompress()
        {
            return compressor_ != nullptr;
        }

        inline void
        SetCompressor(CompressorType &&compressor)
        {
            compressor_ = std::move(compressor);
        }

        inline void
        SetDecompressor(DecompressorType &&decompressor)
        {
            decompressor_ = std::move(decompressor);
        }

        inline void
        SetCleaner(CleanerType &&destructor)
        {
            cleaner_ = std::move(destructor);
        }

      private:
        CompressorType compressor_;
        DecompressorType decompressor_;
        CleanerType cleaner_;
    };
} // namespace rudp

#endif // P2P_TECHDEMO_RUDPCOMPRESS_H
