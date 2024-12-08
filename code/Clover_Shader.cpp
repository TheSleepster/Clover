/* ========================================================================
   $File: Clover_Shader.cpp $
   $Date: Thu, 05 Dec 24: 11:15AM $
   $Revision: $
   $Creator: Justin Lewis $
   ======================================================================== */

struct clover_shader_source_data
{
    string Filepath;
    string Contents;
    time_t LastWriteTime;

    GLuint SourceID;
    GLenum Type;
};

struct clover_shader
{
    clover_shader_source_data Sources[6];
    int32                     SourceCount;
    GLuint                    ProgramID;
};

internal clover_shader_source_data
CloverLoadShaderSourceData(memory_arena *Arena, string Filepath, GLenum Type)
{
    clover_shader_source_data Result = {};
    
    uint32 Size = 0;
    Result.Contents = ReadEntireFileMA(Arena, Filepath, &Size);
    if(Result.Contents != NULLSTR)
    {
        Result.LastWriteTime = FileGetLastWriteTime(Filepath); 
        Result.Type = Type; 

        Result.SourceID = glCreateShader(Result.Type);
        glShaderSource(Result.SourceID, 1, &CSTR(Result.Contents), 0);
        glCompileShader(Result.SourceID);
        CloverTestShader(Result.SourceID, Result.Type);
    }
    else
    {
        cl_Error("File failed to load. It is either invalid or do not exist\n");
    }
    return(Result);
}

internal inline void
CloverAttachShaderSource(memory_arena *Arena, clover_shader *Shader, string Filepath, GLenum Type)
{
    Shader->Sources[Shader->SourceCount] =
        CloverLoadShaderSourceData(Arena, Filepath, Type);

    glAttachShader(Shader->ProgramID, Shader->Sources[Shader->SourceCount].SourceID);
    ++Shader->SourceCount;
}

internal inline void
CloverLinkShaderSources(clover_shader *Shader)
{
    glLinkProgram(Shader->ProgramID);
    CloverTestShader(Shader->ProgramID, GL_PROGRAM);
}

internal inline void
CloverCleanupShaderSources(clover_shader *Shader)
{
    for(int32 ShaderIndex = 0;
        ShaderIndex < Shader->SourceCount;
        ++ShaderIndex)
    {
        clover_shader_source_data *SourceData = &Shader->Sources[ShaderIndex];
        if(SourceData->Type != 0)
        {
            glDetachShader(Shader->ProgramID, SourceData->SourceID);
            glDeleteShader(SourceData->SourceID);
        }
    }
}

// NOTE(Sleepster): These are basic functions to generate generic shaders
internal clover_shader 
CloverLoadBasicPixelShader(memory_arena *Arena, string VertexFilepath, string FragmentFilepath)
{
    clover_shader Result = {};

    Result.Sources[0] = CloverLoadShaderSourceData(Arena, VertexFilepath, GL_VERTEX_SHADER);
    Result.Sources[1] = CloverLoadShaderSourceData(Arena, FragmentFilepath, GL_FRAGMENT_SHADER);

    Result.ProgramID = glCreateProgram();
    glAttachShader(Result.ProgramID, Result.Sources[0].SourceID);
    glAttachShader(Result.ProgramID, Result.Sources[1].SourceID);

    glLinkProgram(Result.ProgramID);
    CloverTestShader(Result.ProgramID, GL_PROGRAM);

    glDetachShader(Result.ProgramID, Result.Sources[0].SourceID);
    glDetachShader(Result.ProgramID, Result.Sources[1].SourceID);

    glDeleteShader(Result.Sources[0].SourceID);
    glDeleteShader(Result.Sources[1].SourceID);

    return(Result);
}

internal clover_shader 
CloverLoadComputeShader(memory_arena *Arena, string ComputeFilepath)
{
    clover_shader Result = {};
    Result.Sources[0] = CloverLoadShaderSourceData(Arena, ComputeFilepath, GL_COMPUTE_SHADER);

    Result.ProgramID = glCreateProgram();
    glAttachShader(Result.ProgramID, Result.Sources[0].SourceID);

    glLinkProgram(Result.ProgramID);
    CloverTestShader(Result.ProgramID, GL_PROGRAM);

    glDetachShader(Result.ProgramID, Result.Sources[0].SourceID);
    glDeleteShader(Result.Sources[0].SourceID);

    return(Result);
}
