#pragma once

#include "Types.hpp"

constexpr inline u16 ToBigEndian16(u16 value)
{
	return (u16(value & 0xff) << 8) | (value >> 8);
}