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

ptr OsMemoryReserveLarge(size_t size)
{
	return VirtualAlloc(0, size, MEM_RESERVE | MEM_COMMIT | MEM_LARGE_PAGES, PAGE_READWRITE);
}

bool OsMemoryCommitLarge(ptr base, size_t size)
{
	return true;
}

ptr OsMemoryReserve(size_t size)
{
	return VirtualAlloc(0, size, MEM_RESERVE, PAGE_READWRITE);
}

ptr OsMemoryCommit(ptr base, size_t size)
{
	return VirtualAlloc(base, size, MEM_COMMIT, PAGE_READWRITE);
}

bool OsMemoryRelease(ptr base, size_t size)
{
	return VirtualFree(base, 0, MEM_RELEASE);
}

OsHandle OsCreateThread(ptr func, ptr param)
{
	return OsHandle(CreateThread(0, 0, LPTHREAD_START_ROUTINE(func), param, 0, 0));
}

proc OsWaitForThread(OsHandle thread, u32 milliseconds)
{
	WaitForSingleObject(HANDLE(thread), milliseconds);
}

u32 OsGetNumberOfProcessors()
{
	SYSTEM_INFO si = {};
	GetSystemInfo(&si);
	return si.dwNumberOfProcessors;
}
