#pragma once

#include "Types.hpp"

struct ThreadStartParams;
using ThreadRoutine = u32(*)(ptr);

struct ThreadStartParams
{
	ThreadRoutine routine;
	ptr param;
};

u32 ThreadMainEntry(ThreadStartParams* params);