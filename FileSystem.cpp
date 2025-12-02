#include "FileSystem.hpp"
#include <windows.h>

String8 FsReadFile(Arena* arena, String8 filePath)
{
	Temp scratch = {};
	HANDLE fileHandle = {};
	u32 fileSize = {};
	u32 bytesrw = {};

	scratch = ScratchBegin();
	fileHandle = CreateFileA(String8ToCString(scratch.arena, filePath), GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	if (InvalidHandle(fileHandle))
	{
		return {};
	}

	fileSize = GetFileSize(fileHandle, 0);
	
	char* fileBuf = PushArray(arena, char, fileSize);

	if (!ReadFile(fileHandle, fileBuf, fileSize, rcast<LPDWORD>(&bytesrw), 0))
	{
		return {};
	}

	CloseHandle(fileHandle);
	
	ScratchEnd(scratch);
	return { fileBuf, fileSize };
}

OsHandle FsCreateFile(String8 filePath)
{
	Temp scratch = ScratchBegin();

	HANDLE fileHandle = CreateFileA(String8ToCString(scratch.arena, filePath), GENERIC_ALL, FILE_SHARE_READ, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);

	ScratchEnd(scratch);

	return OsHandle(fileHandle);
}

bool FsWriteFile(OsHandle fileHandle, String8 data)
{
	DWORD bytesrw = {};
	return WriteFile(HANDLE(fileHandle), data.data, static_cast<DWORD>(data.length), &bytesrw, 0);
}

void FsCloseFile(OsHandle fileHandle)
{
	CloseHandle(HANDLE(fileHandle));
}

String8 FsCurDirectory()
{
	Temp scratch = ScratchBegin();

	u32 curDirectoryPathSize = GetCurrentDirectoryA(0, 0);
	char* fullPathBuf = PushArray(scratch.arena, char, curDirectoryPathSize);
	GetCurrentDirectoryA(curDirectoryPathSize, fullPathBuf);

	ScratchEnd(scratch);

	return { fullPathBuf, curDirectoryPathSize };
}
