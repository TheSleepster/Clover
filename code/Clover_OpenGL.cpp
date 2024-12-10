/* ========================================================================
   $File: Clover_OpenGL.cpp $
   $Date: Mon, 25 Nov 24: 08:PM $
   $Revision: $
   $Creator: Justin Lewis $
   ======================================================================== */
#include "Clover_Rendering.h"
#include "shader/CommonShader.glh"

constexpr uint32 IndirectDrawCommandBufferSize   = (MAX_LAYERS * 2) * sizeof(draw_elements_indirect_command);

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
        case GL_COMPUTE_SHADER:
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

#include "Clover_Shader.cpp"

void 
CloverLoadTextureData(gl_render_info *RenderInfo, clover_texture *Texture, string Filepath)
{
    if(Texture)
    {
        glGenTextures(1, &Texture->TextureID);
        glBindTexture(GL_TEXTURE_2D, Texture->TextureID);

        Texture->Filepath = Filepath;
        Texture->LastWriteTime = FileGetLastWriteTime(Filepath);
        Texture->RawData = (char *)stbi_load(CSTR(Filepath),
                                             &Texture->TextureChannelData.Width,
                                             &Texture->TextureChannelData.Height,
                                             &Texture->TextureChannelData.Channels,
                                             4);
        if(Texture->RawData)
        {
            return;
        }
        cl_Error("Could not extract image data. Image may be invalid.\n");
        glBindTexture(GL_TEXTURE_2D, 0);
        stbi_image_free(Texture->RawData);
    }
    else
    {
        cl_Error("Texture memory Invalid\n");
    }
}

// NOTE)Sleepster): This creates and Loads an SDF Font 
string
CloverLoadFontData(memory_arena *Arena, clover_font_data *FontData, string Filepath, uint32 FontSize)
{
    FT_Library FontFile;
    FT_Face    FontFace;
    FT_Error   Error;

    uint32     AtlasPadding;
    string     TTFFontData;

    FontData->SizeOnLoad = FontSize;
    Error = FT_Init_FreeType(&FontFile);
    if(!Error)
    {
        Error = FT_New_Face(FontFile, CSTR(Filepath), 0, &FontFace);
        if(!Error)
        {
            Error = FT_Set_Pixel_Sizes(FontFace, 0, FontSize);
            if(!Error)
            {
                AtlasPadding = 20;
                int32 CurrentAtlasRow = 0;
                int32 CurrentAtlasColumn = 0;

                FT_GlyphSlot GlyphSlotToWrite = FontFace->glyph;

                uint32 FileSize;
                TTFFontData = ReadEntireFileMA(Arena, Filepath, &FileSize);
                if(TTFFontData.Length > 0)
                {
                    for(uint32 GlyphIndex = 32;
                        GlyphIndex < 127;
                        ++GlyphIndex)
                    {
                        FT_Load_Char(FontFace, GlyphIndex, FT_LOAD_DEFAULT);
                        if(CurrentAtlasColumn + FontFace->glyph->bitmap.width + AtlasPadding >= CLOVER_FONT_ATLAS_SIZE)
                        {
                            CurrentAtlasColumn  = AtlasPadding;
                            CurrentAtlasRow    += int32(FontSize * 1.50);
                        }

                        Error = FT_Render_Glyph(GlyphSlotToWrite, FT_RENDER_MODE_SDF);
                        Check(Error == 0, "Issue Loading the glyph\n");

                        FontData->FontHeight = MAX((FontFace->size->metrics.ascender - FontFace->size->metrics.descender) >> 6,
                                                   FontData->Glyphs[GlyphIndex].Size.Y);

                        for(uint32 YIndex = 0;
                            YIndex < FontFace->glyph->bitmap.rows;
                            ++YIndex)
                        {
                            for(uint32 XIndex = 0;
                                XIndex < FontFace->glyph->bitmap.width;
                                ++XIndex)
                            {
                                TTFFontData.Data[(CurrentAtlasRow + YIndex) * CLOVER_FONT_ATLAS_SIZE + (CurrentAtlasColumn + XIndex)] = 
                                    FontFace->glyph->bitmap.buffer[YIndex * FontFace->glyph->bitmap.width + XIndex];
                            }
                        }

                        clover_font_glyph *CurrentGlyph = &FontData->Glyphs[GlyphIndex];
                        CurrentGlyph->UVs  = {CurrentAtlasColumn, CurrentAtlasRow};
                        CurrentGlyph->Size = 
                            {
                                (int32)FontFace->glyph->bitmap.width, 
                                (int32)FontFace->glyph->bitmap.rows
                            };
                        CurrentGlyph->Advance = 
                            {
                                real32(FontFace->glyph->advance.x >> 6), 
                                real32(FontFace->glyph->advance.y >> 6)
                            };
                        CurrentGlyph->Offset = 
                            {
                                real32(FontFace->glyph->bitmap_left),
                                real32(FontFace->glyph->bitmap_top)
                            };
            
                        CurrentAtlasColumn += FontFace->glyph->bitmap.width + AtlasPadding;
                    }
                }
                else
                {
                    cl_Error("Failure to read the font file data.\n");
                }
            }
            else
            {
                cl_Error("Failure to set the Freetype Font Pixel Sizes. Error Code: %d\n", Error);
            }
        }
        else
        {
            cl_Error("Failure to create the Freetype font face! Error Code: %d\n", Error);
        }
        FT_Done_Face(FontFace);
        FT_Done_FreeType(FontFile);
    }
    else
    {
        cl_Error("Failure to init Freetype, error code: %d\n", Error);
    }
    return(TTFFontData);
}

internal void
CloverCreateSDFTexture(gl_draw_frame_data *DrawFrame, clover_texture *Texture, string TextureData)
{
    glActiveTexture(GL_TEXTURE0 + DrawFrame->WorkingTextureIndex++);
    glGenTextures(1, &Texture->TextureID);
    glBindTexture(GL_TEXTURE_2D, Texture->TextureID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, CLOVER_FONT_ATLAS_SIZE, CLOVER_FONT_ATLAS_SIZE, 0, GL_RED, GL_UNSIGNED_BYTE, CSTR(TextureData));

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 3);
    glGenerateMipmap(GL_TEXTURE_2D);

    glBindTexture(GL_TEXTURE_2D, 0);
}

internal void
CloverCreateLinear2DTexture(gl_draw_frame_data *DrawFrame, clover_texture *Texture)
{
    glActiveTexture(GL_TEXTURE0 + DrawFrame->WorkingTextureIndex++);
    glGenTextures(1, &Texture->TextureID);
    glBindTexture(GL_TEXTURE_2D, Texture->TextureID);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB_ALPHA, 
                 Texture->TextureChannelData.Width, Texture->TextureChannelData.Height, 0, 
                 GL_RGBA, GL_UNSIGNED_BYTE, Texture->RawData);

    stbi_image_free(Texture->RawData);
    glBindTexture(GL_TEXTURE_2D, 0);
}

internal void
CloverLoadInQueueOpenGLData(gl_draw_frame_data *DrawFrame, game_assets *Assets)
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
                CloverCreateLinear2DTexture(DrawFrame, TextureInfo->Texture);
                InterlockedCompareExchange((uint64 volatile *)&TextureInfo->SlotState,
                                           AssetState_Loaded,
                                           AssetState_Queued);
                ++Assets->TextureCount;
            }
        }
    }

    for(uint32 Index = 0;
        Index < GF_FontIDCount;
        ++Index)
    {
        asset_slot *NewFont = &Assets->Fonts[Index];
        if(NewFont->Font && (NewFont->Font->RawData != NULLSTR))
        {
            CloverCreateSDFTexture(DrawFrame, &NewFont->Font->FontAtlas, NewFont->Font->RawData);
            NewFont->Font->RawData = {};
            ++Assets->TextureCount;
        }
    }
}

internal void
CloverDrawIMGUI(game_state *GameState, gl_render_info *RenderInfo, gl_draw_frame_data *DrawFrame, time_data Time)
{
    if(GameState->DrawDebug)
    {
        ImGui::SetCurrentContext(RenderInfo->CurrentImGuiContext);

        ImGui::Begin("Render Quad Color Picker");
        ImGui::SeparatorText("ENGINE DEBUG INFO");
        ImGui::Text("Famerate: %i", Time.FPSCounter);
        ImGui::Text("FrameTime: %.02f", Time.MSPerFrame);

        ImGui::SeparatorText("GAME DEBUG INFO");
        ImGui::Text("Entity Count: %i", GameState->World.EntityCounter);
        ImGui::Text("Quad Count: %i", DrawFrame->QuadCounter);
        ImGui::Text("Vertex Count: %i", DrawFrame->QuadCounter * 4);
        ImGui::Text("GPU Time: %f", RenderInfo->GPUTimeInMS);
        ImGui::End();
    }
}

internal void
CloverInitializeOpenGLRenderer(game_memory *GameMemory, gl_render_info *RenderInfo, gl_draw_frame_data *DrawFrame, transient_state *TransientState)
{
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
    }

    // NOTE(Sleepster): PRIMARY BUFFER SETUP 
    {
        // PRIMARY VERTEX ARRAY
        glGenVertexArrays(1, &RenderInfo->PrimaryVAOID);
        glBindVertexArray(RenderInfo->PrimaryVAOID);

        // PRIMARY VERTEX BUFFER
        glGenBuffers(1, &RenderInfo->PrimaryVBOID);
        glBindBuffer(GL_ARRAY_BUFFER, RenderInfo->PrimaryVBOID);
        glBufferData(GL_ARRAY_BUFFER, sizeof(gl_vertex) * MAX_VERTICES, 0, GL_DYNAMIC_DRAW);
        
        // PRIMARY INDEX BUFFER
        glGenBuffers(1, &RenderInfo->PrimaryEBOID);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, RenderInfo->PrimaryEBOID);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Indices), Indices, GL_STATIC_DRAW);

        glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(gl_vertex), (void *)offsetof(gl_vertex, NDCPosition));
        glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(gl_vertex), (void *)offsetof(gl_vertex, WorldPosition));
        glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(gl_vertex), (void *)offsetof(gl_vertex, Color));
        glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(gl_vertex), (void *)offsetof(gl_vertex, TexCoords));
        glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(gl_vertex), (void *)offsetof(gl_vertex, Normals));

        glVertexAttribIPointer(5, 1, GL_INT,  sizeof(gl_vertex), (void *)offsetof(gl_vertex, TextureIndex));
        glVertexAttribIPointer(6, 1, GL_UNSIGNED_INT, sizeof(gl_vertex), (void *)offsetof(gl_vertex,  RenderingOptions));

        glVertexAttribPointer(7, 1, GL_FLOAT, GL_FALSE, sizeof(gl_vertex), (void *)offsetof(gl_vertex,  LitFactor));
        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);
        glEnableVertexAttribArray(2);
        glEnableVertexAttribArray(3);
        glEnableVertexAttribArray(4);
        glEnableVertexAttribArray(5);
        glEnableVertexAttribArray(6);
        glEnableVertexAttribArray(7);
    }

    // DEFERRED LIGHTING
    {
        glCreateFramebuffers(1, &RenderInfo->gBufferFBID); 
        glBindFramebuffer(GL_FRAMEBUFFER, RenderInfo->gBufferFBID);

        glGenTextures(3, RenderInfo->gBufferTextures);
        glBindTexture(GL_TEXTURE_2D, RenderInfo->gBufferTextures[0]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB_ALPHA, SizeData.Width, SizeData.Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        glBindTexture(GL_TEXTURE_2D, RenderInfo->gBufferTextures[1]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, SizeData.Width, SizeData.Height, 0, GL_RGB, GL_FLOAT, 0);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        glBindTexture(GL_TEXTURE_2D, RenderInfo->gBufferTextures[2]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, SizeData.Width, SizeData.Height, 0, GL_RGBA, GL_FLOAT, 0);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, RenderInfo->gBufferTextures[0], 0);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, RenderInfo->gBufferTextures[1], 0);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, RenderInfo->gBufferTextures[2], 0);

        GLenum ColorAttachments[3] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2};
        glDrawBuffers(3, ColorAttachments);

        glGenRenderbuffers(1, &RenderInfo->gBufferDepthRBID);
        glBindRenderbuffer(GL_RENDERBUFFER, RenderInfo->gBufferDepthRBID);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, SizeData.Width, SizeData.Height);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, RenderInfo->gBufferDepthRBID);
        glBindRenderbuffer(GL_RENDERBUFFER, 0);

        if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        {
            Check(0, "Framebuffer Failure\n");
        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    // DEBUG
    {
        glGenTextures(1, &RenderInfo->TestComputeTexture);
        glBindTexture(GL_TEXTURE_2D, RenderInfo->TestComputeTexture);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, SizeData.Width, SizeData.Height, 0, GL_RGBA, GL_FLOAT, 0);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        glBindTexture(GL_TEXTURE_2D, 0);
    }

    // DRAW COMMAND BUFFER
    {
        glGenBuffers(1, &RenderInfo->IndirectDrawingCommandBufferID);
        glBindBuffer(GL_DRAW_INDIRECT_BUFFER, RenderInfo->IndirectDrawingCommandBufferID);
        glBufferData(GL_DRAW_INDIRECT_BUFFER, (MAX_LAYERS * 2) * sizeof(draw_elements_indirect_command), 0, GL_DYNAMIC_DRAW);
    }

    // SHADERS
    {
        uint32 Size = {};
        string CommonGLHeader = ReadEntireFileMA(&TransientState->Garbage, STR("../code/shader/CommonShader.glh"), &Size);
        glNamedStringARB(GL_SHADER_INCLUDE_ARB, -1, "/../code/shader/CommonShader.glh", int32(CommonGLHeader.Length), CSTR(CommonGLHeader));

        TransientState->GameAssets->Shaders[GS_BasicShader].Shader
            = CloverLoadBasicPixelShader(TransientState,
                                         STR("../code/shader/new/Basic.vert"),
                                         STR("../code/shader/new/Basic.frag"));

        TransientState->GameAssets->Shaders[GS_gBufferShader].Shader
            = CloverLoadBasicPixelShader(TransientState,
                                         STR("../code/shader/new/gBuffer_Geometry.vert"),
                                         STR("../code/shader/new/gBuffer_Geometry.frag"));
        
        TransientState->GameAssets->Shaders[GS_LightingShader].Shader =
            CloverLoadBasicPixelShader(TransientState,
                                       STR("../code/shader/new/Quad.vert"),
                                       STR("../code/shader/new/gBuffer_Lighting.frag"));

        TransientState->GameAssets->Shaders[GS_TestComputeShader].Shader =
            CloverLoadComputeShader(TransientState,
                                     STR("../code/shader/new/RenderWeirdGradient.comp"));
    }

    clover_shader *gBufferLightingShader = GetShaderFromID(GameMemory, GS_LightingShader);

    // UNIFORMS
    {
        RenderInfo->gBufferPointLightSBOID    = glGetUniformLocation(gBufferLightingShader->ProgramID, "gBufferPointLightSBO");
        RenderInfo->uBrightnessFactorUID      = glGetUniformLocation(gBufferLightingShader->ProgramID, "uBrightnessFactor");
        RenderInfo->uWorldBrightnessFactorUID = glGetUniformLocation(gBufferLightingShader->ProgramID, "uWorldBrightness");
        RenderInfo->uPointLightCountUID       = glGetUniformLocation(gBufferLightingShader->ProgramID, "uPointLightCount");
    }

    // BUFFERS
    {
        uint64 MaxBufferSize = sizeof(struct point_light) * MAX_POINT_LIGHTS;
        glGenBuffers(1, &RenderInfo->gBufferPointLightSBOID);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, RenderInfo->gBufferPointLightSBOID);
        glBufferData(GL_SHADER_STORAGE_BUFFER, MaxBufferSize, 0, GL_DYNAMIC_DRAW);

        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    }
}

internal void
CloverResizeFramebuffers(gl_render_info *RenderInfo)
{
    if(RenderInfo->gBufferFBID != 0)
    {
        glDeleteFramebuffers(1, &RenderInfo->gBufferFBID);
    }
    if(RenderInfo->gBufferDepthRBID != 0)
    {
        glDeleteRenderbuffers(1, &RenderInfo->gBufferDepthRBID);
    }

    glDeleteTextures(3, RenderInfo->gBufferTextures);

    glCreateFramebuffers(1, &RenderInfo->gBufferFBID); 
    glBindFramebuffer(GL_FRAMEBUFFER, RenderInfo->gBufferFBID);

    glGenTextures(3, RenderInfo->gBufferTextures);
    glBindTexture(GL_TEXTURE_2D, RenderInfo->gBufferTextures[0]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB_ALPHA, SizeData.Width, SizeData.Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glBindTexture(GL_TEXTURE_2D, RenderInfo->gBufferTextures[1]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, SizeData.Width, SizeData.Height, 0, GL_RGB, GL_FLOAT, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glBindTexture(GL_TEXTURE_2D, RenderInfo->gBufferTextures[2]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, SizeData.Width, SizeData.Height, 0, GL_RGBA, GL_FLOAT, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, RenderInfo->gBufferTextures[0], 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, RenderInfo->gBufferTextures[1], 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, RenderInfo->gBufferTextures[2], 0);

    GLenum ColorAttachments[3] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2};
    glDrawBuffers(3, ColorAttachments);

    glGenRenderbuffers(1, &RenderInfo->gBufferDepthRBID);
    glBindRenderbuffer(GL_RENDERBUFFER, RenderInfo->gBufferDepthRBID);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, SizeData.Width, SizeData.Height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, RenderInfo->gBufferDepthRBID);

    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        Check(0, "Framebuffer Failure\n");
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
}

internal void
ResetLayerData(gl_draw_frame_data *DrawFrame)
{
    for(uint32 LayerIndex = 0;
        LayerIndex < MAX_LAYERS;
        ++LayerIndex)
    {
        DrawFrame->Layers[LayerIndex] = {};
    }
}

internal void
CloverOpenGLRender(game_memory *GameMemory, gl_render_info *RenderInfo, gl_draw_frame_data *DrawFrame, time_data Time)
{
    //clover_shader *BasicShader           = GetShaderFromID(GameMemory, GS_BasicShader); 
    clover_shader *gBufferGeoShader      = GetShaderFromID(GameMemory, GS_gBufferShader);
    clover_shader *gBufferLightingShader = GetShaderFromID(GameMemory, GS_LightingShader);
    // TODO(Sleepster): Change this ViewMatrix so that it zooms in when the screen gets larger  
#if 0
	DrawFrame->SceneCamera.ViewMatrix 		= mat4Scale(mat4Identity(1.0f), vec3{real32(SizeData.Width / 160), real32(SizeData.Width / 160), 1.0});
    DrawFrame->SceneCameraProjectionMatrix = mat4RHGLOrtho((real32)-SizeData.Width,
                                                           (real32)SizeData.Width,
                                                           (real32)-SizeData.Height,
                                                           (real32)SizeData.Height,
                                                           -1.0f,
                                                           1.0f);
#endif 
#if 1
    if(DrawFrame->QuadCounter > 0)
    {
        gl_vertex *VertexBufferptr = DrawFrame->glVertexBuffer;
        if(DrawFrame->ZSorting)
        {
            RadixSort((void *)DrawFrame->QuadBuffer,
                      (void *)DrawFrame->QuadSortingBuffer,
                      DrawFrame->QuadCounter,
                      sizeof(render_quad),
                      offsetof(render_quad, ZLayer),
                      MAX_LAYER_BITS);

            // NOTE(Sleepster): This is so that we know where the Transparent quads are in the buffer
            // So that we can just start at the opaque index and render them and not worry about it
            int32 QuadBufferLayerOffset = 0;
            for(uint32 LayerIndex = 0;
                LayerIndex < MAX_LAYERS;
                ++LayerIndex)
            {
                render_layer_info *CurrentWorkingLayer = &DrawFrame->Layers[LayerIndex];
                CurrentWorkingLayer->BaseIndex   = QuadBufferLayerOffset;
                     
                if(CurrentWorkingLayer->LayerOpaqueQuadCount > 0)
                {
                    CurrentWorkingLayer->EndingIndex = QuadBufferLayerOffset + (CurrentWorkingLayer->LayerOpaqueQuadCount); 
                    QuadBufferLayerOffset += CurrentWorkingLayer->LayerOpaqueQuadCount;
                }

                if(CurrentWorkingLayer->LayerTransparentQuadCount > 0)
                {
                    CurrentWorkingLayer->EndingIndex = QuadBufferLayerOffset + (CurrentWorkingLayer->LayerTransparentQuadCount - 1); 
                    QuadBufferLayerOffset += CurrentWorkingLayer->LayerTransparentQuadCount;
                }
            }

            // NOTE(Sleepster): Sort by transparency
            for(uint32 LayerIndex = 0;
                LayerIndex < MAX_LAYERS;
                ++LayerIndex)
            {
                render_layer_info *CurrentWorkingLayer = &DrawFrame->Layers[LayerIndex];
                if(CurrentWorkingLayer->LayerTransparentQuadCount)
                {
                    RadixSort((void *)(DrawFrame->QuadBuffer + (CurrentWorkingLayer->BaseIndex)),
                              (void *)DrawFrame->QuadSortingBuffer,
                              CurrentWorkingLayer->LayerTransparentQuadCount + CurrentWorkingLayer->LayerOpaqueQuadCount,
                              sizeof(render_quad),
                              offsetof(render_quad, SortingTransparency),
                              MAX_LAYER_BITS);
                }
            }

            // NOTE(Sleepster): Then we set up the indirect draw command buffer 
            DrawFrame->ActiveLayerCounter = 0;
            for(uint32 LayerIndex = 0;
                LayerIndex < MAX_LAYERS;
                ++LayerIndex)
            {
                render_layer_info *CurrentWorkingLayer = &DrawFrame->Layers[LayerIndex];
                if(CurrentWorkingLayer->LayerOpaqueQuadCount > 0)
                {
                    DrawFrame->IndirectRenderCommandBuffer[DrawFrame->ActiveLayerCounter] =
                        {
                            .IndexCount = uint32(CurrentWorkingLayer->LayerOpaqueQuadCount * 6),
                            .InstanceCount = 1,
                            .FirstIndex = uint32(CurrentWorkingLayer->BaseIndex * 6),
                            .FirstVertex = 0,
                            .BaseInstance = 0,
                        };
                }

                if(CurrentWorkingLayer->LayerTransparentQuadCount > 0)
                {
                    uint32 TransparentLayerIndex = int32(MAX_LAYERS) + DrawFrame->ActiveLayerCounter;
                    DrawFrame->IndirectRenderCommandBuffer[TransparentLayerIndex] =
                        {
                            .IndexCount = uint32(CurrentWorkingLayer->LayerTransparentQuadCount * 6),
                            .InstanceCount = 1,
                            .FirstIndex = uint32((CurrentWorkingLayer->BaseIndex + CurrentWorkingLayer->LayerOpaqueQuadCount) * 6),
                            .FirstVertex = 0,
                            .BaseInstance = 0,
                        };
                }

                if(CurrentWorkingLayer->LayerOpaqueQuadCount || CurrentWorkingLayer->LayerTransparentQuadCount) DrawFrame->ActiveLayerCounter++;
            }
        }

        for(int32 QuadIndex = 0;
            QuadIndex < DrawFrame->QuadCounter;
            ++QuadIndex)
        {
            render_quad *Quad = DrawFrame->QuadBuffer + QuadIndex;

            gl_vertex *BottomLeft  = VertexBufferptr + 0;
            gl_vertex *TopLeft     = VertexBufferptr + 1;
            gl_vertex *TopRight    = VertexBufferptr + 2;
            gl_vertex *BottomRight = VertexBufferptr + 3;
            VertexBufferptr += 4;

            BottomLeft->NDCPosition  = {Quad->BottomLeft.NDCPosition.X,  Quad->BottomLeft.NDCPosition.Y,  0, 1};
            TopLeft->NDCPosition     = {Quad->TopLeft.NDCPosition.X,     Quad->TopLeft.NDCPosition.Y,     0, 1};
            TopRight->NDCPosition    = {Quad->TopRight.NDCPosition.X,    Quad->TopRight.NDCPosition.Y,    0, 1};
            BottomRight->NDCPosition = {Quad->BottomRight.NDCPosition.X, Quad->BottomRight.NDCPosition.Y, 0, 1};

            BottomLeft->WorldPosition  = {Quad->BottomLeft.WorldPosition.X,  Quad->BottomLeft.WorldPosition.Y,  0, 1};
            TopLeft->WorldPosition     = {Quad->TopLeft.WorldPosition.X,     Quad->TopLeft.WorldPosition.Y,     0, 1};
            TopRight->WorldPosition    = {Quad->TopRight.WorldPosition.X,    Quad->TopRight.WorldPosition.Y,    0, 1};
            BottomRight->WorldPosition = {Quad->BottomRight.WorldPosition.X, Quad->BottomRight.WorldPosition.Y, 0, 1};

            
            BottomLeft->Color  = Quad->QuadColor;
            TopLeft->Color     = Quad->QuadColor;
            TopRight->Color    = Quad->QuadColor;
            BottomRight->Color = Quad->QuadColor;

            BottomLeft->TexCoords  = Quad->TopLeft.TexCoords;
            TopLeft->TexCoords 	   = Quad->BottomLeft.TexCoords;
            TopRight->TexCoords	   = Quad->BottomRight.TexCoords;
            BottomRight->TexCoords = Quad->TopRight.TexCoords;

            BottomLeft->TextureIndex  = Quad->BottomLeft.TextureIndex;
            TopLeft->TextureIndex 	  = Quad->TopLeft.TextureIndex;
            TopRight->TextureIndex	  = Quad->TopRight.TextureIndex;
            BottomRight->TextureIndex = Quad->BottomRight.TextureIndex;

            BottomLeft->RenderingOptions  = Quad->BottomLeft.RenderingOptions;
            TopLeft->RenderingOptions 	  = Quad->TopLeft.RenderingOptions;
            TopRight->RenderingOptions	  = Quad->TopRight.RenderingOptions;
            BottomRight->RenderingOptions = Quad->BottomRight.RenderingOptions;

            BottomLeft->Normals   = Quad->BottomLeft.Normals;
            TopLeft->Normals 	  = Quad->TopLeft.Normals;
            TopRight->Normals	  = Quad->TopRight.Normals;
            BottomRight->Normals  = Quad->BottomRight.Normals;

            BottomLeft->LitFactor  = Quad->BottomLeft.LitFactor;
            TopLeft->LitFactor 	   = Quad->TopLeft.LitFactor;
            TopRight->LitFactor	   = Quad->TopRight.LitFactor;
            BottomRight->LitFactor = Quad->BottomRight.LitFactor;
        }
    }

    // NOTE(Sleepster): This is indirect drawing using the command buffer
#if 0
    // OPAQUE
    {
        glDisable(GL_BLEND);
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_GREATER);
        glDepthMask(GL_TRUE);

        glBindBuffer(GL_ARRAY_BUFFER, RenderInfo->PrimaryVBOID);
        glBufferSubData(GL_ARRAY_BUFFER, 0, (DrawFrame->QuadCounter * 4) * sizeof(gl_vertex), DrawFrame->glVertexBuffer);

        glBindBuffer(GL_DRAW_INDIRECT_BUFFER, RenderInfo->IndirectDrawingCommandBufferID);
        glBufferSubData(GL_DRAW_INDIRECT_BUFFER, 0, (DrawFrame->ActiveLayerCounter) * sizeof(draw_elements_indirect_command), &DrawFrame->IndirectRenderCommandBuffer[0]);

        glUseProgram(DrawFrame->Shader.ShaderID);
        for(int32 ActiveTextureIndex = 0;
            ActiveTextureIndex < DrawFrame->ActiveTextureCount;
            ++ActiveTextureIndex)
        {
            glActiveTexture(GL_TEXTURE0 + (ActiveTextureIndex));
            glBindTexture(GL_TEXTURE_2D, DrawFrame->ActiveTextures[ActiveTextureIndex]);
        }

        glBindVertexArray(RenderInfo->PrimaryVAOID);
        glMultiDrawElementsIndirect(GL_TRIANGLES,
                                    GL_UNSIGNED_INT,
                                    0, 
                                    DrawFrame->ActiveLayerCounter,
                                    sizeof(draw_elements_indirect_command));
    }

    // TRANSPARENT
    {
        glDisable(GL_DEPTH_TEST);
        glDepthMask(GL_TRUE);

        glEnable(GL_BLEND);        
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glBlendEquation(GL_FUNC_ADD);

        glBindBuffer(GL_DRAW_INDIRECT_BUFFER, RenderInfo->IndirectDrawingCommandBufferID);
        glBufferSubData(GL_DRAW_INDIRECT_BUFFER,
                        GLint(MAX_LAYERS) * sizeof(draw_elements_indirect_command),
                        (DrawFrame->ActiveLayerCounter) * sizeof(draw_elements_indirect_command),
                        &DrawFrame->IndirectRenderCommandBuffer[32]);

        glUseProgram(DrawFrame->Shader.ShaderID);
        glMultiDrawElementsIndirect(GL_TRIANGLES,
                                    GL_UNSIGNED_INT,
                                    (void *)(int32((MAX_LAYERS) * sizeof(draw_elements_indirect_command))), 
                                    DrawFrame->ActiveLayerCounter,
                                    sizeof(draw_elements_indirect_command));
    }

    // NOTE(Sleepster): This is per layer rendering
#else

    glBindFramebuffer(GL_FRAMEBUFFER, RenderInfo->gBufferFBID);
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

    glBindBuffer(GL_ARRAY_BUFFER, RenderInfo->PrimaryVBOID);
    glBufferSubData(GL_ARRAY_BUFFER, 0, (DrawFrame->QuadCounter * 4) * sizeof(gl_vertex), DrawFrame->glVertexBuffer);

    glBindVertexArray(RenderInfo->PrimaryVAOID);
    glUseProgram(gBufferGeoShader->ProgramID);

    for(int32 ActiveTextureIndex = 0;
        ActiveTextureIndex < DrawFrame->ActiveTextureCount;
        ++ActiveTextureIndex)
    {
        glActiveTexture(GL_TEXTURE0 + (ActiveTextureIndex));
        glBindTexture(GL_TEXTURE_2D, DrawFrame->ActiveTextures[ActiveTextureIndex]);
    }

    // TODO(Sleepster): Figure out what to actually do with this 
    Assert(DrawFrame->ActiveTextureCount <= 15);
    glActiveTexture(GL_TEXTURE0 + 16);
    glBindTexture(GL_TEXTURE_2D, RenderInfo->CurrentlyActiveFont->FontAtlas.TextureID);

    // NOTE(Sleepster): Opaque pass
    for(uint32 LayerIndex = 0;
        LayerIndex < MAX_LAYERS;
        ++LayerIndex)
    {
        render_layer_info *CurrentWorkingLayer = &DrawFrame->Layers[LayerIndex];

        // DRAW OPAQUE
        if(CurrentWorkingLayer->LayerOpaqueQuadCount > 0)
        {
            glDisable(GL_BLEND);
            glEnable(GL_DEPTH_TEST);
            glDepthFunc(GL_LEQUAL);
            glDepthMask(GL_TRUE);
             
            glDrawElements(GL_TRIANGLES,
                           CurrentWorkingLayer->LayerOpaqueQuadCount * 6,
                           GL_UNSIGNED_INT,
                           (void *)((CurrentWorkingLayer->BaseIndex * 6) * sizeof(uint32)));
        }

        // DRAW TRANSPARENT
        if(CurrentWorkingLayer->LayerTransparentQuadCount > 0)
        {
            glDisable(GL_DEPTH_TEST);
            
            glEnable(GL_BLEND);        
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glBlendEquation(GL_FUNC_ADD);
             
            glDrawElements(GL_TRIANGLES,
                           CurrentWorkingLayer->LayerTransparentQuadCount* 6,
                           GL_UNSIGNED_INT,
                           (void *)(((CurrentWorkingLayer->BaseIndex + CurrentWorkingLayer->LayerOpaqueQuadCount) * 6) * sizeof(uint32)));
        }
    }

    // NOTE(Sleepster): This blits the depth buffer to the main framebuffer
    glBindFramebuffer(GL_READ_FRAMEBUFFER, RenderInfo->gBufferFBID);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glBlitFramebuffer(0, 0, SizeData.Width, SizeData.Height, 0, 0, SizeData.Width, SizeData.Height, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // NOTE(Sleepster): Lighting
    {
        // GLuint            gBufferPointLightSBOID;
        // GLuint            uBrightnessFactorUID;
        // GLuint            uWorldBrightnessFactorUID;
        // GLuint            uPointLightCountUID;

        glDisable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);        
        glBlendEquation(GL_FUNC_ADD);
        glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

        glDisable(0x809D); // Disabling multisampling

        glUseProgram(gBufferLightingShader->ProgramID);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, RenderInfo->gBufferTextures[0]);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, RenderInfo->gBufferTextures[1]);

        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, RenderInfo->gBufferTextures[2]);

        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, RenderInfo->gBufferPointLightSBOID);
        glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(point_light) * DrawFrame->PointLightCounter, DrawFrame->PointLights);

        glUniform1i(RenderInfo->uPointLightCountUID, DrawFrame->PointLightCounter);
        glUniform1f(RenderInfo->uBrightnessFactorUID, RenderBrightness);
        glUniform1f(RenderInfo->uWorldBrightnessFactorUID, CurrentWorldBrightness);

        glDrawArrays(GL_TRIANGLES, 0, 6);
    }

    #endif

    // NOTE(Sleepster): Compute shader test
#else
    {
        glDisable(GL_DEPTH_TEST);
        glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

        glUseProgram(DrawFrame->TestComputeShader.ProgramID);
        glBindImageTexture(0, RenderInfo->TestComputeTexture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);

        ShaderCounter += (1 * 0.001);
        glUniform1f(TestTimeLocation, ShaderCounter);

        glDispatchCompute((SizeData.Width + 7) / 8, (SizeData.Height + 3) / 4, 1);
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

        glUseProgram(TestQuadShader.ProgramID);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, RenderInfo->TestComputeTexture);

        glDrawArraysInstanced(GL_TRIANGLES, 0, 6, 1);
    }
#endif

    glFlush();

    DrawFrame->QuadCounter = 0;
    DrawFrame->ActiveTextureCount = 0;

    DrawFrame->ZSorting = true;
    DrawFrame->ActiveZLayer = 32;

    DrawFrame->PointLightCounter = 0;
    DrawFrame->WorkingTextureIndex = 0;

    ResetLayerData(DrawFrame);
}
