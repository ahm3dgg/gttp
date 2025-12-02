#pragma once

#include "String.hpp"
#include "Arena.hpp"

String8 GzipCompress(Arena* arena, String8 input);
String8 GzipDecompress(Arena* arena, String8 input, size_t decompressedSize);