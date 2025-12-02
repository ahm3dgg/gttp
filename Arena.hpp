#ifndef _ARENA_H
#define _ARENA_H

#include "Defines.hpp"

typedef unsigned long long u64;

#define ARENA_HEADER_SIZE 128

typedef u64 ArenaFlags;
enum {
    ArenaFlag_NoChain    = (1 << 0),
    ArenaFlag_LargePages = (1 << 1),
};

typedef struct ArenaParams ArenaParams;
struct ArenaParams {
    ArenaFlags flags;
    u64 reserve_size;
    u64 commit_size;
    void *optional_backing_buffer;
    char *allocation_site_file;
    int allocation_site_line;
};

typedef struct Arena Arena;
struct Arena {
    Arena *prev;    // previous arena in chain
    Arena *current; // current arena in chain
    ArenaFlags flags;
    u64 cmt_size;
    u64 res_size;
    u64 base_pos;
    u64 pos;
    u64 aligned_cmt_size;
    u64 aligned_res_size;
    char *allocation_site_file;
    int allocation_site_line;
#if ARENA_FREE_LIST
    u64 free_size;
    Arena *free_last;
#endif
};

typedef struct Temp Temp;
struct Temp {
    Arena *arena;
    u64 pos;
};

constexpr u64 AreanDefaultReserverSize = MB(64);
constexpr u64 ArenaDefaultCommitSize  = KB(64);
constexpr ArenaFlags ArenaDefaultFlags = 0;

Arena *_ArenaAlloc(ArenaParams *params);
Arena* ArenaAlloc();

void ArenaRelease(Arena *arena);

void *ArenaPush(Arena *arena, u64 size, u64 align);
u64 ArenaPos(Arena *arena);
void ArenaPopTo(Arena *arena, u64 pos);

void ArenaClear(Arena *arena);
void ArenaPop(Arena *arena, u64 amt);

Temp TempBegin(Arena *arena);
void TempEnd(Temp temp);

#define PushArrayNoZeroAligned(a, T, c, align) (T *)ArenaPush((a), sizeof(T) * (c), (align))
#define PushArrayAligned(a, T, c, align) (T *)MemoryZero(PushArrayNoZeroAligned(a, T, c, align), sizeof(T) * (c))
#define PushArrayNoZero(a, T, c) PushArrayNoZeroAligned(a, T, c, Max(8, AlignOf(T)))
#define PushArray(a, T, c) PushArrayAligned(a, T, c, Max(8, AlignOf(T)))
#define PushVarSzStruct(a, T, sz) (T *)ArenaPush((a), sz, Max(8, AlignOf(T)))
#define PushBytes(a, c) (char*)ArenaPush((a), (c), 1)

#endif
