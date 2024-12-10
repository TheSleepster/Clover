#if !defined(CLOVER_RENDERING_H)
/* ========================================================================
   $File: Clover_Rendering.h $
   $Date: Mon, 09 Dec 24: 11:46AM $
   $Revision: $
   $Creator: Justin Lewis $
   ======================================================================== */

#define CLOVER_RENDERING_H

#include "Clover_Shader.h"
#include "shader/CommonShader.glh"

#include "../data/deps/ImGui/imgui.h"
#include "Clover_Globals.h"

constexpr uint32 MAX_LAYER_BITS      = 21;
constexpr uint32 MAX_ACTIVE_TEXTURES = 32;
constexpr uint32 MAX_LAYERS          = 32;

struct clover_texture
{
    string Filepath;
    time_t LastWriteTime;

    ivec3  TextureChannelData;
    char  *RawData;

    uint32 TextureID;
};

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
    vec2 Position;
    vec2 RenderSize;
    
    real32 Rotation;
    int32  ZLayer;

    mat4   xForm;
    vec4   QuadColor;
    int32  SortingTransparency;
    uint32 RenderingOptions;

    uint32 BoundTextureID;
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

    string            RawData;
};

struct gl_render_info
{
    uint32 PrimaryVAOID;
    uint32 PrimaryVBOID;
    uint32 PrimaryEBOID;

    uint32 ProjectionMatrixUID;
    uint32 ViewMatrixUID;

    // INDIRECT DRAWING STUFF
    uint32 IndirectDrawingCommandBufferID;

    // DEFERRED LIGHTING BUFFER
    uint32 gBufferFBID;
    uint32 gBufferDepthRBID;
    uint32 gBufferTextures[3];

    // UNIFORM DATA
    uint32            gBufferPointLightSBOID;
    uint32            uBrightnessFactorUID;
    uint32            uWorldBrightnessFactorUID;
    uint32            uPointLightCountUID;

    // SHADERS
    clover_shader     Shader;
    clover_shader     gBufferShader;
    clover_shader     gBufferLightingShader;
    clover_shader     TestComputeShader;
    
    // FONT 
    clover_font_data *CurrentlyActiveFont;

    // DEBUG
    uint32 ComputeFramebuffer;
    uint32 TestComputeTexture;
    clover_texture *Testure;

    // DEBUG TIMERS
    uint32 StartTimer;
    uint32 EndTimer;
    real64 GPUTimeInMS;

    // IMGUI
    ImGuiContext *CurrentImGuiContext;
};

struct draw_elements_indirect_command
{
    uint32 IndexCount;
    uint32 InstanceCount;
    uint32 FirstIndex;
    uint32 FirstVertex;
    uint32 BaseInstance;
};

struct render_layer_info
{
    int32 BaseIndex;
    int32 EndingIndex;

    int32 LayerOpaqueQuadCount;
    int32 LayerTransparentQuadCount;
};

struct camera2d
{
    real32 Zoom;
    vec2   Position;
    vec2   Target;
    
    mat4   ViewMatrix;
    mat4   ProjectionMatrix;
    mat4   ProjectionViewMatrix;
};

// TODO(Sleepster): This is meant to transient. Make it so that resetting it is as a simple as DrawFrame = {}; 
struct gl_draw_frame_data
{
    uint32             ActiveTextures[MAX_ACTIVE_TEXTURES];
    int32              ActiveTextureCount;
    int32              WorkingTextureIndex;

    mat4      	       ProjectionMatrix;
    mat4      	       ViewMatrix;

    render_quad       *QuadBuffer;	
    render_quad       *QuadSortingBuffer;
    gl_vertex         *glVertexBuffer;
    int32              QuadCounter;

    bool32             ZSorting;
    render_quad        NullQuad;

    int32              ActiveZLayer;

    point_light        PointLights[MAX_POINT_LIGHTS];
    int32              PointLightCounter;

    uint32             ActiveLayerCounter;
    render_layer_info  Layers[MAX_LAYERS];

    draw_elements_indirect_command IndirectRenderCommandBuffer[MAX_LAYERS * 2];

    camera2d           SceneCamera;
};

void 
CloverLoadTextureData(gl_render_info *RenderInfo, clover_texture *Texture, string Filepath);

string
CloverLoadFontData(memory_arena *Arena, clover_font_data *FontData, string Filepath, uint32 FontSize);

#endif
