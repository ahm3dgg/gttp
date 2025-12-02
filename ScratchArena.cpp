#include "ScratchArena.hpp"

thread_local Arena* scratchArena = nullptr;

Temp ScratchBegin()
{
	return TempBegin(scratchArena);
}

void ScratchEnd(Temp tmp)
{
	TempEnd(tmp);
}
