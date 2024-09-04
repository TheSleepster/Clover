#pragma once
#include "../Intrinsics.h"

#ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>

// NOTE(Sleepster): Still have no idea if this freelist Idea actually works
struct free_list 
{
    char *FreeMemory;
    struct free_list *NextChunk;
};

struct memory_arena 
{
    uint64 Capacity;
    uint64 Used;
    char* Memory;
    
    free_list *FreeList;
};

internal memory_arena 
ArenaCreate(uint64 Size) 
{
    memory_arena Arena = {};
    Arena.Memory = (char *)VirtualAlloc(0, Size, MEM_COMMIT, PAGE_READWRITE);
    if(Arena.Memory) 
    {
        Arena.Capacity = Size;
        Arena.FreeList = 0;
        memset(Arena.Memory, 0, Size);
    }
    return(Arena);
}

// TODO(Sleepster): Check that the allignment size is actually correct
internal char *
ArenaAlloc(memory_arena *Arena, uint64 Size) 
{
    char *Result = nullptr;
    uint64 AllignedSize = (Size + 7) & ~ 7;
    if(Arena->FreeList) 
    {
        free_list *Chunk = Arena->FreeList;
        Arena->FreeList = Chunk->NextChunk;
        
        return((char *)Chunk->FreeMemory);
    }
    
    if(Arena->Used + AllignedSize <= Arena->Capacity) 
    {
        Result = Arena->Memory + Arena->Used;
        Arena->Used += AllignedSize;
    }
    else 
    {
        printf("Allocation would exceed Arena Capacity!\n");
        Assert(false);
    }
    return(Result);
}

internal inline void
ArenaDealloc(memory_arena *Arena, void *Data) 
{
    free_list *FreeList = {};
    FreeList->FreeMemory = (char *)Data;
    FreeList->NextChunk = Arena->FreeList;
    Arena->FreeList = FreeList;
}

internal inline void 
ArenaReset(memory_arena *Arena) 
{
    memset(Arena->Memory, 0, sizeof(Arena->Capacity));
    Arena->Used = 0;
    Arena->FreeList = 0;
}

internal inline void 
ArenaDestroy(memory_arena *Arena) 
{
    VirtualFree(Arena->Memory, Arena->Capacity, MEM_RELEASE);
    Arena = {};
}
