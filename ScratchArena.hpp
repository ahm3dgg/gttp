#pragma once

#include "Arena.hpp"

extern thread_local Arena* scratchArena;

Temp ScratchBegin();
void ScratchEnd(Temp tmp);
