#pragma once

#include "Types.hpp"

typedef size_t OsHandle;

// System Information
u64 OsGetPageSize();
u64 OsGetLargePageSize();

// Memory
void* OsMemoryReserveLarge(size_t reserveSize);
bool OsMemoryCommitLarge(void* base, size_t commitSize);
void* OsMemoryReserve(size_t reserveSize);
void* OsMemoryCommit(void* base, size_t commitSize);
bool OsMemoryRelease(void* base, size_t size);
OsHandle OsCreateThread(void* func, void* param = nullptr);
void OsWaitForThread(OsHandle thread, u32 milliseconds);

u32 OsGetNumberOfProcessors();