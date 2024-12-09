/* ========================================================================
   $File: Clover_OpenGL.cpp $
   $Date: Mon, 25 Nov 24: 08:PM $
   $Revision: $
   $Creator: Justin Lewis $
   ======================================================================== */
#include "Clover_Shader.cpp"

#include "shader/CommonShader.glh"

constexpr uint32 MAX_LAYER_BITS      = 21;
constexpr uint32 MAX_ACTIVE_TEXTURES = 32;
constexpr uint32 MAX_LAYERS          = 32;

struct clover_texture
{
    string Filepath;
    time_t LastWriteTime;

    ivec3  TextureChannelData;
    char  *RawData;

    GLuint TextureID;
};

uint32 QuadCounter = 3;
uint32 TransparentQuadCounter = 4;

struct gl_vertex
{
    vec4   NDCPosition;
    vec4   WorldPosition;
    vec4   Color;
    vec2   TexCoords;
    vec3   Normals;
    
    int32   TextureIndex;
    uint32  RenderingOptions;
    real32  LitFactor;
};

struct render_quad
{
    union
    {
        gl_vertex Elements[4];
        struct
        {
            gl_vertex TopLeft;
            gl_vertex TopRight;
            gl_vertex BottomLeft;
            gl_vertex BottomRight;
        };
    };
    real32 Rotation;
    int32  ZLayer;

    mat4   xForm;
    vec4   QuadColor;
    int32  SortingTransparency;
    uint32 RenderingOptions;

    GLuint BoundTextureID;
};

struct clover_font_glyph
{
    vec2      Offset;
    vec2      Bearing;
    vec2      Advance;
    ivec2     Size;
    ivec2     UVs;
};

// NOTE(Sleepster): The Glyph count is 256 for ASCII
struct clover_font_data
{
    clover_texture    FontAtlas;
    clover_font_glyph Glyphs[256];

    uint32            SizeOnLoad;
    uint32            FontHeight;
    uint32            GlyphSize;

    char              *RawData;
};

struct gl_render_info
{
    GLuint PrimaryVAOID;
    GLuint PrimaryVBOID;
    GLuint PrimaryEBOID;

    GLuint ProjectionMatrixUID;
    GLuint ViewMatrixUID;

    // INDIRECT DRAWING STUFF
    GLuint IndirectDrawingCommandBufferID;

    // DEFERRED LIGHTING BUFFER
    GLuint gBufferFBID;
    GLuint gBufferDepthRBID;
    GLuint gBufferTextures[3];

    // DEBUG
    GLuint ComputeFramebuffer;
    GLuint TestComputeTexture;
    clover_texture Testure;
};

struct draw_elements_indirect_command
{
    GLuint IndexCount;
    GLuint InstanceCount;
    GLuint FirstIndex;
    GLuint FirstVertex;
    GLuint BaseInstance;
};

struct render_layer_info
{
    int32 BaseIndex;
    int32 EndingIndex;

    int32 LayerOpaqueQuadCount;
    int32 LayerTransparentQuadCount;
};

// TODO(Sleepster): This is meant to transient. Make it so that resetting it is as a simple as DrawFrame = {}; 
struct gl_draw_frame_data
{
    GLuint         ActiveTextures[MAX_ACTIVE_TEXTURES];
    int32          ActiveTextureCount;

    mat4      	   ProjectionMatrix;
    mat4      	   ViewMatrix;

    render_quad   *QuadBuffer;	
    render_quad   *QuadSortingBuffer;
    gl_vertex     *glVertexBuffer;

    int32          QuadCounter;

    bool32         ZSorting;
    render_quad    NullQuad;

    int32          ActiveZLayer;

    clover_shader     Shader;
    clover_shader     gBufferShader;
    clover_shader     gBufferLightingShader;

    GLuint            gBufferPointLightSBOID;
    GLuint            uBrightnessFactorUID;
    GLuint            uWorldBrightnessFactorUID;
    GLuint            uPointLightCountUID;

    point_light       PointLights[MAX_POINT_LIGHTS];
    int32             PointLightCounter;
    
    clover_shader     TestComputeShader;
    clover_font_data  CurrentlyActiveFont;

    uint32            ActiveLayerCounter;
    render_layer_info Layers[MAX_LAYERS];
    draw_elements_indirect_command IndirectRenderCommandBuffer[MAX_LAYERS * 2];
};

constexpr uint32 IndirectDrawCommandBufferSize   = (MAX_LAYERS * 2) * sizeof(draw_elements_indirect_command);

internal render_quad* DrawQuad(gl_draw_frame_data *DrawFrame, vec2 Position, vec2 RenderSize, vec4 Color, uint32 RenderingOptions = 0, real32 LitFactor = 1.0f);
internal render_quad* DrawQuadTextured(gl_draw_frame_data *DrawFrame, vec2 Position, vec2 RenderSize, clover_texture *Texture, ivec2 AtlasOffset, ivec2 SpriteSize, vec4 Color, uint32 RenderingOptions = 0, real32 LitFactor = 1.0f);
internal render_quad* DrawTextureXForm(gl_draw_frame_data *DrawFrame, mat4 XForm, vec2 RenderSize, clover_texture *Texture, ivec2 AtlasOffset, ivec2 SpriteSize, vec4 Color, uint32 RenderingOptions = 0, real32 LitFactor = 1.0f);
internal render_quad* DrawQuadXForm(gl_draw_frame_data *DrawFrame, mat4 XForm, vec2 RenderSize, vec4 Color, uint32 RenderingOptions = 0, real32 LitFactor = 1.0f);
internal void         DisplayText(gl_draw_frame_data *DrawFrame, string TextToRender, vec2 Position, uint32 FontSize, vec4 Color, clover_font_data *FontID, uint32 RenderingOptions = 0, real32 LitFactor = 1.0f);
internal point_light* CreatePointLight(gl_draw_frame_data *DrawFrame, vec2 Position, real32 Strength, real32 Radius, attenuation_data *Attenuation, vec4 Color);


internal inline void PushZLayer(gl_draw_frame_data *DrawFrame, int Layer);
internal inline void PopZLayer(gl_draw_frame_data *DrawFrame);

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

internal void 
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
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

            glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB_ALPHA, 
                         Texture->TextureChannelData.Width, Texture->TextureChannelData.Height, 0, 
                         GL_RGBA, GL_UNSIGNED_BYTE, Texture->RawData);
        }
        else
        {
            cl_Error("Could not extract image data. Image may be invalid.\n");
        }

        glBindTexture(GL_TEXTURE_2D, 0);
    	stbi_image_free(Texture->RawData);
    }
    else
    {
        cl_Error("Texture memory Invalid\n");
    }
}

// NOTE)Sleepster): This creates and Loads an SDF Font 
internal string
CloverLoadFont(gl_render_info *RenderInfo, memory_arena *Arena, clover_font_data *FontData, string Filepath, uint32 FontSize)
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

                    // TODO(Sleepster): When we hook this up to the asset system, remove this:
                    glActiveTexture(GL_TEXTURE0 + 1);
                    glGenTextures(1, &FontData->FontAtlas.TextureID);
                    glBindTexture(GL_TEXTURE_2D, FontData->FontAtlas.TextureID);
                    glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, CLOVER_FONT_ATLAS_SIZE, CLOVER_FONT_ATLAS_SIZE, 0, GL_RED, GL_UNSIGNED_BYTE, CSTR(TTFFontData));

                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 3);
                    glGenerateMipmap(GL_TEXTURE_2D);

                    glBindTexture(GL_TEXTURE_2D, 0);
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


GLuint TestTimeLocation;
clover_shader TestQuadShader;
float ShaderCounter;


internal void
CloverInitializeOpenGLRenderer(gl_render_info *RenderInfo, gl_draw_frame_data *DrawFrame, transient_state *TransientState)
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

        DrawFrame->Shader =
            CloverLoadBasicPixelShader(&TransientState->Garbage,
                                       STR("../code/shader/new/Basic.vert"),
                                       STR("../code/shader/new/Basic.frag"));

        TestQuadShader =
            CloverLoadBasicPixelShader(&TransientState->Garbage,
                                       STR("../code/shader/new/Quad.vert"),
                                       STR("../code/shader/new/Quad.frag"));
        
        DrawFrame->TestComputeShader =
            CloverLoadComputeShader(&TransientState->Garbage,
                                    STR("../code/shader/new/RenderWeirdGradient.comp"));

        DrawFrame->gBufferShader =
            CloverLoadBasicPixelShader(&TransientState->Garbage,
                                       STR("../code/shader/new/gBuffer_Geometry.vert"),
                                       STR("../code/shader/new/gBuffer_Geometry.frag"));
        
        DrawFrame->gBufferLightingShader =
            CloverLoadBasicPixelShader(&TransientState->Garbage,
                                       STR("../code/shader/new/Quad.vert"),
                                       STR("../code/shader/new/gBuffer_Lighting.frag"));
        
    }

    // UNIFORMS
    {
        DrawFrame->gBufferPointLightSBOID    = glGetUniformLocation(DrawFrame->gBufferLightingShader.ProgramID, "gBufferPointLightSBO");
        DrawFrame->uBrightnessFactorUID      = glGetUniformLocation(DrawFrame->gBufferLightingShader.ProgramID, "uBrightnessFactor");
        DrawFrame->uWorldBrightnessFactorUID = glGetUniformLocation(DrawFrame->gBufferLightingShader.ProgramID, "uWorldBrightness");
        DrawFrame->uPointLightCountUID       = glGetUniformLocation(DrawFrame->gBufferLightingShader.ProgramID, "uPointLightCount");
        
        TestTimeLocation = glGetUniformLocation(DrawFrame->TestComputeShader.ProgramID, "Time");
    }

    // BUFFERS
    {
        uint64 MaxBufferSize = sizeof(struct point_light) * MAX_POINT_LIGHTS;
        glGenBuffers(1, &DrawFrame->gBufferPointLightSBOID);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, DrawFrame->gBufferPointLightSBOID);
        glBufferData(GL_SHADER_STORAGE_BUFFER, MaxBufferSize, 0, GL_DYNAMIC_DRAW);

        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    }
    
    CloverLoadFont(RenderInfo, &TransientState->Garbage, &DrawFrame->CurrentlyActiveFont, STR("../data/res/fonts/UbuntuMono-B.ttf"), 48);
    CloverLoadTextureData(RenderInfo, &RenderInfo->Testure, STR("../data/res/textures/TextureAtlas.png"));
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
CloverOpenGLRender(gl_render_info *RenderInfo, gl_draw_frame_data *DrawFrame, time_data Time)
{

    DrawFrame->QuadCounter = 0;
    DrawFrame->ActiveTextureCount = 0;

    DrawFrame->ZSorting = true;
    DrawFrame->ActiveZLayer = 32;

    DrawFrame->PointLightCounter = 0;

    ResetLayerData(DrawFrame);

    // TODO(Sleepster): Change this ViewMatrix so that it zooms in when the screen gets larger  
	DrawFrame->ViewMatrix 		= mat4Scale(mat4Identity(1.0f), vec3{real32(SizeData.Width / 160), real32(SizeData.Width / 160), 1.0});
    DrawFrame->ProjectionMatrix = mat4RHGLOrtho((real32)-SizeData.Width,
                                                (real32)SizeData.Width,
                                                (real32)-SizeData.Height,
                                                (real32)SizeData.Height,
                                                -1.0f,
                                                1.0f);
#if 1
    mat4 XForm = mat4Identity(1.0f);
    XForm = mat4Translation(XForm, vec3{0, 32});
    XForm = mat4Scale(XForm, vec3{16, 16, 1});

    uint32 FontOptions = RENDERING_OPTION_FONT;

    PushZLayer(DrawFrame, 14);
    DrawQuadXForm(DrawFrame, XForm, {1, 1}, RED);

    XForm = mat4Identity(1.0f);
    XForm = mat4Translation(XForm, vec3{8, 32});

    PushZLayer(DrawFrame, 3);
    DrawQuad(DrawFrame, {-16, 0}, {16, 16}, BLUE);

    PushZLayer(DrawFrame, 4);
    DrawQuadTextured(DrawFrame, {32, 0}, {16, 16}, &RenderInfo->Testure, {96, 0}, {11, 11}, WHITE);
    DrawQuad(DrawFrame, {-32, 0}, {16, 16}, WHITE);

    PushZLayer(DrawFrame, 5);
    DrawQuad(DrawFrame, {-42, 0}, {16, 16}, {0.0f, 0.2f, 0.0f, 0.3f}, 0, 1.0f);

    PushZLayer(DrawFrame, 3);
    DrawQuad(DrawFrame, {-36, -10}, {16, 16}, {0.4f, 0.0f, 0.0f, 0.6f}, 0, 0.0f);


    attenuation_data TestLightData = {.Constant = 0.05, .Linear = 0.0009, .Quadratic = 0.001};
    CreatePointLight(DrawFrame, vec2{-100, 0}, 20.0, 100, &TestLightData, GREEN);
    CreatePointLight(DrawFrame, vec2{ 100, 0}, 10.0, 100, &TestLightData, BLUE);
    CreatePointLight(DrawFrame, vec2{ 0,  -100}, 40.0, 100, &TestLightData, RED);
    CreatePointLight(DrawFrame, vec2{ 0,   100}, 50.0, 100, &TestLightData, YELLOW);

    PushZLayer(DrawFrame, 15);
    DrawTextureXForm(DrawFrame, XForm, {16, 16}, &RenderInfo->Testure, {96, 0}, {11, 11}, WHITE, 0, 0.0f);
    DisplayText(DrawFrame, STR("The Quick, Brown Fox Jumps Over The Lazy Dog!"), {-150, -20}, 6, GREEN, &DrawFrame->CurrentlyActiveFont, FontOptions, 0.0f);

    PushZLayer(DrawFrame, 0);
    ivec2  TileRadius   = {32, 32};
    for(int32 TileX = 0;
        TileX < TileRadius.X;
        ++TileX)
    {
        for(int32 TileY = 0;
            TileY < TileRadius.Y;
            ++TileY)
        {
            if((TileX + (TileY % 2 == 0)) % 2 == 0)
            {
                real32 X = TileX * TILE_SIZE;
                real32 Y = TileY * TILE_SIZE;
                DrawQuad(DrawFrame, {X - (TILE_SIZE * 16), (Y - TILE_SIZE) - (TILE_SIZE * 12)}, {16, 16}, DARK_GRAY);
            }
            else
            {
                real32 X = TileX * TILE_SIZE;
                real32 Y = TileY * TILE_SIZE;
                DrawQuad(DrawFrame, {X - (TILE_SIZE * 16), (Y - TILE_SIZE) - (TILE_SIZE * 12)}, {16, 16}, DARKER_GRAY);
            }
        }
    }
    PopZLayer(DrawFrame);

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

#if 0
            for(uint32 LayerIndex = 0;
                LayerIndex < MAX_LAYERS;
                ++LayerIndex)
            {
                render_layer_info *CurrentWorkingLayer = &DrawFrame->Layers[LayerIndex];

                cl_Info("Layer[%i]: CurrentWorkingLayer->BaseIndex = %d\n",      LayerIndex, CurrentWorkingLayer->BaseIndex);
                cl_Info("Layer[%i]: CurrentWorkingLayer->EndingIndex = %d\n",    LayerIndex, CurrentWorkingLayer->EndingIndex);
                cl_Info("Layer[%i]: CurrentWorkingLayer->LayerQuadCount = %d\n", LayerIndex, CurrentWorkingLayer->LayerQuadCount);
            }
#endif
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
    glUseProgram(DrawFrame->gBufferShader.ProgramID);

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
    glBindTexture(GL_TEXTURE_2D, DrawFrame->CurrentlyActiveFont.FontAtlas.TextureID);

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

        glUseProgram(DrawFrame->gBufferLightingShader.ProgramID);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, RenderInfo->gBufferTextures[0]);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, RenderInfo->gBufferTextures[1]);

        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, RenderInfo->gBufferTextures[2]);

        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, DrawFrame->gBufferPointLightSBOID);
        glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(point_light) * DrawFrame->PointLightCounter, DrawFrame->PointLights);

        glUniform1i(DrawFrame->uPointLightCountUID, DrawFrame->PointLightCounter);
        glUniform1f(DrawFrame->uBrightnessFactorUID, RenderBrightness);
        glUniform1f(DrawFrame->uWorldBrightnessFactorUID, CurrentWorldBrightness);

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
}
