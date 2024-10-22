#pragma once

#include <string.h>

#include "../Intrinsics.h"
#include "MemoryArena.h"

#define DEFAULT_CAPACITY 16;

// NOTE : This is a very naive implimentation, fix it later.

struct DynamicArray
{
    uint32 CurrentSize;
    uint32 MaxCapacity;
    void **Entries;
};

inline bool32
ArrayInit(BumpAllocator *Memory, DynamicArray *Array, uint32 Capacity) 
{   
    *(uint32 *)&Array->CurrentSize = 0;
    *(uint32 *)&Array->MaxCapacity = Capacity;
    if(Capacity <= 0) 
    {
        Capacity = DEFAULT_CAPACITY;
    }
    
    Array->Entries = (void **)BumpAllocate(Memory, sizeof(void *)*Capacity);
    if(Array->Entries == 0) 
    {
        return(0);
    }
    return(1);
}

inline bool32
ArrayGrow(ArenaAllocator *Memory, DynamicArray *Array)
{
    uint32 NewMaxCapacity = Array->MaxCapacity * 2;
    void **NewEntries = (void **)BumpAllocate(Memory, sizeof(void *)*NewMaxCapacity);
    
    for(uint32 Index = 0;
        Index < Array->CurrentSize;
        ++Index) 
    {
        NewEntries[Index] = Array->Entries[Index];
    }
    
    BumpDeallocate(Memory, (char *)Array->Entries);
    
    Array->Entries = NewEntries;
    *(uint32 *)&Array->MaxCapacity = NewMaxCapacity;
    return(1);
}

inline bool32 
ArrayAdd(DynamicArray *Array, ArenaAllocator *Memory, void *Element, uint32 Index) 
{ 
    // Move everything up one from the index
    for(uint32 ElementIndex = Array->CurrentSize;
        ElementIndex > Index; 
        --ElementIndex) 
    {
        Array->Entries[ElementIndex] = Array->Entries[ElementIndex - 1];
    }
    // Assign the index
    Array->Entries[Index] = Element;
    Array->CurrentSize++;
    return(1);
}

inline const void *
ArrayGetElement(DynamicArray *Array, uint32 Index) 
{
    return(Array->Entries[Index]);
}

inline void const *
ArraySwap(DynamicArray *Array, uint32 Index, void *Element) 
{ 
    void const *Result = 0;
    if(Index >= Array->CurrentSize) 
    {
        return(0);
    }
    Result = Array->Entries[Index];
    Array->Entries[Index] = Element;
    
    return(Result);
}

inline const void * 
ArrayRemove(DynamicArray *Array, uint32 Index) 
{ 
    const void *Result = Array->Entries[Index];
    
    for(-- *(uint32 *)&Array->CurrentSize; 
        Index <= Array->CurrentSize; 
        ++Index) 
    {
        Array->Entries[Index] = Array->Entries[Index + 1];
    }
    --Array->CurrentSize;
    return(Result);
}

inline void
ArrayDestroy(ArenaAllocator *Memory, DynamicArray *Array) 
{ 
    BumpDeallocate(Memory, (void *)Array);
}