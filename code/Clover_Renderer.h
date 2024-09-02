/* date = August 27 2024 06:39 am*/

#ifndef CLOVER_RENDERER_H
#define CLOVER_RENDERER_H

// NOTE(Sleepster): Freetype must come first due to the #define internal static inside of the intrinsics header
#include "../data/deps/Freetype/include/ft2build.h"
#include FT_FREETYPE_H

#include "Intrinsics.h"

#include "util/Math.h"
#include "util/Array.h"
#include "util/FileIO.h"
#include "util/CustomStrings.h"

#include "../data/deps/OpenGL/glext.h"
#include "../data/deps/OpenGL/wglext.h"
#include "../data/deps/OpenGL/glcorearb.h"

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

    mat4   ViewMatrix;
    mat4   ProjectionMatrix;
    mat4   ProjectionViewMatrix;
};

struct vertex
{
    vec3   Position;
    vec2   TextureCoords;
    vec4   MatColor = WHITE;
    real32 TextureIndex;
};

struct quad
{
    // VERTEX DATA
    union
    {
        vertex Vertices[4];
        struct
        {
            vertex TopLeft;
            vertex TopRight;
            vertex BottomLeft;
            vertex BottomRight;
        };
    };

    vec4   MatColor = WHITE;
    real32 TextureIndex;
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

struct gl_render_data
{
    // OPENGL STUFF
    GLuint GameVAOID;
    GLuint GameVBOID;
    GLuint GameEBOID;

    GLuint ProjectionViewMatrixID;

    // SHADERS
    shader    BasicShader;

    // CAMERAS
    orthocamera2d GameCamera;
    
    // CLEAR COLOR
    vec4      ClearColor;
    
    // TEXTURES
    texture2d GameAtlas;
    font_data LoadedFonts[2];
    uint32    TextureCount;
    real32 AspectRatio;

    // GAME ASSETS
    struct 
    { 
        static_sprite_data Sprites[3];
    }GameData;

    // DRAW FRAME DATA
    struct
    {
        vertex *Vertices;
        vertex *VertexBufferptr;
        
        uint32 QuadCount;
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

internal void CloverRender(gl_render_data *RenderData);

#endif // _CLOVER_RENDERER_H

