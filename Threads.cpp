#include "Threads.hpp"
#include "Arena.hpp"
#include "ScratchArena.hpp"

u32 ThreadMainEntry(ThreadStartParams* params)
{
	scratchArena = ArenaAlloc();
	return params->routine(params->param);
}