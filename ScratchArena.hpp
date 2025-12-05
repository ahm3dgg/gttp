#pragma once

#include "Arena.hpp"

extern thread_local Arena* scratchArena;

Temp ScratchBegin();
proc ScratchEnd(Temp tmp);
