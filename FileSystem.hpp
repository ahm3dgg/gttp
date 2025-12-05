#pragma once
#include "Arena.hpp"
#include "ScratchArena.hpp"
#include "String.hpp"
#include "OS.hpp"

String8 FsReadFile(Arena* arena, String8 filePath);
OsHandle FsCreateFile(String8 filePath);
bool FsWriteFile(OsHandle fileHandle, String8 data);
proc FsCloseFile(OsHandle fileHandle);
String8 FsCurDirectory();
