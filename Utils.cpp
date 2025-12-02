#include "Utils.h"
#include "String.hpp"
#include "Types.hpp"
#include <cstdio>

void HexDump(String8 data, size_t stride)
{
	for (size_t i = 0; i < data.length; i += stride)
	{
		for (size_t j = 0; i + j < data.length && j < stride; j++)
		{
			printf("%02X ", u8(data.data[i + j]));
		}

		printf("\n");
	}
}
