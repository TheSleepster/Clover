#ifndef FILE_IO_H
#define FILE_IO_H
#include "../Intrinsics.h"
#include "MemoryArena.h"

internal int32
GetFileSizeInBytes(string Filepath)
{
    int32 FileSize = 0;
    FILE *File = fopen((const char *)Filepath.Data, "rb");
    
    fseek(File, 0, SEEK_END);
    FileSize = ftell(File);
    fseek(File, 0, SEEK_SET);
    fclose(File);
    
    return(FileSize);
}

internal char *
ReadEntireFile(string Filepath, uint32 *Size, char *Buffer)
{
    Check(Filepath.Data != nullptr, "Cannot find the file designated!\n");
    Check(Buffer != nullptr, "Provide a valid buffer!\n");
    Check(*Size >= 0, "Size is less than 0!\n");
    
    *Size = 0;
    FILE *File = fopen((const char *)Filepath.Data, "rb");
    
    fseek(File, 0, SEEK_END);
    *Size = ftell(File);
    fseek(File, 0, SEEK_SET);
    
    memset(Buffer, 0, *Size + 1);
    fread(Buffer, sizeof(char), *Size, File);
    
    fclose(File);
    return(Buffer);
}

internal string
ReadEntireFileMA(memory_arena *ArenaAllocator, string Filepath, uint32 *FileSize)
{
    string File = {};
    int32 FileSize2 = GetFileSizeInBytes(Filepath);
    Check(FileSize2 >= 0, "FileSize is less than 0!\n");
    
    char *Buffer = (char *)ArenaAlloc(ArenaAllocator, uint64(FileSize2 + 1));
    File.Data = (uint8 *)ReadEntireFile(Filepath, FileSize, Buffer);
    File.Length = FileSize2;
    
    return(File);
}
#endif // FILE_IO_H
