#include <zlib.h>

#include "core/error_macros.h"
#include "core/io/zip_io.h"
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
