#include <vector>

#include <zlib.h>

#include "lib/core/error_macros.h"
#include "compression.h"

int
Compression::get_max_compressed_buffer_size(int p_src_size, Compression::Mode p_mode)
{
    if (p_mode == Mode::FASTLS)
    {
        // ...
    }
    else if (p_mode == Mode::DEFLATE || p_mode == Mode::GZIP)
    {
        int window_bits = p_mode == Mode::DEFLATE ? 15 : 15 + 16;

        z_stream stream;

        stream.zalloc = Z_NULL;
        stream.zfree = Z_NULL;
        stream.opaque = Z_NULL;

        auto err = deflateInit2(&stream, Z_DEFAULT_COMPRESSION, Z_DEFLATED, window_bits, 8, Z_DEFAULT_STRATEGY);

        if (err != Z_OK)
        {
            return -1;
        }

        auto aout = deflateBound(&stream, p_src_size);

        deflateEnd(&stream);

        return aout;
    }
    else if (p_mode == Mode::ZSTD)
    {
        // ...
    }

    ERR_FAIL_V(-1)
}

int
Compression::compress(std::vector<uint8_t> &dst, std::vector<uint8_t> &src, Compression::Mode mode)
{
    if (mode == Mode::DEFLATE || mode == Mode::GZIP)
    {
        auto window_bits = mode == Mode::DEFLATE ? 15 : 15 + 16;

        z_stream stream;

        stream.zalloc = Z_NULL;
        stream.zfree = Z_NULL;
        stream.opaque = Z_NULL;

        auto level = mode == Mode::DEFLATE ? zlib_level : gzip_level;

        auto err = deflateInit2(&stream, level, Z_DEFLATED, window_bits, 8, Z_DEFAULT_STRATEGY);

        if (err != Z_OK)
        {
            return -1;
        }

        stream.avail_in = src.size();

        auto aout = deflateBound(&stream, src.size());

        stream.avail_out = aout;
        stream.next_in = reinterpret_cast<Bytef *>(&src.at(0));
        stream.next_out = &dst.at(0);

        deflate(&stream, Z_FINISH);

        aout = aout - stream.avail_out;

        deflateEnd(&stream);

        return aout;
    }
    else if (mode == Mode::ZSTD)
    {
        // ...
    }

    ERR_FAIL_V(-1)
}

int
Compression::decompress(std::vector<uint8_t> &dst, int dst_max_size, std::vector<uint8_t> &src, int src_max_size, Mode mode)
{
    if (mode == Mode::DEFLATE || mode == Mode::GZIP)
    {
        auto window_bits = mode == Mode::DEFLATE ? 15 : 15 + 16;

        z_stream stream;

        stream.zalloc = Z_NULL;
        stream.zfree = Z_NULL;
        stream.opaque = Z_NULL;
        stream.avail_in = 0;
        stream.next_in = Z_NULL;

        auto err = inflateInit2(&stream, window_bits);

        ERR_FAIL_COND_V(err != Z_OK, -1)

        stream.avail_in = src.size();
        stream.avail_out = dst_max_size;
        stream.next_in = reinterpret_cast<Bytef *>(&src.at(0));
        stream.next_out = &dst.at(0);

        err = inflate(&stream, Z_FINISH);

        auto total = stream.total_out;

        inflateEnd(&stream);

        ERR_FAIL_COND_V(err != Z_STREAM_END, -1)

        return total;
    }
    else if (mode == Mode::ZSTD)
    {
        // ...
    }

    ERR_FAIL_V(-1)
}

int Compression::zlib_level = Z_DEFAULT_COMPRESSION;
int Compression::gzip_level = Z_DEFAULT_COMPRESSION;
int Compression::zstd_level = 3;
bool Compression::zstd_long_distance_matching = false;
int Compression::zstd_window_log_size = 27;
