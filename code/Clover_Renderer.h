/* date = August 27 2024 06:39 am*/

#ifndef CLOVER_RENDERER_H
#define CLOVER_RENDERER_H

#include "Intrinsics.h"

#ifndef GLAD_OPENGL_IMPL
// GLAD
#define GLAD_OPENGL_IMPL
#include "../data/deps/OpenGL/glad/include/glad/glad.h"

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../data/deps/stb/stb_image.h"
#include "../data/deps/stb/stb_image_write.h"
#endif

#include "util/Math.h"
#include "util/Array.h"
#include "util/FileIO.h"
#include "util/CustomStrings.h"

#include "../data/deps/OpenGL/glext.h"
#include "../data/deps/OpenGL/wglext.h"
#include "../data/deps/OpenGL/glcorearb.h"

// IMGUI IMPl
#include "../data/deps/ImGui/imgui.h"
#include "../data/deps/ImGui/imgui_impl_win32.h"
#include "../data/deps/ImGUI/imgui_impl_opengl3.h"

#include "Clover.h"
#include "shader/ShaderHeader.h"
#include "Clover_Globals.h"

enum bound_texture_index
{
    TEXTURE_NIL,
    GAME_ATLAS,
    BoundTextureCount,
};

enum font_index
{
    UBUNTU_MONO,
    LITERATION_MONO,
    FONT_COUNT,
};

struct static_sprite_data
{
    ivec2 AtlasOffset;
    ivec2 SpriteSize;
};

struct texture2d
{
    FILETIME LastWriteTime;
    
    ivec3  TextureData;
    char  *RawData;
    string Filepath;
    
    GLuint TextureID;
    GLenum ActiveTexture;
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
};


struct gl_shader_source
{
    GLuint   SourceID;
    string   Filepath;
    FILETIME LastWriteTime;
};

struct shader
{
    gl_shader_source VertexShader;
    gl_shader_source FragmentShader;
    GLuint           Shader;
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

struct vertex
{
    vec4   Position;
    vec2   TextureCoords;
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
    
    vec4   DrawColor;
    real32 TextureIndex;
    
    vec2 Position;
    vec2 Size;
    
    real32 Rotation;
};

struct gl_render_data
{
    // OPENGL STUFF
    GLuint GameVAOID;
    GLuint GameVBOID;
    GLuint GameEBOID;
    
    GLuint GameUIVAOID;
    GLuint GameUIVBOID;
    GLuint GameUIEBOID;
    
    GLuint ProjectionViewMatrixID;
    
    // SHADERS
    shader    BasicShader;
    
    // CAMERAS
    orthocamera2d GameCamera;
    orthocamera2d GameUICamera;
    
    // CLEAR COLOR
    vec4          ClearColor;
    
    // TEXTURES
    texture2d     GameAtlas;
    font_data     LoadedFonts[2];
    uint32        TextureCount;
    real32        AspectRatio;
    
    // IMGUI SHIT
    ImGuiContext *CurrentImGuiContext;
    
    // DRAW FRAME DATA
    struct
    {
        vertex *Vertices;
        vertex *VertexBufferptr;
        uint32  QuadCount;
        
        vertex *UIVertices;
        vertex *UIVertexBufferptr;
        uint32  UIElementCount;
    }DrawFrame;
};

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
#endif // _CLOVER_RENDERER_H