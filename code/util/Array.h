/* date = July 31st 2024 6:22 pm */

#ifndef _SH_ARRAY_H
#define _SH_ARRAY_H

#include "../Intrinsics.h"
#include "MemoryArena.h"

// NOTE(Sleepster): Just a test, C++ makes me vomit
template <typename Type, int32 Capacity>
struct array
{
    int32 Count = 0;
    Type  Data[Capacity];

    inline void
    MakeHeap(memory_arena *Memory)
    {
        Data = (Type *)ArenaAlloc(Memory, sizeof(Type) * Capacity);
        for(int Index = 0; Index < Capacity; ++Index)
        {
            Type Element = Data[Index];
            memset(&Element, 0, (sizeof(Type)));
        }
    }

    int32 
    Add(Type Element)
    {
        Check(Count >= 0, "Invalid Count\n");
        Check(Count < Capacity, "Array Full\n");

        Data[Count] = Element;
        return(++Count);
    }

    inline void
    Remove(int32 Index)
    {
        Check(Index >= 0, "Invalid Index\n");
        Check(Index < Count, "Index Is Invalid\n");

        Data[Index] = Data[--Count];
    }

    inline void
    Clear()
    {
        Count = 0;
    }

    inline bool
    Full()
    {
        return(Count >= Capacity);
    }

    inline Type& 
    operator[](int32 Index)
    {
        Check(Index >= 0, "Invalid Index\n");
        Check(Index < Capacity, "Invalid Index\n");

        return(Data[Index]);
    }
};

#endif
