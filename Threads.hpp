#pragma once

#include "Types.hpp"

struct ThreadStartParams;
using ThreadRoutine = u32(*)(void*);

struct ThreadStartParams
{
	ThreadRoutine routine;
	void* param;
};

u32 ThreadMainEntry(ThreadStartParams* params);