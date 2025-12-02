#pragma once

#include "Types.hpp"

#define rcast reinterpret_cast
#define typeof decltype

#define InvalidHandle(h) ((h) == INVALID_HANDLE_VALUE)
#define IsNull(v) ((v) == nullptr)
#define InvalidHandle2(v) IsNull(v)
#define AlignPow2(x, b) (((x) + (b) - 1) & (~((b) - 1)))

#define KB(n) (((u64)(n)) << 10)
#define MB(n) (((u64)(n)) << 20)
#define GB(n) (((u64)(n)) << 30)
#define TB(n) (((u64)(n)) << 40)

#define AlignOf(T) __alignof(T)

#define SLLStackPush_N(f, n, next) ((n)->next = (f), (f) = (n))
#define SLLStackPop_N(f, next) ((f) = (f)->next)

#define Min(A, B) (((A) < (B)) ? (A) : (B))
#define Max(A, B) (((A) > (B)) ? (A) : (B))
#define ClampTop(A, X) Min(A, X)
#define ClampBot(X, B) Max(X, B)
#define Clamp(A, X, B) (((X) < (A)) ? (A) : ((X) > (B)) ? (B) : (X))

#define MemoryZero(s, z) memset((s), 0, (z))

#ifndef CopyMemory
#define CopyMemory(d,s,l) memcpy((d), (s), (l))
#endif 

#define Trap() __debugbreak()

#define AssertAlways(x) do{if(!(x)) {Trap();}}while(0)
#if BUILD_DEBUG
# define Assert(x) AssertAlways(x)
#else
# define Assert(x) (void)(x)
#endif
#define InvalidPath        Assert(!"Invalid Path!")
#define NotImplemented     Assert(!"Not Implemented!")
#define NoOp               ((void)0)
#define StaticAssert(C, ID) global U8 Glue(ID, __LINE__)[(C)?1:-1]

#if COMPILER_MSVC || (COMPILER_CLANG && OS_WINDOWS)
# pragma section(".rdata$", read)
# define read_only __declspec(allocate(".rdata$"))
#elif (COMPILER_CLANG && OS_LINUX)
# define read_only __attribute__((section(".rodata")))
#else
// NOTE(rjf): I don't know of a useful way to do this in GCC land.
// __attribute__((section(".rodata"))) looked promising, but it introduces a
// strange warning about malformed section attributes, and it doesn't look
// like writing to that section reliably produces access violations, strangely
// enough. (It does on Clang)
# define read_only
#endif

#define C_LINKAGE extern "C"

#ifdef _DEBUG
#define ASAN_ENABLED 1
#else 
#define ASNA_ENABLED 0
#endif

#if ASAN_ENABLED
C_LINKAGE void __asan_poison_memory_region(void const volatile* addr, size_t size);
C_LINKAGE void __asan_unpoison_memory_region(void const volatile* addr, size_t size);
# define AsanPoisonMemoryRegion(addr, size)   __asan_poison_memory_region((addr), (size))
# define AsanUnpoisonMemoryRegion(addr, size) __asan_unpoison_memory_region((addr), (size))
#else
# define AsanPoisonMemoryRegion(addr, size)   ((void)(addr), (void)(size))
# define AsanUnpoisonMemoryRegion(addr, size) ((void)(addr), (void)(size))
#endif

inline constexpr char Lower(char c)
{
	return c | 0x20;
}

inline constexpr u64 operator "" _KB(u64 n) 
{
	return 1024ULL * n;
}

inline constexpr u64 operator "" _MB(u64 n) 
{
	return 1024ULL * 1024ULL * n;
}

inline constexpr u64 operator "" _GB(u64 n) 
{
	return 1024ULL * 1024ULL * 1024ULL * n;
}

inline constexpr u64 operator "" _TB(u64 n) 
{
	return 1024ULL * 1024ULL * 1024ULL * 1024ULL * n;
}