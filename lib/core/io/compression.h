#ifndef P2P_TECHDEMO_CORE_COMPRESSION_H
#define P2P_TECHDEMO_CORE_COMPRESSION_H

#include "lib/core/typedefs.h"

class Compression
{
public:
    static int zlib_level;
    static int gzip_level;
    static int zstd_level;
    static bool zstd_long_distance_matching;
    static int zstd_window_log_size;

    enum class Mode: int
    {
        FASTLS,
        DEFLATE,
        ZSTD,
        GZIP
    };

    static int compress(std::vector<uint8_t> &dst, std::vector<uint8_t> &src, Compression::Mode mode);
    static int decompress(std::vector<uint8_t> &dst,
                          int dst_max_size,
                          std::vector<uint8_t> &src,
                          int src_max_size, Mode mode);
    static int get_max_compressed_buffer_size(int p_src_size, Mode p_mode);
};

#endif // P2P_TECHDEMO_CORE_COMPRESSION_H
