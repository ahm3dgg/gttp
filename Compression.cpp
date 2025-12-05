#include "Compression.hpp"
#include "Defines.hpp"
#include <zlib.h>

String8 GzipCompress(Arena* arena, String8 input)
{
	z_stream zs{};
	String8 compressed{};

	compressed = String8New(arena, compressBound(static_cast<uLong>(input.length)));

	zs.zalloc = Z_NULL;
	zs.zfree = Z_NULL;
	zs.opaque = Z_NULL;
	zs.avail_in = uInt(input.length);
	zs.next_in = rcast<Bytef*>(input.data);
	zs.avail_out = uInt(compressed.length);
	zs.next_out = rcast<Bytef*>(compressed.data);

	deflateInit2(&zs, Z_DEFAULT_COMPRESSION, Z_DEFLATED, 15 | 16, 8, Z_DEFAULT_STRATEGY);
	deflate(&zs, Z_FINISH);
	deflateEnd(&zs);

	compressed.length = zs.total_out;

	return compressed;
}

String8 GzipDecompress(Arena* arena, String8 input, size_t decompressedSize)
{
	z_stream zs{};
	String8 decompressed{};

	decompressed = String8New(arena, decompressedSize);

	zs.zalloc = Z_NULL;
	zs.zfree = Z_NULL;
	zs.opaque = Z_NULL;
	zs.avail_in = uInt(input.length);
	zs.next_in = rcast<Bytef*>(input.data);
	zs.avail_out = uInt(decompressedSize);
	zs.next_out = rcast<Bytef*>(decompressed.data);

	if (inflateInit2(&zs, 15 + 32) != Z_OK)
	{
		return {};
	}

	int ret = inflate(&zs, Z_FINISH);
	if (ret != Z_STREAM_END && ret != Z_OK && ret != Z_BUF_ERROR)
	{
		inflateEnd(&zs);
		return {};
	}

	inflateEnd(&zs);

	decompressed.length = zs.total_out;

	return decompressed;
}