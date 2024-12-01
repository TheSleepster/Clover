#include "../data/deps/Freetype/include/ft2build.h"
#include FT_FREETYPE_H

#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#include "../data/deps/stb/stb_image.h"
#endif

// INTRINSICS
#include "Intrinsics.h"

// UTILS
#include "util/Math.h"
#include "util/Array.h"
#include "util/FileIO.h"
#include "util/Arena.h"
#include "util/String.h"

#include "Clover.h"
#include "Clover_Renderer.h"

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

void
CloverTestShader(GLuint TestID, GLuint Type)
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

char *
CloverLoadSDFFontData(transient_state *TransientState, font_data *NewFont, string Filepath, uint32 FontSize)
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
    Error = FT_Set_Pixel_Sizes(Font.FontFace, 0, Font.FontSize);
    Check(Error == 0, "Issue setting the pixel size of the font\n");
    
    Font.AtlasPadding = 8;
    int32 Row = {};
    int32 Column = Font.AtlasPadding;
    
    FT_GlyphSlot CurrentSlot = Font.FontFace->glyph;
    
    // TODO(Sleepster): See if pushing onto garbage is a bad idea 
    char *TextureData = (char *)PushSize(&TransientState->Garbage, (uint64)(sizeof(char) * (BITMAP_ATLAS_SIZE * BITMAP_ATLAS_SIZE)));
    if(TextureData)
    {
        for(uint32 GlyphIndex = 32;
            GlyphIndex < 127;
            ++GlyphIndex)
        {
            FT_Load_Char(Font.FontFace, GlyphIndex, FT_LOAD_DEFAULT);
            if(Column + Font.FontFace->glyph->bitmap.width + Font.AtlasPadding >= BITMAP_ATLAS_SIZE)
            {
                Column = Font.AtlasPadding;
                Row += int32(Font.FontSize * 1.20);
            }
            
            Error = FT_Render_Glyph(CurrentSlot, FT_RENDER_MODE_SDF);
            Check(Error == 0, "Issues here\n");
            
            NewFont->FontHeight = MAX((Font.FontFace->size->metrics.ascender - Font.FontFace->size->metrics.descender) >> 6, 
                                       NewFont->Glyphs[GlyphIndex].GlyphSize.Y);
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
            
            font_glyph *CurrentGlyph = &NewFont->Glyphs[GlyphIndex];
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
    }
    
    FT_Done_Face(Font.FontFace);
    FT_Done_FreeType(Font.FontFile);

    return(TextureData);
}

// TODO(Sleepster): Revisit this to fix the alignment issues with letters like "p" "g" "l" "y" and such
void
CloverLoadSDFFont(transient_state *TransientState, font_data *NewFont, string Filepath, uint32 FontSize)
{
    char *TextureData = CloverLoadSDFFontData(TransientState, NewFont, Filepath, FontSize);
    CloverCreateSDFTexture(TransientState, &NewFont->FontAtlas, TextureData);
}

gl_shader_source
CloverLoadShaderSource(memory_arena *Scratch, uint32 ShaderType, string Filepath)
{
    uint32 FileSize = 0;
    gl_shader_source ReturnShader = {};
    
    string ShaderSource = ReadEntireFileMA(Scratch, Filepath, &FileSize);
    ReturnShader.Filepath = Filepath;
    
    if(ShaderSource.Data)
    {
        const char *ShaderSourceChar = (const char *)ShaderSource.Data;

        ReturnShader.SourceID = glCreateShader(ShaderType);
        glShaderSource(ReturnShader.SourceID, 1, &ShaderSourceChar, 0);
        glCompileShader(ReturnShader.SourceID);
        CloverTestShader(ReturnShader.SourceID, GL_VERTEX_SHADER);
    }
    else
    {
        Trace("File is either not found or does not contain strings!\n");
        Assert(false);
    }
    return(ReturnShader);
}

shader
CloverCreateShader(memory_arena *Memory, string VertexShader, string FragmentShader)
{
    glUseProgram(0);
    shader ReturnShader = {};
    
    ReturnShader.VertexShader   = CloverLoadShaderSource(Memory, GL_VERTEX_SHADER, VertexShader);
    ReturnShader.FragmentShader = CloverLoadShaderSource(Memory, GL_FRAGMENT_SHADER, FragmentShader);
    
    ReturnShader.VertexShader.LastWriteTime   = FileGetLastWriteTime(VertexShader);
    ReturnShader.FragmentShader.LastWriteTime = FileGetLastWriteTime(FragmentShader);
    
    ReturnShader.ShaderID = glCreateProgram();
    glAttachShader(ReturnShader.ShaderID, ReturnShader.VertexShader.SourceID);
    glAttachShader(ReturnShader.ShaderID, ReturnShader.FragmentShader.SourceID);
    glLinkProgram(ReturnShader.ShaderID);
    
    CloverTestShader(ReturnShader.ShaderID, GL_PROGRAM);
    
    glDetachShader(ReturnShader.ShaderID, ReturnShader.VertexShader.SourceID);
    glDetachShader(ReturnShader.ShaderID, ReturnShader.FragmentShader.SourceID);
    glDeleteShader(ReturnShader.VertexShader.SourceID);
    glDeleteShader(ReturnShader.FragmentShader.SourceID);
    
    return(ReturnShader);
}

void
CloverLoadTexture(transient_state *TransientState, texture2d *TextureInfo, string Filepath)
{
    uint32 TextureCount = TransientState->GameAssets->TextureCount;
    glActiveTexture(GL_TEXTURE0 + TextureCount);
    InterlockedIncrement(&TransientState->GameAssets->TextureCount);
    
    glGenTextures(1, &TextureInfo->TextureID);
    glBindTexture(GL_TEXTURE_2D, TextureInfo->TextureID);
    
    TextureInfo->Filepath = Filepath;
    TextureInfo->LastWriteTime = FileGetLastWriteTime(Filepath);
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
        
        glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB_ALPHA, 
                     TextureInfo->TextureData.Width, TextureInfo->TextureData.Height, 0, 
                     GL_RGBA, GL_UNSIGNED_BYTE, TextureInfo->RawData);
    }
    stbi_image_free(TextureInfo->RawData);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void
CloverReloadTexture(texture2d *TextureInfo, uint32 TextureIndex)
{
    glBindTexture(GL_TEXTURE_2D, TextureInfo->TextureID);
    
    TextureInfo->LastWriteTime = FileGetLastWriteTime(TextureInfo->Filepath);
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
        
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 
                     TextureInfo->TextureData.Width, TextureInfo->TextureData.Height, 0, 
                     GL_RGBA, GL_UNSIGNED_BYTE, TextureInfo->RawData);
    }
    stbi_image_free(TextureInfo->RawData);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void
CloverCreateSDFTexture(transient_state *TransientState, texture2d *TextureInfo, char *TextureData)
{
    TransientState->GameAssets->TextureCount++;
    glActiveTexture(GL_TEXTURE0 + TransientState->GameAssets->TextureCount);
    glGenTextures(1, &TextureInfo->TextureID);
    glBindTexture(GL_TEXTURE_2D, TextureInfo->TextureID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, BITMAP_ATLAS_SIZE, BITMAP_ATLAS_SIZE, 0, GL_RED, GL_UNSIGNED_BYTE, TextureData);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 3);

    glBindTexture(GL_TEXTURE_2D, 0);
}

void
CloverResetRendererState(gl_render_data *RenderData, transient_state *TransientState)
{

    RenderData->DrawFrameData.LastFrameQuadCount = RenderData->DrawFrameData.QuadCounter;

    RenderData->DrawFrameData.VertexBufferptr            = &RenderData->DrawFrameData.Vertices[0];
    RenderData->DrawFrameData.TransparentVertexBufferptr = &RenderData->DrawFrameData.Vertices[int32(MAX_VERTICES * 0.5f)];
    RenderData->DrawFrameData.OpaqueQuadCount = 0;
    RenderData->DrawFrameData.TransparentQuadCount = 0;
    RenderData->DrawFrameData.TotalQuadCount = 0;
    
    RenderData->DrawFrameData.UIVertexBufferptr            = &RenderData->DrawFrameData.UIVertices[0];
    RenderData->DrawFrameData.TransparentUIVertexBufferptr = &RenderData->DrawFrameData.UIVertices[int32(MAX_VERTICES * 0.5f)];
    RenderData->DrawFrameData.OpaqueUIElementCount = 0;
    RenderData->DrawFrameData.TransparentUIElementCount = 0;
    RenderData->DrawFrameData.TotalUIElementCount = 0;

    RenderData->DrawFrameData.PointLightCount = 0;
    RenderData->DrawFrameData.SpotLightCount = 0;

    RenderData->DrawFrameData.QuadCounter = 0;
}

internal int32
CompareVertexYAxis(const void *A, const void *B)
{
    const vertex *VertexA = (vertex *)A;
    const vertex *VertexB = (vertex *)B;
    
    return((VertexA->Position.Y > VertexB->Position.Y) ? -1 :
           (VertexA->Position.Y < VertexB->Position.Y) ?  1 : 0);
}

internal void
RebuildShader(memory_arena *Memory, shader *ReloadingShader)
{
    filetime NewVertexShaderWriteTime   = FileGetLastWriteTime(ReloadingShader->VertexShader.Filepath);
    filetime NewFragmentShaderWriteTime = FileGetLastWriteTime(ReloadingShader->FragmentShader.Filepath);
    if(!CloverCompareFiletime(NewVertexShaderWriteTime,   ReloadingShader->VertexShader.LastWriteTime)||
       !CloverCompareFiletime(NewFragmentShaderWriteTime, ReloadingShader->FragmentShader.LastWriteTime))
    {
        glDeleteProgram(ReloadingShader->ShaderID);
        *ReloadingShader = CloverCreateShader(Memory, 
                                              ReloadingShader->VertexShader.Filepath, 
                                              ReloadingShader->FragmentShader.Filepath);
        Sleep(100);
    }
}

// Poll for textures to upload to opengl
internal void
LoadQueuedOpenGLTextures(game_assets *Assets)
{   
    for(uint32 Index = 0;
        Index < GT_TextureIDCount;
        ++Index)
    {
        asset_slot *TextureInfo = &Assets->Textures[Index];
        if(TextureInfo->Texture && TextureInfo->SlotState == AssetState_Queued)
        {
            if(TextureInfo->Texture->RawData)
            {
                int32 TextureCount = Assets->TextureCount;
                glActiveTexture(GL_TEXTURE0 + TextureCount);
                InterlockedIncrement(&Assets->TextureCount);

                glGenTextures(1, &TextureInfo->Texture->TextureID);
                glBindTexture(GL_TEXTURE_2D, TextureInfo->Texture->TextureID);

                TextureInfo->Texture->LastWriteTime = FileGetLastWriteTime(TextureInfo->Texture->Filepath);
                TextureInfo->Texture->RawData = (char *)stbi_load((const char *)TextureInfo->Texture->Filepath.Data, 
                        &TextureInfo->Texture->TextureData.Width, 
                        &TextureInfo->Texture->TextureData.Height, 
                        &TextureInfo->Texture->TextureData.Channels, 
                        4);
                if(TextureInfo->Texture->RawData)
                {
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

                    glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB_ALPHA, 
                            TextureInfo->Texture->TextureData.Width, TextureInfo->Texture->TextureData.Height, 0, 
                            GL_RGBA, GL_UNSIGNED_BYTE, TextureInfo->Texture->RawData);
                }
                stbi_image_free(TextureInfo->Texture->RawData);
                glBindTexture(GL_TEXTURE_2D, 0);
                ++Assets->TextureCount;

                InterlockedCompareExchange((uint64 volatile *)&TextureInfo->SlotState,
                                           AssetState_Loaded,
                                           AssetState_Queued);
            }
        }
    }
    
    for(uint32 Index = 0;
        Index < GF_FontIDCount;
        ++Index)
    {
        asset_slot *NewFont = &Assets->Fonts[Index];
        if(NewFont->Font && NewFont->Font->RawData)
        {
            CloverCreateSDFTexture(Assets->TransientState, &NewFont->Font->FontAtlas, NewFont->Font->RawData);
            NewFont->Font->RawData = 0;
            ++Assets->TextureCount;
        }
    }
}

internal void
CloverSetupRenderer(game_memory *GameMemory, gl_render_data *RenderData, transient_state *TransientState)
{
    // STATE INITIALIZATION
    {
        glDebugMessageCallback(&OpenGLDebugMessageCallback, nullptr);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        glEnable(GL_DEBUG_OUTPUT);
        
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_GREATER);
        
        glEnable(GL_BLEND);        
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glBlendEquation(GL_FUNC_ADD);

        glEnable(GL_FRAMEBUFFER_SRGB);
        glDisable(0x809D); // Disabling multisampling
        
        RenderData->ClearColor = DARK_GRAY;
        TransientState->GameAssets->TextureCount = 0;

        RenderData->DrawFrameData.EnableZLayering = true;
        RenderData->DrawFrameData.EnableZSorting;
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
        
        // NOTE(Sleepster): Try 4 on position??? 
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vertex), (void *)offsetof(vertex, Position));
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(vertex), (void *)offsetof(vertex, TextureCoords));
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(vertex), (void *)offsetof(vertex, VertexNormals));
        glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(vertex), (void *)offsetof(vertex, DrawColor));
        glVertexAttribPointer(4, 1, GL_FLOAT, GL_FALSE, sizeof(vertex), (void *)offsetof(vertex, TextureIndex));
        
        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);
        glEnableVertexAttribArray(2);
        glEnableVertexAttribArray(3);
        glEnableVertexAttribArray(4);
    }
    
    // GAME UI BUFFER SETUP
    {
        glGenVertexArrays(1, &RenderData->GameUIVAOID);
        glBindVertexArray(RenderData->GameUIVAOID);
        
        glGenBuffers(1, &RenderData->GameUIVBOID);
        glBindBuffer(GL_ARRAY_BUFFER, RenderData->GameUIVBOID);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertex) * MAX_VERTICES, 0, GL_DYNAMIC_DRAW);
        
        glGenBuffers(1, &RenderData->GameUIEBOID);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, RenderData->GameUIEBOID);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Indices), Indices, GL_STATIC_DRAW);
        
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vertex), (void *)offsetof(vertex, Position));
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(vertex), (void *)offsetof(vertex, TextureCoords));
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(vertex), (void *)offsetof(vertex, VertexNormals));
        glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(vertex), (void *)offsetof(vertex, DrawColor));
        glVertexAttribPointer(4, 1, GL_FLOAT, GL_FALSE, sizeof(vertex), (void *)offsetof(vertex, TextureIndex));
        
        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);
        glEnableVertexAttribArray(2);
        glEnableVertexAttribArray(3);
        glEnableVertexAttribArray(4);
    }

    // GBUFFER FRAMEBUFFER
    {
        glCreateFramebuffers(1, &RenderData->gBuffer); 
        glBindFramebuffer(GL_FRAMEBUFFER, RenderData->gBuffer);

        glGenTextures(3, RenderData->gBufferTextures);
        glBindTexture(GL_TEXTURE_2D, RenderData->gBufferTextures[0]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB_ALPHA, SizeData.Width, SizeData.Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        glBindTexture(GL_TEXTURE_2D, RenderData->gBufferTextures[1]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, SizeData.Width, SizeData.Height, 0, GL_RGB, GL_FLOAT, 0);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        glBindTexture(GL_TEXTURE_2D, RenderData->gBufferTextures[2]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, SizeData.Width, SizeData.Height, 0, GL_RGB, GL_FLOAT, 0);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, RenderData->gBufferTextures[0], 0);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, RenderData->gBufferTextures[1], 0);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, RenderData->gBufferTextures[2], 0);

        GLenum ColorAttachments[3] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2};
        glDrawBuffers(3, ColorAttachments);

        glGenRenderbuffers(1, &RenderData->gBufferDepthRBID);
        glBindRenderbuffer(GL_RENDERBUFFER, RenderData->gBufferDepthRBID);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, SizeData.Width, SizeData.Height);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, RenderData->gBufferDepthRBID);

        if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        {
            Check(0, "Framebuffer Failure\n");
        }
    }

    // SHADER SETUP
    {
        // NOTE(Sleepster): This is for the common shader includes 
        uint32 Size = {};
        string CommonGLHeader = ReadEntireFileMA(&TransientState->Garbage, STR("../code/shader/CommonShader.glh"), &Size);
        glNamedStringARB(GL_SHADER_INCLUDE_ARB, -1, "/../code/shader/CommonShader.glh", int32(CommonGLHeader.Length), CSTR(CommonGLHeader));

        TransientState->GameAssets->Shaders[GS_BasicShader] = 
            CloverCreateShader(&TransientState->Garbage, STR("../code/shader/Basic.vert"), STR("../code/shader/Basic.frag"));
        TransientState->GameAssets->Shaders[GS_BasicShader].ShaderState = AssetState_Loaded;

        TransientState->GameAssets->Shaders[GS_GBufferShader] = 
            CloverCreateShader(&TransientState->Garbage, STR("../code/shader/gBuffer_Geo.vert"), STR("../code/shader/gBuffer_Geo.frag"));
        TransientState->GameAssets->Shaders[GS_GBufferShader].ShaderState = AssetState_Loaded; 

        TransientState->GameAssets->Shaders[GS_LightingShader] = 
            CloverCreateShader(&TransientState->Garbage, STR("../code/shader/gBuffer_lighting.vert"), STR("../code/shader/gBuffer_lighting.frag"));
        TransientState->GameAssets->Shaders[GS_LightingShader].ShaderState = AssetState_Loaded;
    }

    // SHADER UNIFORM / STORAGE BUFFER SETUP
    {
        RenderData->GameCamera.ViewMatrix            = mat4Identity(1.0f);
        RenderData->GameUICamera.ViewMatrix          = mat4Identity(1.0f);

        RenderData->ProjectionMatrixUID              = glGetUniformLocation(TransientState->GameAssets->Shaders[GS_BasicShader].ShaderID,    "ProjectionMatrix");
        RenderData->ViewMatrixUID                    = glGetUniformLocation(TransientState->GameAssets->Shaders[GS_BasicShader].ShaderID,    "ViewMatrix");
        RenderData->BasicShaderBrightnessUID         = glGetUniformLocation(TransientState->GameAssets->Shaders[GS_BasicShader].ShaderID,    "uBrightness");

        RenderData->gBufferProjectionMatrixUID       = glGetUniformLocation(TransientState->GameAssets->Shaders[GS_GBufferShader].ShaderID,  "ProjectionMatrix");
        RenderData->gBufferViewMatrixUID             = glGetUniformLocation(TransientState->GameAssets->Shaders[GS_GBufferShader].ShaderID,  "ViewMatrix");
        RenderData->gBufferBrightnessUID             = glGetUniformLocation(TransientState->GameAssets->Shaders[GS_GBufferShader].ShaderID,  "uBrightness");
 
        RenderData->PointLightSBOID                  = glGetUniformLocation(TransientState->GameAssets->Shaders[GS_LightingShader].ShaderID, "gBufferPointLightSBO");
        RenderData->PointLightCountUID               = glGetUniformLocation(TransientState->GameAssets->Shaders[GS_LightingShader].ShaderID, "uPointLightCount");
        RenderData->LightingShaderUserBrightnessUID  = glGetUniformLocation(TransientState->GameAssets->Shaders[GS_LightingShader].ShaderID, "uBrightnessFactor");
        RenderData->LightingShaderWorldBrightnessUID = glGetUniformLocation(TransientState->GameAssets->Shaders[GS_LightingShader].ShaderID, "uWorldBrightness");

        // NOTE(Sleepster): Point Light Shader Buffer
        uint64 MaxBufferSize = sizeof(struct point_light) * MAX_POINT_LIGHTS;
        glGenBuffers(1, &RenderData->PointLightSBOID);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, RenderData->PointLightSBOID);
        glBufferData(GL_SHADER_STORAGE_BUFFER, MaxBufferSize, 0, GL_DYNAMIC_DRAW);

        // NOTE(Sleepster): Spot Light Shader Buffer
        MaxBufferSize = sizeof(struct spot_light) * MAX_SPOT_LIGHTS;
        glGenBuffers(1, &RenderData->SpotLightSBOID);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, RenderData->SpotLightSBOID);
        glBufferData(GL_SHADER_STORAGE_BUFFER, MaxBufferSize, 0, GL_DYNAMIC_DRAW);

        glBindTexture(GL_TEXTURE_2D, 0);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    }

    glGenQueries(1, &RenderData->StartTimer);
    glGenQueries(1, &RenderData->EndTimer);
}

internal void
DrawImGui(game_state *State, gl_render_data *RenderData, time_data Time)
{
    if(State->DrawDebug)
    {
        ImGui::SetCurrentContext(RenderData->CurrentImGuiContext);

        ImGui::Begin("Render Quad Color Picker");
        ImGui::SeparatorText("ENGINE DEBUG INFO");
        ImGui::Text("Famerate: %i", Time.FPSCounter);
        ImGui::Text("FrameTime: %.02f", Time.MSPerFrame);

        ImGui::SeparatorText("GAME DEBUG INFO");
        ImGui::Text("Entity Count: %i", State->World.EntityCounter);
        ImGui::Text("Quad Count: %i", RenderData->DrawFrameData.QuadCounter);
        ImGui::Text("Vertex Count: %i", RenderData->DrawFrameData.QuadCounter * 4);
        ImGui::Text("GPU Time: %f", RenderData->GPUTimeInMS);
        ImGui::End();
    }
}

internal
CLOVER_OGL_RENDER(CloverRender)
{
    glQueryCounter(RenderData->StartTimer, GL_TIMESTAMP);
    texture2d *CurrentGameAtlasTexture = GetTextureFromID(GameMemory, GT_GameAtlas);
    shader    *BasicShader             = GetShaderFromID(GameMemory, GS_BasicShader);
    shader    *GBufferShader           = GetShaderFromID(GameMemory, GS_GBufferShader);
    shader    *LightingShader          = GetShaderFromID(GameMemory, GS_LightingShader);
    font_data *CurrentBoundFont        = GetFontFromID(GameMemory, 48, GF_UbuntuMono);


    // OPAQUE GAME OBJECT RENDERING PASS
    if(RenderData->DrawFrameData.OpaqueQuadCount > 0)
    {
        // GBUFFER RENDERING
        {
            glBindFramebuffer(GL_FRAMEBUFFER, RenderData->gBuffer);
            glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    		glUseProgram(GBufferShader->ShaderID);

            glEnable(GL_DEPTH_TEST);
            glDepthFunc(GL_LEQUAL);
            
            glEnable(GL_FRAMEBUFFER_SRGB);
            glDisable(GL_BLEND);        
            glDisable(0x809D); // Disabling multisampling

            glBindBuffer(GL_ARRAY_BUFFER, RenderData->GameVBOID);
            glBufferSubData(GL_ARRAY_BUFFER, 
                            0, 
                            (RenderData->DrawFrameData.OpaqueQuadCount * 4) * sizeof(vertex), 
                             RenderData->DrawFrameData.Vertices);

            glUniformMatrix4fv(RenderData->gBufferProjectionMatrixUID, 1, GL_FALSE, &RenderData->GameCamera.ProjectionMatrix.Elements[0][0]);
            glUniformMatrix4fv(RenderData->gBufferViewMatrixUID, 1, GL_FALSE, &RenderData->GameCamera.ViewMatrix.Elements[0][0]);
            glUniform1f(RenderData->gBufferBrightnessUID, RenderBrightness);

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, CurrentGameAtlasTexture->TextureID);

            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, CurrentBoundFont->FontAtlas.TextureID);

            glBindVertexArray(RenderData->GameVAOID);
            glDrawElements(GL_TRIANGLES, 
                        RenderData->DrawFrameData.OpaqueQuadCount * 6, 
                        GL_UNSIGNED_INT, 
                        0); 
        }

        // DEFERRED RENDERER LIGHTING
        {
            glBindFramebuffer(GL_FRAMEBUFFER, 0);

            glDisable(GL_DEPTH_TEST);
            glEnable(GL_FRAMEBUFFER_SRGB);

            glEnable(GL_BLEND);        
            glBlendEquation(GL_FUNC_ADD);
            glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

            glDisable(0x809D); // Disabling multisampling
            glUseProgram(LightingShader->ShaderID);

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, RenderData->gBufferTextures[0]);

            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, RenderData->gBufferTextures[1]);

            glActiveTexture(GL_TEXTURE2);
            glBindTexture(GL_TEXTURE_2D, RenderData->gBufferTextures[2]);

            glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, RenderData->PointLightSBOID);
            glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(point_light) * RenderData->DrawFrameData.PointLightCount, RenderData->DrawFrameData.PointLights);

            glUniform1i(RenderData->PointLightCountUID, RenderData->DrawFrameData.PointLightCount);
            glUniform1f(RenderData->LightingShaderUserBrightnessUID, RenderBrightness);
            glUniform1f(RenderData->LightingShaderWorldBrightnessUID, CurrentWorldBrightness);

            glDrawArrays(GL_TRIANGLES, 0, 6);
        }

        glBindFramebuffer(GL_READ_FRAMEBUFFER, RenderData->gBuffer);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
        glBlitFramebuffer(0, 0, SizeData.Width, SizeData.Height, 0, 0, SizeData.Width, SizeData.Height, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    glUseProgram(BasicShader->ShaderID);
    if(RenderData->DrawFrameData.TransparentQuadCount > 0)
    {
        GLintptr BufferOffset   =  (RenderData->DrawFrameData.OpaqueQuadCount * 4) * sizeof(vertex);
        GLintptr ElementOffset  =  (RenderData->DrawFrameData.OpaqueQuadCount * 6) * sizeof(uint32); 
        // TRANSPARENT GAME OBJECT RENDERERING PASS
        {
            glDisable(GL_DEPTH_TEST);

            glEnable(GL_BLEND);        
            glBlendEquation(GL_FUNC_ADD);
            glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

            glBufferSubData(GL_ARRAY_BUFFER, 
                            BufferOffset, 
                            (RenderData->DrawFrameData.TransparentQuadCount * 4) * sizeof(vertex), 
                            &RenderData->DrawFrameData.Vertices[int32(MAX_VERTICES * 0.5f)]);

            glUniformMatrix4fv(RenderData->ProjectionMatrixUID, 1, GL_FALSE, &RenderData->GameCamera.ProjectionMatrix.Elements[0][0]);
            glUniformMatrix4fv(RenderData->ViewMatrixUID, 1, GL_FALSE, &RenderData->GameCamera.ViewMatrix.Elements[0][0]);
            glUniform1f(RenderData->BasicShaderBrightnessUID, RenderBrightness);

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, CurrentGameAtlasTexture->TextureID);

            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, CurrentBoundFont->FontAtlas.TextureID);

            glBindVertexArray(RenderData->GameVAOID);
            glDrawElements(GL_TRIANGLES, 
                        RenderData->DrawFrameData.TransparentQuadCount * 6, 
                        GL_UNSIGNED_INT, 
                        (void*)ElementOffset);
        }
    }
    
    // TODO(Sleepster): Make Transparent Objects affected by the uBrightnessFactor of the game world 
    if(RenderData->DrawFrameData.TransparentUIElementCount > 0)
    {
        GLintptr UIBufferOffset   = int32((RenderData->DrawFrameData.OpaqueUIElementCount * 4) * sizeof(vertex));
        GLintptr UIElementOffset  = (RenderData->DrawFrameData.OpaqueUIElementCount * 6) * sizeof(uint32); 
        // TRANSPARENT UI RENDERING PASS
        {
            glDisable(GL_DEPTH_TEST);
            glEnable(GL_BLEND);        
            glBlendEquation(GL_FUNC_ADD);
            glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

            glBindBuffer(GL_ARRAY_BUFFER, RenderData->GameUIVBOID);
            glBufferSubData(GL_ARRAY_BUFFER, 
                            UIBufferOffset, 
                            (RenderData->DrawFrameData.TransparentUIElementCount * 4) * sizeof(vertex), 
                            &RenderData->DrawFrameData.UIVertices[int32(MAX_VERTICES * 0.5f)]);

            glUniformMatrix4fv(RenderData->ProjectionMatrixUID, 1, GL_FALSE, &RenderData->GameUICamera.ProjectionMatrix.Elements[0][0]);
            glUniformMatrix4fv(RenderData->ViewMatrixUID, 1, GL_FALSE, &RenderData->GameUICamera.ViewMatrix.Elements[0][0]);
            glUniform1f(RenderData->BasicShaderBrightnessUID, RenderBrightness);

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, CurrentGameAtlasTexture->TextureID);

            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, CurrentBoundFont->FontAtlas.TextureID);

            glBindVertexArray(RenderData->GameUIVAOID);
            glDrawElements(GL_TRIANGLES, 
                        RenderData->DrawFrameData.TransparentUIElementCount * 6, 
                        GL_UNSIGNED_INT, 
                        (void *)UIElementOffset); 
        }
    }
    
    if(RenderData->DrawFrameData.OpaqueUIElementCount > 0)
    {
        // OPAQUE UI RENDERING PASS
        {
            glDisable(GL_DEPTH_TEST);

            glBindBuffer(GL_ARRAY_BUFFER, RenderData->GameUIVBOID);
            glBufferSubData(GL_ARRAY_BUFFER, 
                            0, 
                            (RenderData->DrawFrameData.OpaqueUIElementCount * 4) * sizeof(vertex), 
                            RenderData->DrawFrameData.UIVertices);

            glUniformMatrix4fv(RenderData->ProjectionMatrixUID, 1, GL_FALSE, &RenderData->GameUICamera.ProjectionMatrix.Elements[0][0]);
            glUniformMatrix4fv(RenderData->ViewMatrixUID, 1, GL_FALSE, &RenderData->GameUICamera.ViewMatrix.Elements[0][0]);
            glUniform1f(RenderData->BasicShaderBrightnessUID, RenderBrightness);

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, CurrentGameAtlasTexture->TextureID);

            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, CurrentBoundFont->FontAtlas.TextureID);

            glBindVertexArray(RenderData->GameUIVAOID);
            glDrawElements(GL_TRIANGLES, 
                        RenderData->DrawFrameData.OpaqueUIElementCount * 6, 
                        GL_UNSIGNED_INT, 
                        0); 
        }
    }

    glQueryCounter(RenderData->EndTimer, GL_TIMESTAMP);
    RenderData->GPUTimeInMS = (RenderData->EndTimer - RenderData->StartTimer);
}
