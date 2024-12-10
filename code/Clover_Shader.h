#if !defined(CLOVER_SHADER_H)
/* ========================================================================
   $File: Clover_Shader.h $
   $Date: Mon, 09 Dec 24: 11:47AM $
   $Revision: $
   $Creator: Justin Lewis $
   ======================================================================== */

#define CLOVER_SHADER_H
#include "Intrinsics.h"
#include "util/FileIO.h"

struct clover_shader_source_data
{
    string Filepath;
    string Contents;
    time_t LastWriteTime;

    uint32 SourceID;
    uint32 Type;
};

struct clover_shader
{
    clover_shader_source_data Sources[6];
    int32                     SourceCount;
    uint32                    ProgramID;
};

#endif
