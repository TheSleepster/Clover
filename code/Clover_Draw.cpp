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

internal quad
CreateDrawRect(gl_render_data *RenderData, vec2 Size, real32 Rotation, vec4 Color)
{
    quad Quad = {};

    Quad.TopLeft.Position     = {0, 0};
    Quad.TopRight.Position    = {Size.X, 0};
    Quad.BottomRight.Position = {Size.X, Size.Y};
    Quad.BottomLeft.Position  = {0, Size.Y}; 
    
    Quad.TopLeft.DrawColor = Color;
    Quad.TopRight.DrawColor = Color;
    Quad.BottomRight.DrawColor = Color;
    Quad.BottomLeft.DrawColor = Color;

    Quad.DrawColor = Color;
    Quad.Size = Size;
    Quad.Rotation = Rotation;

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
DrawUIQuadXForm(gl_render_data *RenderData, quad *Quad, mat4 *Transform)
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
    RenderData->DrawFrame.UIVertexBufferptr->Position      = mat4Transform(*Transform, Quad->Elements[0].Position);
    RenderData->DrawFrame.UIVertexBufferptr->TextureCoords = Quad->TopLeft.TextureCoords;
    RenderData->DrawFrame.UIVertexBufferptr->DrawColor     = Quad->DrawColor;
    RenderData->DrawFrame.UIVertexBufferptr->TextureIndex  = Quad->TextureIndex;
    RenderData->DrawFrame.UIVertexBufferptr++;
    
    // BOTTOM LEFT
    RenderData->DrawFrame.UIVertexBufferptr->Position      = mat4Transform(*Transform, Quad->Elements[1].Position);
    RenderData->DrawFrame.UIVertexBufferptr->TextureCoords = Quad->TopRight.TextureCoords;
    RenderData->DrawFrame.UIVertexBufferptr->DrawColor     = Quad->DrawColor;
    RenderData->DrawFrame.UIVertexBufferptr->TextureIndex  = Quad->TextureIndex;
    RenderData->DrawFrame.UIVertexBufferptr++;
    
    // BOTTOM RIGHT
    RenderData->DrawFrame.UIVertexBufferptr->Position      = mat4Transform(*Transform, Quad->Elements[2].Position);
    RenderData->DrawFrame.UIVertexBufferptr->TextureCoords = Quad->BottomRight.TextureCoords;
    RenderData->DrawFrame.UIVertexBufferptr->DrawColor     = Quad->DrawColor;
    RenderData->DrawFrame.UIVertexBufferptr->TextureIndex  = Quad->TextureIndex;
    RenderData->DrawFrame.UIVertexBufferptr++;
    
    // BOTTOM LEFT
    RenderData->DrawFrame.UIVertexBufferptr->Position      = mat4Transform(*Transform, Quad->Elements[3].Position);
    RenderData->DrawFrame.UIVertexBufferptr->TextureCoords = Quad->BottomLeft.TextureCoords;
    RenderData->DrawFrame.UIVertexBufferptr->DrawColor     = Quad->DrawColor;
    RenderData->DrawFrame.UIVertexBufferptr->TextureIndex  = Quad->TextureIndex;
    RenderData->DrawFrame.UIVertexBufferptr++;
    
    RenderData->DrawFrame.UIElementCount++;
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
DrawUIQuadProjected(gl_render_data *RenderData, quad *Quad)
{
    mat4 Translation = mat4Multiply(mat4Identity(1.0f), mat4Translate(v2Expand(vec2{Quad->Position.X - (Quad->Size.X * 0.5f), Quad->Position.Y}, 0.0f)));
    mat4 Rotation    = mat4Multiply(mat4Identity(1.0f), mat4RHRotate(AngleRad(Quad->Rotation), vec3{0.0f, 0.0f, 1.0f}));
    mat4 Scale       = mat4MakeScale(v2Expand(Quad->Size, 1.0f));
    
    mat4 Transform = Translation * Rotation * Scale;
    
    return(DrawUIQuadXForm(RenderData, Quad, &Transform));
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
DrawUIQuadTextured(gl_render_data *RenderData, 
                   vec2            Position, 
                   vec2            Size, 
                   ivec2           AtlasOffset, 
                   ivec2           SpriteSize, 
                   real32          Rotation,
                   vec4            Color, 
                   uint32          TextureIndex)
{
    quad Quad = CreateDrawQuad(RenderData, Position, Size, SpriteSize, AtlasOffset, Rotation, Color, (real32)TextureIndex);
    return(DrawUIQuadProjected(RenderData, &Quad));
}

// TODO(Sleepster): Perhaps make it where we have solids, and actors. Solids will be placed without matrix transforms.
//                  STATIC SOLIDS if you would. Actors will be Dynamic and will require matrix calculations.
internal void
DrawQuad(gl_render_data *RenderData, vec2 Position, vec2 Size, real32 Rotation, vec4 Color)
{
    quad Quad = CreateDrawQuad(RenderData, Position, Size, ivec2{16, 16}, ivec2{0, 0}, Rotation, Color, 0);
    return(DrawQuadProjected(RenderData, &Quad));
}

internal void
DrawUIQuad(gl_render_data *RenderData, vec2 Position, vec2 Size, real32 Rotation, vec4 Color)
{
    quad Quad = CreateDrawQuad(RenderData, Position, Size, ivec2{16, 16}, ivec2{0, 0}, Rotation, Color, 0);
    return(DrawUIQuadProjected(RenderData, &Quad));
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
DrawUIText(gl_render_data *RenderData, 
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
        
        DrawUIQuadTextured(RenderData, Position, RenderScale, AtlasOffset, GlyphSize, 0.0f, Color, 1);
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

internal inline static_sprite_data
GetSprite(game_state *State, sprite_type Sprite)
{
    return(State->GameData.Sprites[Sprite]);
}

internal inline void
DrawSprite(gl_render_data    *RenderData,
           static_sprite_data SpriteData, 
           vec2               Position, 
           vec2               RenderSize, 
           vec4               Color, 
           real32             Rotation,
           uint32             TextureIndex)
{    
    return(DrawQuadTextured(RenderData, 
                            Position, 
                            RenderSize, 
                            SpriteData.AtlasOffset, 
                            SpriteData.SpriteSize, 
                            Rotation, 
                            Color, 
                            TextureIndex));
}

internal void
DrawEntity(gl_render_data *RenderData, game_state *State, entity *Entity, vec2 Position, vec4 Color)
{
    static_sprite_data SpriteData = State->GameData.Sprites[Entity->Sprite];
    return(DrawSprite(RenderData, SpriteData, Entity->Position, v2Cast(SpriteData.SpriteSize), Color, Entity->Rotation, 0));
}


internal void
DrawUISprite(gl_render_data     *RenderData,
             static_sprite_data  SpriteData,
             vec2                Position,
             vec2                RenderSize,
             vec4                Color,
             real32              Rotation,
             uint32              TextureIndex)
{
    return(DrawUIQuadTextured(RenderData, 
                              Position, 
                              RenderSize, 
                              SpriteData.AtlasOffset, 
                              SpriteData.SpriteSize, 
                              Rotation, 
                              Color, 
                              TextureIndex));
}

internal void
DrawUIEntity(gl_render_data *RenderData, 
             game_state     *State, 
             entity         *Entity, 
             vec2            Position, 
             vec4            Color)
{
    static_sprite_data SpriteData = State->GameData.Sprites[Entity->Sprite];
    return(DrawUISprite(RenderData, SpriteData, Entity->Position, v2Cast(SpriteData.SpriteSize), Color, Entity->Rotation, 0));
}

internal void
DrawRectXForm(gl_render_data *RenderData, mat4 XForm, vec2 Size, real32 Rotation, vec4 Color)
{
    quad Quad = CreateDrawRect(RenderData, Size, Rotation, Color);
    DrawQuadXForm(RenderData, &Quad, &XForm);
}

internal void
DrawUIRectXForm(gl_render_data *RenderData, mat4 XForm, vec2 Size, real32 Rotation, vec4 Color)
{
    quad Quad = CreateDrawRect(RenderData, Size, Rotation, Color);
    DrawUIQuadXForm(RenderData, &Quad, &XForm);
}

internal void
DrawUISpriteXForm(gl_render_data    *RenderData, 
                  mat4               XForm, 
                  static_sprite_data Sprite,
                  real32             Rotation, 
                  vec4               Color)
{
    quad Quad = CreateDrawQuad(RenderData, 
                               {0, 0}, 
                               v2Cast(Sprite.SpriteSize), 
                               Sprite.SpriteSize, 
                               Sprite.AtlasOffset, 
                               Rotation, 
                               Color, 
                               0); 
    DrawUIQuadXForm(RenderData, &Quad, &XForm);
}

internal void
DrawSpriteXForm(gl_render_data    *RenderData, 
                mat4               XForm, 
                static_sprite_data Sprite,
                real32             Rotation, 
                vec4               Color)
{
    quad Quad = CreateDrawQuad(RenderData, 
                               {0, 0}, 
                               v2Cast(Sprite.SpriteSize), 
                               Sprite.SpriteSize, 
                               Sprite.AtlasOffset, 
                               Rotation, 
                               Color, 
                               0); 
    DrawQuadXForm(RenderData, &Quad, &XForm);
}
