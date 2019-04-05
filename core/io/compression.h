#ifndef P2P_TECHDEMO_CORE_COMPRESSION_H
#define P2P_TECHDEMO_CORE_COMPRESSION_H

#include "core/typedefs.h"

class Compression
{
public:
    static int zlib_level;
    static int gzip_level;
    static int zstd_level;
    static bool zstd_long_distance_matching;
    static int zstd_window_log_size;

    enum class Mode : int
    {
        FASTLS,
        DEFLATE,
        ZSTD,
        GZIP
    };

    static int compress(uint8_t *p_dst, const uint8_t *p_src, int p_src_size, Mode p_mode = Mode::ZSTD);
    static int get_max_compressed_buffer_size(int p_src_size, Mode p_mode = Mode::ZSTD);
    static int decompress(uint8_t *p_dst, int p_dst_max_size, const uint8_t *p_src, int p_src_size, Mode p_mode = Mode::ZSTD);

    Compression();
};

#endif // P2P_TECHDEMO_CORE_COMPRESSION_H
