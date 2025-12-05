#include "ScratchArena.hpp"

thread_local Arena* scratchArena = nullptr;

Temp ScratchBegin()
{
	return TempBegin(scratchArena);
}

proc ScratchEnd(Temp tmp)
{
	TempEnd(tmp);
}
