/* date = August 20 2024 12:28 am*/

#ifndef INTRINSICS_H
#define INTRINSICS_H

#define CLOVER_SLOW 1
#if CLOVER_SLOW 

#define Check(Expression, Message) if(!(Expression)) {OutputDebugStringA(Message); DebugBreak();}
#define Assert(Expression) if(!(Expression)) {DebugBreak();}
#define InvalidCodePath DebugBreak()
#define Trace(Message) {OutputDebugStringA(Message);}
#define printm(Message, ...)  {char BUFFER[128] = {};  if(strlen(Message) > sizeof(BUFFER)) {Check(0, "stwing to warge >w<\n")}; sprintf(BUFFER, Message, __VA_ARGS__); OutputDebugStringA(BUFFER); printf("%s\n", BUFFER);}
#define printlm(Message, ...) {char BUFFER[5192] = {}; if(strlen(Message) > sizeof(BUFFER)) {Check(0, "stwing to warge >w<\n")}; sprintf(BUFFER, Message, __VA_ARGS__); OutputDebugStringA(BUFFER); printf("%s\n", BUFFER);}

#else

#define Assert(Expression)
#define Check(Expression, Message)
#define dAssert(Expression)
#define Trace(Message)
#define printm(Message, ...)
#define printlm(Message, ...)
#define InvalidCodePath

#endif

#define Kilobytes(Value) ((uint64)(Value) * 1024)
#define Megabytes(Value) ((uint64)Kilobytes(Value) * 1024)
#define Gigabytes(Value) ((uint64)Megabytes(Value) * 1024)

#define ArrayCount(Array) (sizeof(Array) / sizeof(Array[0]))

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// NOTE(Sleepster): Freetype must come first due to the #define internal static inside of the intrinsics header
#include "../data/deps/Freetype/include/ft2build.h"
#include FT_FREETYPE_H

#define global_variable static
#define local_persist   static
#define internal        static

#define external extern "C"

typedef uint8_t  uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

typedef int8_t   int8;
typedef int16_t  int16;
typedef int32_t  int32;
typedef int64_t  int64;

typedef int8     bool8;
typedef int32    bool32;

typedef float    real32;
typedef double   real64;

#define FIRST_ARG(arg1, ...) arg1
#define SECOND_ARG(arg1, arg2, ...) arg2

#define alignas(x) __declspec(align(x))

#include "util/CustomStrings.h"


#endif // _INTRINSICS_H

