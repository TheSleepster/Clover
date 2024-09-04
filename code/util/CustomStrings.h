/* date = July 25 2024 01:18 am*/

#ifndef SH_STRINGS_H
#define SH_STRINGS_H

#include "../Intrinsics.h"
#include "MemoryArena.h"

// NOTE(Sleepster): Length Based Strings
struct string
{
    uint64 Length;
    uint8 *Data;
};

#define STR(s)  (string{GetStringLength((const char *)s), (uint8 *)s})

const string NULL_STRING = {};

internal inline uint64
GetStringLength(const char *String)
{
    uint64 Length = 0;
    while(*String != 0)
    {
        ++Length;
        ++String;
    }
    return(Length);
}

internal inline bool
StringsMatch(string A, string B)
{
    if(A.Length != B.Length) return(0);
    if(A.Data == B.Data) return(1);
    
    return(memcmp(A.Data, B.Data, A.Length) == 0);
}

internal inline string
HeapString(memory_arena *Scratch, uint64 Length)
{
    string S;
    S.Length = Length;
    S.Data = (uint8 *)ArenaAlloc(Scratch, Length);
    
    return(S);
}

internal inline string
StringCopy(string A, memory_arena *Scratch)
{
    string C = HeapString(Scratch, A.Length);
    memcpy(C.Data, A.Data, A.Length);
    
    return(C);
}

internal inline string
ConcatinatePair(memory_arena *Arena, const string Left, const string Right)
{
    if(Right.Length + Left.Length == 0) return(NULL_STRING);
    if(Left.Length == 0) return(Right);
    if(Right.Length == 0) return(Left);
    
    string Result = {};
    Result.Length = Left.Length + Right.Length;
    Result.Data = (uint8 *)ArenaAlloc(Arena, Result.Length);
    
    const char *LeftS = (const char *)Left.Data;
    const char *RightS = (const char *)Right.Data;
    
    strcpy((char *)Result.Data, LeftS);
    strcpy((char *)Result.Data + Left.Length, RightS);
    
    // NOTE(Sleepster): memcpy is replaced because it caused weirdness when hotreloading (Shader Header was ending up in a sound filepath)
    //    memcpy(Result.Data, Left.Data, Left.Length);
    //    memcpy(Result.Data + Left.Length, Right.Data, Right.Length);
    
    return(Result);
}

internal inline string
CStringToString(const char *CString)
{
    string Result = {};
    Result.Data = (uint8 *)CString;
    Result.Length = strlen(CString);
    
    return(Result);
}

internal inline char *
StringToCString(memory_arena *Memory, const string String)
{
    char *CString = ArenaAlloc(Memory, sizeof(String.Data + 1));
    memcpy(CString, String.Data, String.Length);
    CString[String.Length] = 0;
    
    return(CString);
}

internal uint64
FormatStringToBuffer(char *Buffer, uint64 Count, const char* fmt, va_list args)
{
    if(!Buffer) (Count = UINT64_MAX);
    
    const char *temp = fmt;
    char *Bufferp = Buffer;
    
    while(*temp != '\0' && uint64((Bufferp - Buffer)) < Count - 1)
    {
        if(*temp == '%')
        {
            temp += 1;
            if(*temp == 's')
            {
                // NOTE(Sleepster): %s in formatting 
                temp += 1;
                string S = va_arg(args, string);
                Check(S.Length < (1024ULL * 1024ULL * 1024ULL * 256ULL), "This is not a fixed length 'string' to %%s. Check if it is a 'char*', if it is use %%cs instead");
                for(uint64 Index = 0;
                    Index < S.Length;
                    ++Index)
                {
                    if(Buffer) *Bufferp = S.Data[Index];
                    ++Bufferp;
                }
            }
            else if(*temp == 'c' && *(temp + 1) == 's')
            {
                // NOTE(Sleepster): Allows for support of formatting CStrings
                temp += 2;
                char *s = va_arg(args, char *);
                uint64 Length = 0;
                
                while(*s != '\0' && uint64(Bufferp - Buffer) < Count - 1)
                {
                    Check(Length < (1024ULL * 1024ULL * 1024ULL * 256ULL), "Brother you passed something that isn't a CString, missing null terminator\n");
                    if(Buffer)
                    {
                        *Bufferp = *s;
                    }
                    ++s;
                    ++Bufferp;
                    ++Length;
                    Check(Length < (1024ULL * 1024ULL * 1024ULL * 256ULL), "Brother you passed something that isn't a CString, missing null terminator\n");
                }
            }
            else
            {
                // NOTE(Sleepster): Standard vsnprintf 
                
                char TempFallback[512] = {};
                char FormatSpecifier[64] = {};
                int32 SpecifierLength = 0;
                
                FormatSpecifier[SpecifierLength++] = '%';
                while(*temp != '\0' && strchr("diuoxXfFeEgGaAcCpn%", *temp) == NULL)
                {
                    FormatSpecifier[SpecifierLength++] = *temp++;
                }
                if(*temp != '\0')
                {
                    FormatSpecifier[SpecifierLength++] = *temp++;
                }
                FormatSpecifier[SpecifierLength] = '\0';
                
                int32 TempLength = vsnprintf(TempFallback, sizeof(TempFallback), FormatSpecifier, args);
                switch (FormatSpecifier[SpecifierLength - 1]) 
                {
                    case 'd': case 'i': va_arg(args, int); break;
                    case 'u': case 'x': case 'X': case 'o': va_arg(args, unsigned int); break;
                    case 'f': case 'F': case 'e': case 'E': case 'g': case 'G': case 'a': case 'A': va_arg(args, double); break;
                    case 'c': va_arg(args, int); break;
                    case 's': va_arg(args, char*); break;
                    case 'p': va_arg(args, void*); break;
                    case 'n': va_arg(args, int*); break;
                    default: break;
                }
                
                if(TempLength < 0)
                {
                    return 0; // Error;
                }
                
                for(int32 Index = 0;
                    Index < TempLength && uint64(Bufferp - Buffer) < Count - 1;
                    ++Index)
                {
                    if(Buffer) *Bufferp = TempFallback[Index];
                    ++Bufferp;
                }
            }
        }
        else 
        {
            if(Buffer) 
            {
                *Bufferp = *temp;
            }
            ++Bufferp;
            ++temp;
        }
    }
    if(Buffer) *Bufferp = '\0';
    
    return(Bufferp - Buffer);
}

internal inline string
SprintCStringArgsToBuffer(const char *fmt, va_list args, void *Buffer, uint64 BufferSize)
{
    uint64 FormatLength = FormatStringToBuffer((char *)Buffer, BufferSize, fmt, args);
    
    string Result = {};
    Result.Data = (uint8 *)Buffer;
    
    if(FormatLength >= 0 && FormatLength < BufferSize)
    {
        Result.Length = FormatLength;
    }
    else
    {
        Result.Length = BufferSize - 1;
    }
    
    return(Result);
}

internal string
SprintVAList(memory_arena *Memory, const string fmt, va_list args)
{
    char *fmt_cstring = StringToCString(Memory, fmt);
    uint64 Count = FormatStringToBuffer(NULL, 0, fmt_cstring, args) + 1;
    
    char* Buffer = {};
    Buffer = ArenaAlloc(Memory, Count);
    
    return(SprintCStringArgsToBuffer(fmt_cstring, args, Buffer, Count));
}

// NOTE(Sleepster): This ends up formatting similar to that of printf
internal string
sprints(memory_arena *Memory, const string Text, ...)
{
    string sfmt = {};
    sfmt.Data = Text.Data;
    sfmt.Length = Text.Length;
    
    va_list args;
    va_start(args, Text);
    string S = SprintVAList(Memory, sfmt, args);
    va_end(args);
    
    return(S);
}

// NOTE(Sleepster): This is literally the same thing, but will instead just straight up take a char *
internal string
sprintd(memory_arena *Memory, const char *Text, ...)
{
    string sfmt = {};
    sfmt.Data = (uint8 *)Text;
    sfmt.Length = strlen(Text);
    
    va_list args;
    va_start(args, Text);
    string S = SprintVAList(Memory, sfmt, args);
    va_end(args);
    
    return(S);
}

internal char *
ConcatinateCString(memory_arena *Memory, char *A, char *B)
{
    char *Result = {};
    Result = ArenaAlloc(Memory, strlen(A) + strlen(B) + 1);
    
    memcpy(Result, A, strlen(A));
    memcpy(Result + strlen(A), B, strlen(B) + 1);
    return(Result);
}

#endif // _SH_STRINGS_H
