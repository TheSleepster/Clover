#if !defined(INTRINSICS_H)
/* ========================================================================
   $File: Intrinsics.h $
   $Date: October 19 2024 04:46 pm $
   $Revision: $
   $Creator: Justin Lewis $
   ======================================================================== */

#define INTRINSICS_H
#if CLOVER_SLOW 

#ifdef _WIN32
#define Check(Expression, Message, ...) if(!(Expression)) {char CHECKBUFFER[1024] = {}; sprintf(CHECKBUFFER, Message, __VA_ARGS__); printf("%s\n", CHECKBUFFER); __debugbreak();}
#define Assert(Expression) if(!(Expression)) {__debugbreak();}
#define InvalidCodePath __debugbreak()
#define Trace(Message) {printm(Message)}
#define printm(Message, ...)  {char BUFFER[128] = {};  if(strlen(Message) > sizeof(BUFFER)) {Check(0, "stwing to warge >w<\n")}; sprintf(BUFFER, Message, ##__VA_ARGS__); printf("%s\n", BUFFER);}
#define printlm(Message, ...) {char BUFFER[5192] = {}; if(strlen(Message) > sizeof(BUFFER)) {Check(0, "stwing to warge >w<\n")}; sprintf(BUFFER, Message, ##__VA_ARGS__); printf("%s\n", BUFFER);}

#elif __linux__
#include <csignal>
#define Check(Expression, Message, ...) if(!(Expression)) {char CHECKBUFFER[1024] = {}; sprintf(CHECKBUFFER, Message, ##__VA_ARGS__); raise(SIGTRAP);}
#define Assert(Expression) if(!(Expression)) {raise(SIGTRAP);}
#define InvalidCodePath raise(SIGTRAP)
#define Trace(Message) {printm(Message)}
#define printm(Message, ...)  {char BUFFER[128] = {};  if(strlen(Message) > sizeof(BUFFER)) {Check(0, "stwing to warge >w<\n")}; sprintf(BUFFER, Message, ##__VA_ARGS__); printf("%s\n", BUFFER);}
#define printlm(Message, ...) {char BUFFER[5192] = {}; if(strlen(Message) > sizeof(BUFFER)) {Check(0, "stwing to warge >w<\n")}; sprintf(BUFFER, Message, ##__VA_ARGS__); printf("%s\n", BUFFER);}

#elif __APPLE__
#define Assert(Expression)
#define Check(Expression, Message, ...)
#define Trace(Message)
#define printm(Message, ...)
#define printlm(Message, ...)
#define InvalidCodePath
#endif

#else
#define Assert(Expression)
#define Check(Expression, Message)
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

#ifdef MSVC
#define alignas(x) __declspec(align(x))
#define inline     __forceinline
#endif

#endif // INTRINSICS_H

