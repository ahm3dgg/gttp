#pragma once

#include "String.hpp"

void HexDump(String8 data, size_t stride = 16);

inline constexpr size_t Align(size_t addr, size_t align)
{
	size_t off = addr % align;
	if (off != 0)
	{
		addr += align - off;
	}

	return addr;
}

inline constexpr bool IsPower2(size_t value)
{
	return (value & (value - 1)) == 0;
}