#include "OS.hpp"
#include <Windows.h>
#include "ScratchArena.hpp"

u64 OsGetPageSize()
{
	SYSTEM_INFO sysinfo{};
	GetSystemInfo(&sysinfo);
	return sysinfo.dwPageSize;
}

u64 OsGetLargePageSize()
{
	return GetLargePageMinimum();
}

void* OsMemoryReserveLarge(size_t size)
{
	return VirtualAlloc(0, size, MEM_RESERVE | MEM_COMMIT | MEM_LARGE_PAGES, PAGE_READWRITE);
}

bool OsMemoryCommitLarge(void* base, size_t size)
{
	return true;
}

void* OsMemoryReserve(size_t size)
{
	return VirtualAlloc(0, size, MEM_RESERVE, PAGE_READWRITE);
}

void* OsMemoryCommit(void* base, size_t size)
{
	return VirtualAlloc(base, size, MEM_COMMIT, PAGE_READWRITE);
}

bool OsMemoryRelease(void* base, size_t size)
{
	return VirtualFree(base, 0, MEM_RELEASE);
}

OsHandle OsCreateThread(void* func, void* param)
{
	return OsHandle(CreateThread(0, 0, LPTHREAD_START_ROUTINE(func), param, 0, 0));
}

void OsWaitForThread(OsHandle thread, u32 milliseconds)
{
	WaitForSingleObject(HANDLE(thread), milliseconds);
}

u32 OsGetNumberOfProcessors()
{
	SYSTEM_INFO si = {};
	GetSystemInfo(&si);
	return si.dwNumberOfProcessors;
}
