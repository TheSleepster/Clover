/* date = August 27 2024 06:39 am*/

#ifndef CLOVER_RENDERER_H
#define CLOVER_RENDERER_H

// NOTE(Sleepster): Freetype must come first due to the #define internal static inside of the intrinsics header
#include "../data/deps/Freetype/include/ft2build.h"
#include FT_FREETYPE_H

#include "Intrinsics.h"

// GLAD
#ifndef GLAD_OPENGL_IMPL
#define GLAD_OPENGL_IMPL
#include "../data/deps/OpenGL/glad/include/glad/glad.h"
#endif

#include "util/Math.h"
#include "util/Array.h"
#include "util/FileIO.h"
#include "util/String.h"

// IMGUI IMPl
#include "../data/deps/ImGui/imgui.h"

#include "shader/CommonShader.glh"
#include "Clover_Globals.h"
#include "Clover.h"

struct game_memory;
struct transient_state;

// TODO(Sleepster): Fix this, this is stupid
enum   asset_state;

struct static_sprite_data
{
    ivec2 AtlasOffset;
    ivec2 SpriteSize;
};

struct texture2d
{
    time_t LastWriteTime;
    string Filepath;
    GLuint TextureID;
    
    ivec3  TextureData;
    char  *RawData;
};

struct game_texture2d
{
    texture2d SpriteAtlas;  
    texture2d NormalAtlas;
};

// FONT RENDERING
#define MAX_FONT_SIZE 512
#define BITMAP_ATLAS_SIZE 512

struct freetype_font_data
{
    FT_Library FontFile;
    FT_Face    FontFace;
    uint32     FontSize;
    uint32     AtlasPadding;
};

struct font_glyph
{
    vec2      GlyphOffset;
    vec2      GlyphAdvance;
    ivec2     GlyphSize;
    ivec2     GlyphUVs;
};

struct font_data
{
    texture2d FontAtlas;
    font_glyph Glyphs[256]; 
    
    uint32 FontHeight;
    uint32 GlyphSize;

    char *RawData;
};

struct gl_shader_source
{
    GLuint   SourceID;
    string   Filepath;
    time_t   LastWriteTime;
};

struct shader
{
    gl_shader_source VertexShader;
    gl_shader_source FragmentShader;
    GLuint           ShaderID;

    string           Filepath;
    time_t           LastWriteTime;

    asset_state      ShaderState;    
};

struct orthocamera2d
{
    real32 Zoom;
    vec2   Position;
    vec2   Target;
    
    mat4   ViewMatrix;
    mat4   ProjectionMatrix;
    mat4   ProjectionViewMatrix;
};

struct material_data
{
    real32 Specular;
};

struct vertex
{
    vec4   Position;
    vec2   TextureCoords;
    vec3   VertexNormals;
    vec4   DrawColor;
    real32 TextureIndex;
};

struct quad
{
    // VERTEX DATA
    union
    {
        struct
        {
            vertex TopLeft;
            vertex TopRight;
            vertex BottomLeft;
            vertex BottomRight;
        };
        vertex Elements[4];
    };
    
    material_data Material;
    vec4          DrawColor;
    
    vec2   Position;
    vec2   Size;
    real32 Rotation;

    real32 TextureIndex;
    real32 ZLayer;
};

struct draw_frame_data
{
    quad       *QuadBuffer;
    quad       *QuadSortingBuffer;
    int32       QuadCounter;

    vertex     *Vertices;
    vertex     *VertexBufferptr;
    vertex     *TransparentVertexBufferptr;

    uint32      OpaqueQuadCount;
    uint32      TransparentQuadCount;
    uint32      LastFrameQuadCount;

    vertex     *UIVertices;
    vertex     *UIVertexBufferptr;
    vertex     *TransparentUIVertexBufferptr;

    uint32      OpaqueUIElementCount;
    uint32      TransparentUIElementCount;

    uint32      TotalQuadCount;
    uint32      TotalUIElementCount;

    point_light PointLights[MAX_POINT_LIGHTS];
    spot_light  SpotLights [MAX_SPOT_LIGHTS];

    int32       PointLightCount;
    int32       SpotLightCount;

    bool32      EnableZLayering;
    bool32      EnableZSorting;
};

// TODO(Sleepster): Figure out a better way to store our textures and shaders
struct gl_render_data
{
    // OPENGL STUFF
    GLuint GameVAOID;
    GLuint GameVBOID;
    GLuint GameEBOID;
    
    // TODO(Sleepster): Font Buffer 
    GLuint GameUIVAOID;
    GLuint GameUIVBOID;
    GLuint GameUIEBOID;
    
    // MATRICES
    GLuint ProjectionMatrixUID;
    GLuint ViewMatrixUID;

    GLuint BackgroundColorUID;

    GLuint gBufferViewMatrixUID;
    GLuint gBufferProjectionMatrixUID;

    // FRAMEBUFFERS
    GLuint gBufferFBOID;
    GLuint LightingFBOID;

    // SSBO
    GLuint PointLightSBOID;
    GLuint SpotLightSBOID;

    // OTHER UNIFORMS
    GLuint PointLightCountUID;
    GLuint SpotLightCountUID;
    
    // CAMERAS
    orthocamera2d GameCamera;
    orthocamera2d GameUICamera;
    
    // CLEAR COLOR
    vec4          ClearColor;
    
    // GAME TEXTURES
    real32        AspectRatio;

    // RENDERING TEXTURES
    GLuint gBuffer;
    GLuint gBufferTextures[3];
    GLuint LightmapID;

    // RENDER BUFFERS
    GLuint gBufferDepthRBID;

    // POST PROCESSING UNIFORMS
    GLuint  gBufferBrightnessUID;
    GLuint  BasicShaderBrightnessUID;
    GLuint  LightingShaderUserBrightnessUID;
    GLuint  LightingShaderWorldBrightnessUID;

    void(*CloverRender)(game_memory *GameMemory, gl_render_data *RenderData, transient_state *TransientState);

    // IMGUI STUFF
    ImGuiContext *CurrentImGuiContext;
    uint32        LastFrameQuadCount;

    memory_arena  VertexArena;
    memory_arena  UIVertexArena;
    memory_arena  QuadBufferArena;

    memory_arena  RendererArena;

    // DEBUG TIMERS
    GLuint StartTimer;
    GLuint EndTimer;
    real64 GPUTimeInMS;

    draw_frame_data DrawFrameData;
};

#define CLOVER_OGL_RENDER(name) void name(game_memory *GameMemory, gl_render_data *RenderData, transient_state *TransientState)
typedef CLOVER_OGL_RENDER(clover_ogl_render);


internal vec4 
HexToRGBA(int64 hex) 
{
    vec4 Result = {};
    
    uint8 R = (hex>>24) & 0x000000FF;
    uint8 G = (hex>>16) & 0x000000FF;
    uint8 B = (hex>>8) & 0x000000FF;
    uint8 A = (hex>>0) & 0x000000FF;
    
    Result = 
    {
        (real32)R / 255.0f,
        (real32)G / 255.0f,
        (real32)B / 255.0f,
        (real32)A / 255.0f,
    };
    
    return(Result);
}

void             CloverCreateSDFTexture(transient_state *TransientState, texture2d *TextureInfo, char *TextureData); 
void             CloverTestShader(GLuint TestID, GLuint Type); 
gl_shader_source CloverLoadShaderSource(memory_arena *Scratch, uint32 ShaderType, string Filepath);
shader           CloverCreateShader(memory_arena *Memory, string VertexShader, string FragmentShader);
void             CloverLoadTexture(transient_state *TransientState, texture2d *TextureInfo, string Filepath);
void             CloverReloadTexture(texture2d *TextureInfo, uint32 TextureIndex);
char *           CloverLoadSDFFontData(transient_state *TransientState, font_data *NewFont, string Filepath, uint32 FontSize);
void             CloverLoadSDFFont(transient_state *TransientState, font_data *NewFont, string Filepath, uint32 FontSize);

#endif // _CLOVER_RENDERER_H
