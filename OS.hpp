#pragma once

#include "Defines.hpp"
#include "Types.hpp"

typedef size_t OsHandle;

// System Information
u64 OsGetPageSize();
u64 OsGetLargePageSize();

// Memory
ptr OsMemoryReserveLarge(size_t reserveSize);
ptr OsMemoryReserve(size_t reserveSize);
ptr OsMemoryCommit(ptr base, size_t commitSize);
bool OsMemoryCommitLarge(ptr base, size_t commitSize);
bool OsMemoryRelease(ptr base, size_t size);
proc OsWaitForThread(OsHandle thread, u32 milliseconds);

OsHandle OsCreateThread(ptr func, ptr param = nullptr);

u32 OsGetNumberOfProcessors();