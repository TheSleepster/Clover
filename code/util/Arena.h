#if !defined(ARENA_H)
/* ========================================================================
   $File: Arena.h $
   $Date: October 19 2024 04:51 pm $
   $Revision: $
   $Creator: Justin Lewis $
   ======================================================================== */

#define ARENA_H
#include "../Intrinsics.h"
#include <stdlib.h>
#include <string.h>

#if 0
// OLD API
struct memory_arena
{
    uint64 Capacity;
    uint64 Used;
    uint8 *Memory;
};

internal memory_arena
ArenaCreate(uint64 Size)
{
    memory_arena Arena = {};
    Arena.Memory = (uint8 *)malloc(Size);
    if(Arena.Memory)
    {
        Arena.Capacity = Size;
        memset(Arena.Memory, 0, Size);
    }
    return(Arena);    
}

internal uint8*
ArenaAlloc(memory_arena *Memory, uint64 Size)
{   
    uint8 *Result = {};
    uint64 AlignedSize = (Size + 7) & ~ 7;
    if(Memory->Used + AlignedSize <= Memory->Capacity)
    {
        Result = Memory->Memory + Memory->Used;
        Memory->Used += AlignedSize;
    }
    else
    {
        Check(0, "Allocation is too large!\n");
    }
    return(Result);
}

internal inline void
ArenaReset(memory_arena *Memory)
{
    memset(Memory->Memory, 0, sizeof(Memory->Capacity));
    Memory->Used = 0;
}

internal inline void
ArenaDestroy(memory_arena *Memory)
{
    free(Memory->Memory);
    Memory = {};
}
// OLD API
#endif

typedef size_t memory_index;

struct memory_arena
{
    memory_index  Capacity;
    memory_index  Used;
    uint8        *Base;

    int32 ScratchCount;
};

struct scratch_memory
{
    memory_arena *Arena;
    memory_index  Used;
};

// TODO(Sleepster): Add the ability to "free" the memory of the arena
#define PushSize(Arena, size, ...)                 PushSize_(Arena, size * sizeof(uint8), ##__VA_ARGS__)
#define PushStruct(Arena, type, ...)       (type *)PushSize_(Arena, sizeof(type), ##__VA_ARGS__)
#define PushArray(Arena, type, Count, ...) (type *)PushSize_(Arena, sizeof(type) * (Count), ##__VA_ARGS__)

internal inline uint8*
InitializeArena(memory_arena *Arena, memory_index Capacity, void *Base)
{
    Arena->Capacity     = Capacity;
    Arena->Used         = 0;
    Arena->Base         = (uint8 *)Base;
    Arena->ScratchCount = 0;

    // NOTE(Sleepster): Return the new offset 
    return((uint8 *)(Arena->Base + Arena->Capacity));
}

internal void*
PushSize_(memory_arena *Arena, memory_index Size, memory_index Alignment = 4)
{
    memory_index ResultOffset    = (memory_index)Arena->Base + Arena->Used;
    memory_index AlignmentOffset = 0;

    memory_index AlignmentMask = Alignment - 1;
    if(ResultOffset & AlignmentMask) // If the memory is misaligned
    {
        AlignmentOffset = Alignment - (ResultOffset & AlignmentMask); // align it 
    }
    Size += AlignmentOffset;

    Assert((Arena->Used + Size) <= Arena->Capacity);
    Arena->Used += Size;

    void *Result = (void *)(ResultOffset + AlignmentOffset);
    return(Result);
}

internal inline scratch_memory
BeginScratchBlock(memory_arena *Arena)
{
    scratch_memory Result;
    Result.Arena = Arena;
    Result.Used  = Arena->Used;

    ++Arena->ScratchCount;
    return(Result);
}

internal inline void
EndScratchBlock(scratch_memory *Scratch)
{
    memory_arena *Arena = Scratch->Arena;
    Assert(Arena->Used >= Scratch->Used);
    Assert(Arena->ScratchCount > 0);

    Arena->Used = Scratch->Used;
    Arena->ScratchCount--;
}

internal inline void
ClearArena(memory_arena *Arena)
{
    InitializeArena(Arena, Arena->Capacity, Arena->Base);
    Arena->Used = 0;
}

#endif // ARENA_H
