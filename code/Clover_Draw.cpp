#include "Clover_Renderer.h"

#include "../data/deps/Freetype/include/ft2build.h"
#include FT_FREETYPE_H

#include "Intrinsics.h"

#include "util/Math.h"
#include "util/Array.h"
#include "util/FileIO.h"
#include "util/CustomStrings.h"

// GLAD
#ifndef GLAD_OPENGL_IMPL
#define GLAD_OPENGL_IMPL
#include "../data/deps/OpenGL/glad/src/glad.c"
#include "../data/deps/OpenGL/glad/include/glad/glad.h"

#include "../data/deps/OpenGL/glext.h"
#include "../data/deps/OpenGL/wglext.h"
#include "../data/deps/OpenGL/glcorearb.h"

#include "../data/deps/stb/stb_image.h"
#include "../data/deps/stb/stb_image_write.h"
#endif

#include "shader/ShaderHeader.h"
#include "Clover_Globals.h"

internal void APIENTRY
OpenGLDebugMessageCallback(GLenum Source, GLenum Type, GLuint ID, GLenum Severity,
                           GLsizei Length, const GLchar *Message, const void *UserParam)
{
    if(Severity == GL_DEBUG_SEVERITY_LOW||
       Severity == GL_DEBUG_SEVERITY_MEDIUM||
       Severity == GL_DEBUG_SEVERITY_HIGH)
    {
        printlm("Error: %s\n", Message);
        Assert(false);
    }
    else
    {
        // NOTE(Sleepster): We don't have Tracing here because ImGui will BLAST my output window otherwise 
    }
}

internal void
glVerifyIVStatus(GLuint TestID, GLuint Type)
{
    bool32 Success = {};
    char ShaderLog[2048] = {};

    switch(Type)
    {
        case GL_VERTEX_SHADER:
        case GL_FRAGMENT_SHADER:
        {
            glGetShaderiv(TestID, GL_COMPILE_STATUS, &Success);
        }break;
        case GL_PROGRAM:
        {
            glGetProgramiv(TestID, GL_LINK_STATUS, &Success);
        }break;
    }

    if(!Success)
    {
        glGetShaderInfoLog(TestID, 2048, 0, ShaderLog);
        printm("ERROR ON SHADER COMPILATION: %s\n", ShaderLog);
        Assert(false);
    }
}

internal void
CloverLoadFont(memory_arena *Memory, gl_render_data *RenderData, string Filepath, uint32 FontSize, font_index FontIndex)
{
    freetype_font_data Font = {};
    Font.FontSize = FontSize;
    FT_Error Error;

    Error = FT_Init_FreeType(&Font.FontFile);
    Check(Error == 0, "Failed to initialize Freetype\n");
    
    Error = FT_New_Face(Font.FontFile, (const char *)Filepath.Data, 0, &Font.FontFace);
    Check(Error == 0, "Failed to initialize the Font Face\n");
    Check(Error != FT_Err_Unknown_File_Format, "Failed to load the font file, it is found but not supported\n");

    // NOTE(Sleepster): Test, we would normally use FT_Set_Pixel_Sizes(); 
    Error = FT_Set_Char_Size(Font.FontFace, 0, Font.FontSize, SizeData.Width, SizeData.Height);
    Check(Error == 0, "Issue setting the pixel size of the font\n");
    
    Font.AtlasPadding = 20;
    int32 Row = {};
    int32 Column = Font.AtlasPadding;

    char *TextureData = ArenaAlloc(Memory, (uint64)(sizeof(char) * (BITMAP_ATLAS_SIZE * BITMAP_ATLAS_SIZE)));
    if(TextureData)
    {
        for(uint32 GlyphIndex = 32;
            GlyphIndex < 127;
            ++GlyphIndex)
        {
            FT_UInt Glyph = FT_Load_Char(Font.FontFace, GlyphIndex, FT_LOAD_RENDER);
            if(Column + Font.FontFace->glyph->bitmap.width + Font.AtlasPadding >= BITMAP_ATLAS_SIZE)
            {
                Column = Font.AtlasPadding;
                // NOTE(Sleepster): This is a product of our "test". Freetype uses >> 6 on sizes, so we divide by 6 to offset it 
                Row += int32(Font.FontSize / 3.0f);
            }
            // NOTE(Sleepster): Leaving this here would make it only work for one font, perhaps make it so we pass in a font index into an array of 
            //                  fonts? Simply have a cap for the amount of fonts that can be loaded at any one point.
            RenderData->LoadedFonts[FontIndex].FontHeight = Max((Font.FontFace->size->metrics.ascender - Font.FontFace->size->metrics.descender) >> 6,
                                                                 (int32)RenderData->LoadedFonts[FontIndex].FontHeight);
            for(uint32 YIndex = 0;
                YIndex < Font.FontFace->glyph->bitmap.rows;
                ++YIndex)
            {
                for(uint32 XIndex = 0;
                    XIndex < Font.FontFace->glyph->bitmap.width;
                    ++XIndex)
                {
                    TextureData[(Row + YIndex) * BITMAP_ATLAS_SIZE + (Column + XIndex)] = 
                    Font.FontFace->glyph->bitmap.buffer[YIndex * Font.FontFace->glyph->bitmap.width + XIndex];
                }
            }
            font_glyph *CurrentGlyph = &RenderData->LoadedFonts[FontIndex].Glyphs[GlyphIndex];
            CurrentGlyph->GlyphUVs  = {Column, Row};
            CurrentGlyph->GlyphSize = 
            {
                (int32)Font.FontFace->glyph->bitmap.width, 
                (int32)Font.FontFace->glyph->bitmap.rows
            };
            CurrentGlyph->GlyphAdvance = 
            {
                real32(Font.FontFace->glyph->advance.x >> 6), 
                real32(Font.FontFace->glyph->advance.y >> 6)
            };
            CurrentGlyph->GlyphOffset = 
            {
                real32(Font.FontFace->glyph->bitmap_left),
                real32(Font.FontFace->glyph->bitmap_top)
            };

            Column += Font.FontFace->glyph->bitmap.width + Font.AtlasPadding;
        }

        FT_Done_Face(Font.FontFace);
        FT_Done_FreeType(Font.FontFile);

        // OPENGL TEXTURE
        {
            glActiveTexture(GL_TEXTURE0 + RenderData->TextureCount);
            ++RenderData->TextureCount;

            glGenTextures(1, &RenderData->LoadedFonts[FontIndex].FontAtlas.TextureID);
            glBindTexture(GL_TEXTURE_2D, RenderData->LoadedFonts[FontIndex].FontAtlas.TextureID);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

            glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, BITMAP_ATLAS_SIZE, BITMAP_ATLAS_SIZE, 0, GL_RED, GL_UNSIGNED_BYTE, TextureData);
            glBindTexture(GL_TEXTURE_2D, 0);
        }
    }
    else
    {
        Check(0, "Failed to Allocate Storage\n");
    }
}

internal gl_shader_source
CloverLoadShaderSource(memory_arena *Scratch, uint32 ShaderType, string Filepath)
{
    uint32 FileSize = 0;
    gl_shader_source ReturnShader = {};
    
    string ShaderHeader = ReadEntireFileMA(Scratch, STR("../code/shader/ShaderHeader.h"), &FileSize);
    string ShaderSource = ReadEntireFileMA(Scratch, Filepath, &FileSize);
    
    ReturnShader.Filepath = Filepath;
    
    if(ShaderSource.Data && ShaderHeader.Data)
    {
        char *ShaderSources[] = 
        {
            "#version 430 core\n",
            (char *)ShaderHeader.Data,
            (char *)ShaderSource.Data
        };
        
        ReturnShader.SourceID = glCreateShader(ShaderType);
        glShaderSource(ReturnShader.SourceID, ArrayCount(ShaderSources), ShaderSources, 0);
        glCompileShader(ReturnShader.SourceID);
        glVerifyIVStatus(ReturnShader.SourceID, GL_VERTEX_SHADER);
    }
    else
    {
        Trace("File is either not found or does not contain strings!\n");
        Assert(false);
    }
    return(ReturnShader);
}

internal shader
CloverCreateShader(memory_arena *Memory, string VertexShader, string FragmentShader)
{
    glUseProgram(0);
    shader ReturnShader = {};

    ReturnShader.VertexShader   = CloverLoadShaderSource(Memory, GL_VERTEX_SHADER, VertexShader);
    ReturnShader.FragmentShader = CloverLoadShaderSource(Memory, GL_FRAGMENT_SHADER, FragmentShader);

    ReturnShader.VertexShader.LastWriteTime   = Win32GetLastWriteTime(VertexShader);
    ReturnShader.FragmentShader.LastWriteTime = Win32GetLastWriteTime(FragmentShader);

    ReturnShader.Shader = glCreateProgram();
    glAttachShader(ReturnShader.Shader, ReturnShader.VertexShader.SourceID);
    glAttachShader(ReturnShader.Shader, ReturnShader.FragmentShader.SourceID);
    glLinkProgram(ReturnShader.Shader);
    
    glVerifyIVStatus(ReturnShader.Shader, GL_PROGRAM);
    
    glDetachShader(ReturnShader.Shader, ReturnShader.VertexShader.SourceID);
    glDetachShader(ReturnShader.Shader, ReturnShader.FragmentShader.SourceID);
    glDeleteShader(ReturnShader.VertexShader.SourceID);
    glDeleteShader(ReturnShader.FragmentShader.SourceID);
    
    return(ReturnShader);
}

internal void
CloverLoadTexture(gl_render_data *RenderData, texture2d *TextureInfo, string Filepath)
{
    glActiveTexture(GL_TEXTURE0 + RenderData->TextureCount);
    RenderData->TextureCount++;

    glGenTextures(1, &TextureInfo->TextureID);
    glBindTexture(GL_TEXTURE_2D, TextureInfo->TextureID);

    TextureInfo->Filepath = Filepath;
    TextureInfo->LastWriteTime = Win32GetLastWriteTime(Filepath);
    TextureInfo->RawData = (char *)stbi_load((const char *)Filepath.Data, 
                                              &TextureInfo->TextureData.Width, 
                                              &TextureInfo->TextureData.Height, 
                                              &TextureInfo->TextureData.Channels, 
                                              4);
    if(TextureInfo->RawData)
    {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        
        glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB8_ALPHA8, 
                     TextureInfo->TextureData.Width, TextureInfo->TextureData.Height, 0, 
                     GL_RGBA, GL_UNSIGNED_BYTE, TextureInfo->RawData);
    }
    stbi_image_free(TextureInfo->RawData);
    glBindTexture(GL_TEXTURE_2D, 0);
}

internal void
CloverReloadTexture(gl_render_data *RenderData, texture2d *TextureInfo, uint32 TextureIndex)
{
    glBindTexture(GL_TEXTURE_2D, TextureInfo->TextureID);

    TextureInfo->LastWriteTime = Win32GetLastWriteTime(TextureInfo->Filepath);
    TextureInfo->RawData = (char *)stbi_load((const char *)TextureInfo->Filepath.Data, 
                                              &TextureInfo->TextureData.Width, 
                                              &TextureInfo->TextureData.Height, 
                                              &TextureInfo->TextureData.Channels, 
                                              4);
    if(TextureInfo->RawData)
    {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB8_ALPHA8, 
                     TextureInfo->TextureData.Width, TextureInfo->TextureData.Height, 0, 
                     GL_RGBA, GL_UNSIGNED_BYTE, TextureInfo->RawData);
    }
    stbi_image_free(TextureInfo->RawData);
    glBindTexture(GL_TEXTURE_2D, 0);
}

internal void
CloverSetupRenderer(memory_arena *Memory, gl_render_data *RenderData)
{
    // STATE INITIALIZATION
    {
        glDebugMessageCallback(&OpenGLDebugMessageCallback, nullptr);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        glEnable(GL_DEBUG_OUTPUT);

        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_GREATER);
 
        glEnable(GL_BLEND);        // ImGui might use blending for its UI elements
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        glEnable(GL_FRAMEBUFFER_SRGB);
        glDisable(0x809D); // Disabling multisampling
        
        RenderData->ClearColor = DARK_GRAY;
    }

    uint32 Indices[MAX_INDICES] = {};
    uint32 Offset               = 0;
    for(uint32 Index = 0;
        Index < MAX_INDICES;
        Index += 6)
    {
        Indices[Index + 0] = Offset + 0;
        Indices[Index + 1] = Offset + 1;
        Indices[Index + 2] = Offset + 2;
        Indices[Index + 3] = Offset + 2;
        Indices[Index + 4] = Offset + 3;
        Indices[Index + 5] = Offset + 0;
        Offset += 4;
    }

    // GAME ASSETS BUFFER SETUP
    {
        glGenVertexArrays(1, &RenderData->GameVAOID);
        glBindVertexArray(RenderData->GameVAOID);

        glGenBuffers(1, &RenderData->GameVBOID);
        glBindBuffer(GL_ARRAY_BUFFER, RenderData->GameVBOID);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertex) * MAX_VERTICES, 0, GL_DYNAMIC_DRAW);

        glGenBuffers(1, &RenderData->GameEBOID);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, RenderData->GameEBOID);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Indices), Indices, GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vertex), (void *)offsetof(vertex, Position));
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(vertex), (void *)offsetof(vertex, TextureCoords));
        glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(vertex), (void *)offsetof(vertex, MatColor));
        glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(vertex), (void *)offsetof(vertex, TextureIndex));

        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);
        glEnableVertexAttribArray(2);
        glEnableVertexAttribArray(3);
    }

    // SHADER SETUP
    {
        RenderData->BasicShader = 
            CloverCreateShader(Memory, STR("../code/shader/Basic_vert.glsl"), STR("../code/shader/Basic_frag.glsl"));
    }


    // TEXTURE/FONT LOADING
    {
        // NOTE(Sleepster): The order is important, whatever you gen first will end up in the GL_TEXTUREX slot 
        CloverLoadTexture(RenderData, &RenderData->GameAtlas, STR("../data/res/textures/TextureAtlas.png"));
        CloverLoadFont(Memory, RenderData, STR("../data/res/fonts/UbuntuMono-B.ttf"), 100, UBUNTU_MONO);
    }


    // CAMERAS/MATRICES
    {
        RenderData->GameCamera.ViewMatrix  = mat4Identity(1.0f);
        RenderData->ProjectionViewMatrixID = glGetUniformLocation(RenderData->BasicShader.Shader, "ProjectionViewMatrix");
    }

    // LOGGING 
    {
        glGetString(GL_VENDOR);
    }
}
    
// TODO(Sleepster): Make an array of textures and bind this way
#if 0
    for(uint32 TextureIndex = 0; 
        TextureIndex < RenderData->TextureCount;
        ++TextureIndex)
    {
    }
#endif

internal void
CloverRender(gl_render_data *RenderData)
{
    if(RenderData->DrawFrame.QuadCount == 0) return;

    // TODO(Sleepster): Make an array of textures to bind too, perhaps make a seperate VBO for the fonts 
    //glBindTexture(GL_TEXTURE_2D, RenderData->LoadedFonts[UBUNTU_MONO].FontAtlas.TextureID);
 

    // GAME TEXTURE RENDERING PASS
    {
        glUseProgram(RenderData->BasicShader.Shader);
        glBindBuffer(GL_ARRAY_BUFFER, RenderData->GameVBOID);
        glBufferSubData(GL_ARRAY_BUFFER, 0, (RenderData->DrawFrame.QuadCount * 4) * sizeof(vertex), RenderData->DrawFrame.Vertices);

        glUniformMatrix4fv(RenderData->ProjectionViewMatrixID, 1, GL_FALSE, &RenderData->GameCamera.ProjectionViewMatrix.Elements[0][0]);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, RenderData->GameAtlas.TextureID);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, RenderData->LoadedFonts[UBUNTU_MONO].FontAtlas.TextureID);

        glBindVertexArray(RenderData->GameVAOID);
        glDrawElements(GL_TRIANGLES, RenderData->DrawFrame.QuadCount * 6, GL_UNSIGNED_INT, 0);
    }
}

