#include "Arena.hpp"
#include "Utils.h"
#include "OS.hpp"

Arena *_ArenaAlloc(ArenaParams *params)
{
    u64 reserve_size = params->reserve_size;
    u64 commit_size  = params->commit_size;

    u64 page_size = OsGetPageSize();
    size_t large_page_size = OsGetLargePageSize();

    if (params->flags & ArenaFlag_LargePages)
    {
        reserve_size = AlignPow2(reserve_size, large_page_size);
        commit_size  = AlignPow2(commit_size, large_page_size);
    } else
    {
        reserve_size = AlignPow2(reserve_size, page_size);
        commit_size  = AlignPow2(commit_size, page_size);
    }

    void *base = params->optional_backing_buffer;
    if (base == 0)
    {
        if (params->flags & ArenaFlag_LargePages)
        {
            base = OsMemoryReserveLarge(reserve_size);
            OsMemoryCommitLarge(base, commit_size);
        } else
        {
            base = OsMemoryReserve(reserve_size);
            OsMemoryCommit(base, commit_size);
        }
    }

    Arena *arena                = (Arena *)base;
    arena->current              = arena;
    arena->flags                = params->flags;
    arena->cmt_size             = params->commit_size;
    arena->res_size             = params->reserve_size;
    arena->base_pos             = 0;
    arena->pos                  = ARENA_HEADER_SIZE;
    arena->aligned_cmt_size     = commit_size;
    arena->aligned_res_size     = reserve_size;
    arena->allocation_site_file = params->allocation_site_file;
    arena->allocation_site_line = params->allocation_site_line;
#if ARENA_FREE_LIST
    arena->free_size = 0;
    arena->free_last = 0;
#endif
	AsanPoisonMemoryRegion(base, commit_size);
	AsanUnpoisonMemoryRegion(base, ARENA_HEADER_SIZE);
    return arena;
}

Arena* ArenaAlloc()
{
    ArenaParams arenaParams = {
            .flags = ArenaDefaultFlags,
            .reserve_size = AreanDefaultReserverSize, 
            .commit_size = ArenaDefaultCommitSize, 
    };

    return _ArenaAlloc(&arenaParams);
}

void ArenaRelease(Arena *arena)
{
    for (Arena *n = arena->current, *prev = 0; n != 0; n = prev)
    {
        prev = n->prev;
        OsMemoryRelease(n, n->aligned_res_size);
    }
}

void *ArenaPush(Arena *arena, u64 size, u64 align)
{
    Arena *current = arena->current;
    u64 pos_old    = AlignPow2(current->pos, align);
    u64 pos_new    = pos_old + size;

    if (current->aligned_res_size < pos_new && !(arena->flags & ArenaFlag_NoChain))
    {
        Arena *new_block = 0;

#if ARENA_FREE_LIST
        Arena *prev_block;
        for (new_block = arena->free_last, prev_block = 0; new_block != 0;
             prev_block = new_block, new_block = new_block->prev)
        {
            if (new_block->aligned_res_size >= AlignPow2(size, align))
            {
                if (prev_block)
                {
                    prev_block->prev = new_block->prev;
                } else
                {
                    arena->free_last = new_block->prev;
                }
                arena->free_size -= new_block->res_size;
				AsanUnpoisonMemoryRegion((u8*)new_block + ARENA_HEADER_SIZE, new_block->res_size - ARENA_HEADER_SIZE);
                break;
            }
        }
#endif

        if (new_block == 0)
        {
            u64 res_size = current->res_size;
            u64 cmt_size = current->cmt_size;
            if (size + ARENA_HEADER_SIZE > res_size)
            {
                res_size = AlignPow2(size + ARENA_HEADER_SIZE, align);
                cmt_size = AlignPow2(size + ARENA_HEADER_SIZE, align);
            }
            ArenaParams params = {
                .flags = current->flags,
                .reserve_size            = res_size,
                .commit_size             = cmt_size,
                .optional_backing_buffer = NULL,
                .allocation_site_file    = current->allocation_site_file,
                .allocation_site_line    = current->allocation_site_line
            };

            new_block = _ArenaAlloc(&params);
        }

        new_block->base_pos = current->base_pos + current->aligned_res_size;
        SLLStackPush_N(arena->current, new_block, prev);

        current = new_block;
        pos_old = AlignPow2(current->pos, align);
        pos_new = pos_old + size;
    }

    if (current->aligned_cmt_size < pos_new)
    {
        size_t cmt_new = Min(Align(pos_new, current->cmt_size), current->aligned_res_size);
        size_t cmt_size = cmt_new - current->aligned_cmt_size;
        u8 *cmt_ptr = (u8 *)current + current->aligned_cmt_size;

        if (current->flags & ArenaFlag_LargePages)
        {
            OsMemoryCommitLarge(cmt_ptr, cmt_size);
        } 
        
        else
        {
            OsMemoryCommit(cmt_ptr, cmt_size);
        }

        current->aligned_cmt_size = cmt_new;
    }

    void *result = nullptr;
    if (current->aligned_cmt_size >= pos_new)
    {
        result       = (u8 *)current + pos_old;
        current->pos = pos_new;
        AsanUnpoisonMemoryRegion(result, size);
    }

    return result;
}

u64 ArenaPos(Arena *arena)
{
    Arena *current = arena->current;
    u64 pos        = current->base_pos + current->pos;
    return pos;
}

void ArenaPopTo(Arena *arena, u64 pos)
{
    u64 big_pos    = ClampBot(ARENA_HEADER_SIZE, pos);
    Arena *current = arena->current;

#if ARENA_FREE_LIST
    for (Arena *prev = 0; current->base_pos >= big_pos; current = prev)
    {
        prev              = current->prev;
        current->pos      = ARENA_HEADER_SIZE;
        arena->free_size += current->res_size;
        SLLStackPush_N(arena->free_last, current, prev);
        AsanPoisonMemoryRegion((u8*)current + ARENA_HEADER_SIZE, current->res_size - ARENA_HEADER_SIZE);
    }
#else
    for (Arena *prev = 0; current->base_pos >= big_pos; current = prev)
    {
        prev = current->prev;
        OsMemoryRelease(current, current->aligned_res_size);
    }
#endif
    arena->current = current;
    u64 new_pos    = big_pos - current->base_pos;
	AssertAlways(new_pos <= current->pos);
	AsanPoisonMemoryRegion((u8*)current + new_pos, (current->pos - new_pos));
    current->pos = new_pos;
}

void ArenaClear(Arena *arena)
{
    ArenaPopTo(arena, 0);
}

void ArenaPop(Arena *arena, u64 amt)
{
    u64 pos_old = ArenaPos(arena);
    u64 pos_new = pos_old;
    if (amt < pos_old)
    {
        pos_new = pos_old - amt;
    }
    ArenaPopTo(arena, pos_new);
}

Temp TempBegin(Arena *arena)
{
    u64 pos   = ArenaPos(arena);
    Temp temp = {arena, pos};
    return temp;
}

void TempEnd(Temp temp)
{
    ArenaPopTo(temp.arena, temp.pos);
}
