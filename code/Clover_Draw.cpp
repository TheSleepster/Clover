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
#include "../data/deps/OpenGL/glad/include/glad/glad.h"

#include "../data/deps/OpenGL/glext.h"
#include "../data/deps/OpenGL/wglext.h"
#include "../data/deps/OpenGL/glcorearb.h"

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../data/deps/stb/stb_image.h"
#include "../data/deps/stb/stb_image_write.h"
#endif

#include "shader/ShaderHeader.h"
#include "Clover_Globals.h"
#include "Win32_Clover.h"

// RENDERING INTERFACE FUNCTIONS

internal quad
CreateDrawQuad(gl_render_data *RenderData, 
               vec2            Position, 
               vec2            Size, 
               ivec2           SpriteSizeIn,
               ivec2           AtlasOffsetIn,
               real32          Rotation, 
               vec4            Color, 
               real32          TextureIndex)
{
    vec2 SpriteSize  = v2Cast(SpriteSizeIn);
    vec2 AtlasOffset = v2Cast(AtlasOffsetIn);
    
    const real32 LeftUV   = AtlasOffset.X;
    const real32 RightUV  = AtlasOffset.X + SpriteSize.X;
    const real32 TopUV    = AtlasOffset.Y;
    const real32 BottomUV = AtlasOffset.Y + SpriteSize.Y;
    
    quad Quad = {};
    Quad.Position    = Position;
    Quad.Size        = Size;
    Quad.Rotation    = Rotation;
    
    Quad.DrawColor = Color;
    Quad.TextureIndex = TextureIndex;
    
    Quad.TopLeft.TextureCoords     = {LeftUV, TopUV};
    Quad.TopRight.TextureCoords    = {RightUV, TopUV};
    Quad.BottomRight.TextureCoords = {RightUV, BottomUV};
    Quad.BottomLeft.TextureCoords  = {LeftUV, BottomUV};
    
    for(int32 Index = 0;
        Index < 4;
        ++Index)
    {
        Quad.Elements[Index].DrawColor    = Color;
        Quad.Elements[Index].TextureIndex = TextureIndex;
    }
    
    Quad.DrawColor    = Color;
    Quad.TextureIndex = TextureIndex;
    
    return(Quad);
}

internal void
DrawQuadXForm(gl_render_data *RenderData, quad *Quad, mat4 *Transform)
{
    if(RenderData->DrawFrame.QuadCount >= MAX_QUADS)
    {
        Check(0, "Max Quads Reached");
    }
    
    Quad->Elements[0].Position = vec4{-0.5f,  0.5f, 0.0f, 1.0f};
    Quad->Elements[1].Position = vec4{ 0.5f,  0.5f, 0.0f, 1.0f};
    Quad->Elements[2].Position = vec4{ 0.5f, -0.5f, 0.0f, 1.0f};
    Quad->Elements[3].Position = vec4{-0.5f, -0.5f, 0.0f, 1.0f};
    
    // TOP LEFT
    RenderData->DrawFrame.VertexBufferptr->Position      = mat4Transform(*Transform, Quad->Elements[0].Position);
    RenderData->DrawFrame.VertexBufferptr->TextureCoords = Quad->TopLeft.TextureCoords;
    RenderData->DrawFrame.VertexBufferptr->DrawColor     = Quad->DrawColor;
    RenderData->DrawFrame.VertexBufferptr->TextureIndex  = Quad->TextureIndex;
    RenderData->DrawFrame.VertexBufferptr++;
    
    // BOTTOM LEFT
    RenderData->DrawFrame.VertexBufferptr->Position      = mat4Transform(*Transform, Quad->Elements[1].Position);
    RenderData->DrawFrame.VertexBufferptr->TextureCoords = Quad->TopRight.TextureCoords;
    RenderData->DrawFrame.VertexBufferptr->DrawColor     = Quad->DrawColor;
    RenderData->DrawFrame.VertexBufferptr->TextureIndex  = Quad->TextureIndex;
    RenderData->DrawFrame.VertexBufferptr++;
    
    // BOTTOM RIGHT
    RenderData->DrawFrame.VertexBufferptr->Position      = mat4Transform(*Transform, Quad->Elements[2].Position);
    RenderData->DrawFrame.VertexBufferptr->TextureCoords = Quad->BottomRight.TextureCoords;
    RenderData->DrawFrame.VertexBufferptr->DrawColor     = Quad->DrawColor;
    RenderData->DrawFrame.VertexBufferptr->TextureIndex  = Quad->TextureIndex;
    RenderData->DrawFrame.VertexBufferptr++;
    
    // BOTTOM LEFT
    RenderData->DrawFrame.VertexBufferptr->Position      = mat4Transform(*Transform, Quad->Elements[3].Position);
    RenderData->DrawFrame.VertexBufferptr->TextureCoords = Quad->BottomLeft.TextureCoords;
    RenderData->DrawFrame.VertexBufferptr->DrawColor     = Quad->DrawColor;
    RenderData->DrawFrame.VertexBufferptr->TextureIndex  = Quad->TextureIndex;
    RenderData->DrawFrame.VertexBufferptr++;
    
    RenderData->DrawFrame.QuadCount++;
}

internal void
DrawQuadProjected(gl_render_data *RenderData, quad *Quad)
{
    mat4 Translation = mat4Multiply(mat4Identity(1.0f), mat4Translate(v2Expand(vec2{Quad->Position.X - (Quad->Size.X * 0.5f), Quad->Position.Y}, 0.0f)));
    mat4 Rotation    = mat4Multiply(mat4Identity(1.0f), mat4RHRotate(AngleRad(Quad->Rotation), vec3{0.0f, 0.0f, 1.0f}));
    mat4 Scale       = mat4MakeScale(v2Expand(Quad->Size, 1.0f));
    
    mat4 Transform = Translation * Rotation * Scale;
    
    return(DrawQuadXForm(RenderData, Quad, &Transform));
}

internal void
DrawQuadTextured(gl_render_data *RenderData, 
                 vec2            Position, 
                 vec2            Size, 
                 ivec2           AtlasOffset, 
                 ivec2           SpriteSize, 
                 real32          Rotation,
                 vec4            Color, 
                 uint32          TextureIndex)
{
    quad Quad = CreateDrawQuad(RenderData, Position, Size, SpriteSize, AtlasOffset, Rotation, Color, (real32)TextureIndex);
    return(DrawQuadProjected(RenderData, &Quad));
}

internal void
DrawQuad(gl_render_data *RenderData, vec2 Position, vec2 Size, real32 Rotation, vec4 Color)
{
    quad Quad = CreateDrawQuad(RenderData, Position, Size, ivec2{16, 16}, ivec2{0, 0}, Rotation, Color, 0);
    return(DrawQuadProjected(RenderData, &Quad));
}

internal void
DrawGameText(gl_render_data *RenderData, 
             string          Text,
             vec2            Position, 
             real32          FontScale, 
             font_index      Font, 
             vec4            Color) 
{
    vec2 TextOrigin = Position;
    
    for(uint32 StringIndex = 0;
        StringIndex < Text.Length;
        ++StringIndex)
    {
        if(Text.Data[StringIndex] == '\n')    
        {
            Position.Y += RenderData->LoadedFonts[Font].FontHeight * FontScale;
            Position.X = TextOrigin.X;
            continue;
        }
        
        char C = (Text.Data[StringIndex]);
        font_glyph Glyph = RenderData->LoadedFonts[Font].Glyphs[C];
        
        vec2 WorldPosition = {(Position.X + Glyph.GlyphOffset.X) * FontScale, (Position.Y - Glyph.GlyphOffset.Y) * FontScale};
        vec2 RenderScale   = {Glyph.GlyphSize.X * FontScale, (real32)Glyph.GlyphSize.Y * (FontScale * 2)};
        ivec2 AtlasOffset   = Glyph.GlyphUVs;
        ivec2 GlyphSize     = Glyph.GlyphSize;
        
        DrawQuadTextured(RenderData, Position, RenderScale, AtlasOffset, GlyphSize, 0.0f, Color, 1);
        Position.X += Glyph.GlyphAdvance.X * FontScale;
    }
}

internal void
DrawImGui(gl_render_data *RenderData, time Time)
{
    ImGui::SetCurrentContext(RenderData->CurrentImGuiContext);
    
    ImGui::Begin("Render Quad Color Picker");
    ImGui::Text("Famerate: %i", Time.FPSCounter);
    ImGui::Text("FrameTime: %.02f", Time.MSPerFrame);
    ImGui::Separator();
    
    ImGui::Text("Clear Color:");
    ImGui::ColorPicker4("ClearColor", &RenderData->ClearColor.R, ImGuiColorEditFlags_PickerHueWheel);
    ImGui::SameLine();
    ImGui::End();
    //RenderData->GameCamera.Position += vec2{0.001f, 0.0} * Time.Delta;
}