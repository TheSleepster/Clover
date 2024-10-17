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

#include "shader/CommonShader.glh"
#include "Clover_Globals.h"
#include "Win32_Clover.h"

// RENDERING INTERFACE FUNCTIONS

internal void
DrawImGui(game_state *State, gl_render_data *RenderData, time Time)
{
    if(State->DrawDebug)
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
    }
}

// TODO(Sleepster): Perhaps add TextureIndex into the static_sprite_data Struct?
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
        Quad.Elements[Index].DrawColor     = Color;
        Quad.Elements[Index].TextureIndex  = TextureIndex;
    }
    
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
    
    for(int32 Index = 0;
        Index < 4;
        ++Index)
    {
        Quad.Elements[Index].DrawColor    = Color;
        Quad.Elements[Index].TextureIndex = 0;
    }
    
    return(Quad);
}

internal quad *
DrawQuadXForm(gl_render_data *RenderData, quad *Quad, mat4 *Transform, bool IsFont)
{
    if(RenderData->DrawFrame.TotalQuadCount >= MAX_QUADS * 0.5f)
    {
        RenderData->CloverRender(RenderData);
    }
    
    Quad->Elements[0].Position = vec4{-0.5f,  0.5f, 0.0f, 1.0f};
    Quad->Elements[1].Position = vec4{ 0.5f,  0.5f, 0.0f, 1.0f};
    Quad->Elements[2].Position = vec4{ 0.5f, -0.5f, 0.0f, 1.0f};
    Quad->Elements[3].Position = vec4{-0.5f, -0.5f, 0.0f, 1.0f};
    
    Quad->Elements[0].Position = mat4Transform(*Transform, Quad->Elements[0].Position);
    Quad->Elements[1].Position = mat4Transform(*Transform, Quad->Elements[1].Position);
    Quad->Elements[2].Position = mat4Transform(*Transform, Quad->Elements[2].Position);
    Quad->Elements[3].Position = mat4Transform(*Transform, Quad->Elements[3].Position);

    vec3 Normals = {0, 0, 1};

    Quad->TopLeft.VertexNormals     = mat4Transform(*Transform, v3Expand(Normals, 1.0)).XYZ; 
    Quad->TopRight.VertexNormals    = mat4Transform(*Transform, v3Expand(Normals, 1.0)).XYZ;
    Quad->BottomLeft.VertexNormals  = mat4Transform(*Transform, v3Expand(Normals, 1.0)).XYZ;
    Quad->BottomRight.VertexNormals = mat4Transform(*Transform, v3Expand(Normals, 1.0)).XYZ;
    
    vertex **VertexBufferptr;
    uint32  *ElementCounter;
    
    bool IsOpaque = (Quad->DrawColor.A == 1.0f && !IsFont);
    if(IsOpaque)
    {
        VertexBufferptr = &RenderData->DrawFrame.VertexBufferptr;
        ElementCounter  = &RenderData->DrawFrame.OpaqueQuadCount;
    }
    else
    {
        VertexBufferptr = &RenderData->DrawFrame.TransparentVertexBufferptr;
        ElementCounter  = &RenderData->DrawFrame.TransparentQuadCount;
    }
    
    // TOP LEFT
    (*VertexBufferptr)->Position      = Quad->Elements[0].Position;
    (*VertexBufferptr)->TextureCoords = Quad->TopLeft.TextureCoords;
    (*VertexBufferptr)->DrawColor     = Quad->DrawColor;
    (*VertexBufferptr)->TextureIndex  = Quad->TextureIndex;
    (*VertexBufferptr)++;
    
    
    (*VertexBufferptr)->Position      = Quad->Elements[1].Position;
    (*VertexBufferptr)->TextureCoords = Quad->TopRight.TextureCoords;
    (*VertexBufferptr)->DrawColor     = Quad->DrawColor;
    (*VertexBufferptr)->TextureIndex  = Quad->TextureIndex;
    (*VertexBufferptr)++;
    
    
    (*VertexBufferptr)->Position      = Quad->Elements[2].Position;
    (*VertexBufferptr)->TextureCoords = Quad->BottomRight.TextureCoords;
    (*VertexBufferptr)->DrawColor     = Quad->DrawColor;
    (*VertexBufferptr)->TextureIndex  = Quad->TextureIndex;
    (*VertexBufferptr)++;
    
    
    (*VertexBufferptr)->Position      = Quad->Elements[3].Position;
    (*VertexBufferptr)->TextureCoords = Quad->BottomLeft.TextureCoords;
    (*VertexBufferptr)->DrawColor     = Quad->DrawColor;
    (*VertexBufferptr)->TextureIndex  = Quad->TextureIndex;
    (*VertexBufferptr)++;
    
    (*ElementCounter)++;
    RenderData->DrawFrame.TotalQuadCount++;
    return(Quad);
}


internal quad *
DrawUIQuadXForm(gl_render_data *RenderData, quad *Quad, mat4 *Transform, bool IsFont)
{
    if(RenderData->DrawFrame.TotalUIElementCount >= MAX_QUADS * 0.5f)
    {
        RenderData->CloverRender(RenderData);
    }
    
    Quad->Elements[0].Position = vec4{-0.5f,  0.5f, 0.0f, 1.0f};
    Quad->Elements[1].Position = vec4{ 0.5f,  0.5f, 0.0f, 1.0f};
    Quad->Elements[2].Position = vec4{ 0.5f, -0.5f, 0.0f, 1.0f};
    Quad->Elements[3].Position = vec4{-0.5f, -0.5f, 0.0f, 1.0f};
    
    Quad->Elements[0].Position = mat4Transform(*Transform, Quad->Elements[0].Position);
    Quad->Elements[1].Position = mat4Transform(*Transform, Quad->Elements[1].Position);
    Quad->Elements[2].Position = mat4Transform(*Transform, Quad->Elements[2].Position);
    Quad->Elements[3].Position = mat4Transform(*Transform, Quad->Elements[3].Position);

    vec3 Normals = {0, 0, 1};

    Quad->TopLeft.VertexNormals     = v4Normalize(mat4Transform(*Transform, v3Expand(Normals, 1.0))).XYZ; 
    Quad->TopRight.VertexNormals    = v4Normalize(mat4Transform(*Transform, v3Expand(Normals, 1.0))).XYZ;
    Quad->BottomLeft.VertexNormals  = v4Normalize(mat4Transform(*Transform, v3Expand(Normals, 1.0))).XYZ;
    Quad->BottomRight.VertexNormals = v4Normalize(mat4Transform(*Transform, v3Expand(Normals, 1.0))).XYZ;

    
    vertex **UIVertexBufferptr;
    uint32  *ElementCounter;
    
    bool IsOpaque = (Quad->DrawColor.A == 1.0f && !IsFont);
    if(IsOpaque)
    {
        UIVertexBufferptr = &RenderData->DrawFrame.UIVertexBufferptr;
        ElementCounter    = &RenderData->DrawFrame.OpaqueUIElementCount;
    }
    else
    {
        UIVertexBufferptr = &RenderData->DrawFrame.TransparentUIVertexBufferptr;
        ElementCounter    = &RenderData->DrawFrame.TransparentUIElementCount;
    }
    
    // TOP LEFT
    (*UIVertexBufferptr)->Position      = Quad->Elements[0].Position;
    (*UIVertexBufferptr)->TextureCoords = Quad->TopLeft.TextureCoords;
    (*UIVertexBufferptr)->DrawColor     = Quad->DrawColor;
    (*UIVertexBufferptr)->TextureIndex  = Quad->TextureIndex;
    (*UIVertexBufferptr)++;
    
    
    (*UIVertexBufferptr)->Position      = Quad->Elements[1].Position;
    (*UIVertexBufferptr)->TextureCoords = Quad->TopRight.TextureCoords;
    (*UIVertexBufferptr)->DrawColor     = Quad->DrawColor;
    (*UIVertexBufferptr)->TextureIndex  = Quad->TextureIndex;
    (*UIVertexBufferptr)++;
    
    
    (*UIVertexBufferptr)->Position      = Quad->Elements[2].Position;
    (*UIVertexBufferptr)->TextureCoords = Quad->BottomRight.TextureCoords;
    (*UIVertexBufferptr)->DrawColor     = Quad->DrawColor;
    (*UIVertexBufferptr)->TextureIndex  = Quad->TextureIndex;
    (*UIVertexBufferptr)++;
    
    
    (*UIVertexBufferptr)->Position      = Quad->Elements[3].Position;
    (*UIVertexBufferptr)->TextureCoords = Quad->BottomLeft.TextureCoords;
    (*UIVertexBufferptr)->DrawColor     = Quad->DrawColor;
    (*UIVertexBufferptr)->TextureIndex  = Quad->TextureIndex;
    (*UIVertexBufferptr)++;
    
    (*ElementCounter)++;
    RenderData->DrawFrame.TotalUIElementCount++;
    return(Quad);
}

internal quad *
DrawQuadProjected(gl_render_data *RenderData, quad *Quad, bool IsFont)
{
    mat4 Translation = mat4Multiply(mat4Identity(1.0f), mat4Translate(v2Expand(vec2{Quad->Position.X, Quad->Position.Y + (Quad->Size.Y * 0.5f)}, 0.0f)));
    mat4 Rotation    = mat4Multiply(mat4Identity(1.0f), mat4RHRotate(AngleRad(Quad->Rotation), vec3{0.0f, 0.0f, 1.0f}));
    mat4 Scale       = mat4MakeScale(v2Expand(Quad->Size, 1.0f));
    
    mat4 Transform = Translation * Rotation * Scale;
    
    return(DrawQuadXForm(RenderData, Quad, &Transform, IsFont));
}

internal quad *
DrawUIQuadProjected(gl_render_data *RenderData, quad *Quad, bool IsFont)
{
    mat4 Translation = mat4Multiply(mat4Identity(1.0f), mat4Translate(v2Expand(vec2{Quad->Position.X - (Quad->Size.X * 0.5f), Quad->Position.Y + (Quad->Size.Y * 0.5f)}, 0.0f)));
    mat4 Rotation    = mat4Multiply(mat4Identity(1.0f), mat4RHRotate(AngleRad(Quad->Rotation), vec3{0.0f, 0.0f, 1.0f}));
    mat4 Scale       = mat4MakeScale(v2Expand(Quad->Size, 1.0f));
    
    mat4 Transform = Translation * Rotation * Scale;
    
    return(DrawUIQuadXForm(RenderData, Quad, &Transform, IsFont));
}

internal quad*
DrawQuadTextured(gl_render_data *RenderData, 
                 vec2            Position, 
                 vec2            Size, 
                 ivec2           AtlasOffset, 
                 ivec2           SpriteSize, 
                 real32          Rotation,
                 vec4            Color, 
                 uint32          TextureIndex,
                 bool            IsFont)
{
    quad Quad = CreateDrawQuad(RenderData, Position, Size, SpriteSize, AtlasOffset, Rotation, Color, (real32)TextureIndex);
    return(DrawQuadProjected(RenderData, &Quad, IsFont));
}

internal quad*
DrawUIQuadTextured(gl_render_data *RenderData, 
                   vec2            Position, 
                   vec2            Size, 
                   ivec2           AtlasOffset, 
                   ivec2           SpriteSize, 
                   real32          Rotation,
                   vec4            Color, 
                   uint32          TextureIndex,
                   bool            IsFont)
{
    quad Quad = CreateDrawQuad(RenderData, Position, Size, SpriteSize, AtlasOffset, Rotation, Color, (real32)TextureIndex);
    return(DrawUIQuadProjected(RenderData, &Quad, IsFont));
}

// TODO(Sleepster): Perhaps make it where we have solids, and actors. Solids will be placed without matrix transforms.
//                  STATIC SOLIDS if you would. Actors will be Dynamic and will require matrix calculations.
internal quad*
DrawQuad(gl_render_data *RenderData, vec2 Position, vec2 Size, real32 Rotation, vec4 Color, bool IsFont)
{
    quad Quad = CreateDrawQuad(RenderData, Position, Size, ivec2{16, 16}, ivec2{0, 0}, Rotation, Color, 1);
    return(DrawQuadProjected(RenderData, &Quad, IsFont));
}

internal quad*
DrawUIQuad(gl_render_data *RenderData, vec2 Position, vec2 Size, real32 Rotation, vec4 Color, bool IsFont)
{
    quad Quad = CreateDrawQuad(RenderData, Position, Size, ivec2{16, 16}, ivec2{0, 0}, Rotation, Color, 1);
    return(DrawUIQuadProjected(RenderData, &Quad, IsFont));
}

internal inline static_sprite_data
GetSprite(game_state *State, sprite_type Sprite)
{
    return(State->GameData.Sprites[Sprite]);
}

internal inline quad *
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
                            TextureIndex,
                            0));
}

internal quad *
DrawEntity(gl_render_data *RenderData, game_state *State, entity *Entity, vec2 Position, vec4 Color)
{
    static_sprite_data SpriteData = State->GameData.Sprites[Entity->Sprite];
    return(DrawSprite(RenderData, SpriteData, Entity->Position, Entity->Size, Color, Entity->Rotation, 1));
}


internal quad *
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
                              TextureIndex,
                              0));
}

internal quad *
DrawUIEntity(gl_render_data *RenderData, 
             game_state     *State, 
             entity         *Entity, 
             vec2            Position, 
             vec4            Color)
{
    static_sprite_data SpriteData = State->GameData.Sprites[Entity->Sprite];
    return(DrawUISprite(RenderData, SpriteData, Entity->Position, v2Cast(SpriteData.SpriteSize), Color, Entity->Rotation, 1));
}

internal void
DrawRectXForm(gl_render_data *RenderData, mat4 XForm, vec2 Size, real32 Rotation, vec4 Color)
{
    quad Quad = CreateDrawRect(RenderData, Size, Rotation, Color);
    DrawQuadXForm(RenderData, &Quad, &XForm, 0);
}

internal void
DrawUIRectXForm(gl_render_data *RenderData, mat4 XForm, vec2 Size, real32 Rotation, vec4 Color)
{
    quad Quad = CreateDrawRect(RenderData, Size, Rotation, Color);
    DrawUIQuadXForm(RenderData, &Quad, &XForm, 0);
}

internal quad*
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
                               1); 
    return(DrawUIQuadXForm(RenderData, &Quad, &XForm, 0));
}

internal quad* 
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
                               1); 
    return(DrawQuadXForm(RenderData, &Quad, &XForm, 0));
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
    real32 TrueScale = FontScale / 100.0f;
    
    for(uint32 StringIndex = 0;
        StringIndex < Text.Length;
        ++StringIndex)
    {
        if(Text.Data[StringIndex] == '\n')    
        {
            Position.Y += RenderData->LoadedFonts[Font].FontHeight * TrueScale;
            Position.X = TextOrigin.X;
            continue;
        }
        
        char C = (Text.Data[StringIndex]);
        font_glyph Glyph = RenderData->LoadedFonts[Font].Glyphs[C];
        
        vec2  RenderScale   = {Glyph.GlyphSize.X * TrueScale, (real32)Glyph.GlyphSize.Y * (TrueScale * 2)};
        ivec2 AtlasOffset   = Glyph.GlyphUVs;
        ivec2 GlyphSize     = Glyph.GlyphSize;
        
        DrawQuadTextured(RenderData, Position, RenderScale, AtlasOffset, GlyphSize, 0.0f, Color, 2, 1);
        Position.X += Glyph.GlyphAdvance.X * TrueScale;
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
    real32 TrueScale = FontScale / 100.0f;
    
    for(uint32 StringIndex = 0;
        StringIndex < Text.Length;
        ++StringIndex)
    {
        if(Text.Data[StringIndex] == '\n')    
        {
            Position.Y += RenderData->LoadedFonts[Font].FontHeight * TrueScale;
            Position.X = TextOrigin.X;
            continue;
        }
        
        char C = (Text.Data[StringIndex]);
        font_glyph Glyph = RenderData->LoadedFonts[Font].Glyphs[C];
        
        vec2 RenderScale    = {Glyph.GlyphSize.X * TrueScale, (real32)Glyph.GlyphSize.Y * (TrueScale * 2)};
        ivec2 AtlasOffset   = Glyph.GlyphUVs;
        ivec2 GlyphSize     = Glyph.GlyphSize;
        
        DrawUIQuadTextured(RenderData, Position, RenderScale, AtlasOffset, GlyphSize, 0.0f, Color, 2, 1);
        Position.X += Glyph.GlyphAdvance.X * TrueScale;
    }
}

internal point_light*
CreatePointLight(gl_render_data   *RenderData,
                 vec2              Position, // THIS IS IN WORLDSPACE POSITION
                 real32            Strength,
                 real32            Radius, 
                 attenuation_data *Attenuation,
                 vec4              Color)
{   
    point_light *Light    = &RenderData->DrawFrame.PointLights[RenderData->DrawFrame.PointLightCount++];
    
    mat4 Translation  = mat4Identity(1.0);
         Translation  = mat4Multiply(Translation, RenderData->GameCamera.ProjectionMatrix);
         Translation  = mat4Multiply(Translation, RenderData->GameCamera.ViewMatrix);
         Translation  = mat4Translation(Translation, v2Expand(Position, 0.0));
    vec3 TruePosition = Translation.Columns[3].XYZ;
    
    Light->Position    = TruePosition;
    Light->Radius      = Radius;
    Light->Strength    = Strength;
    Light->LightColor  = Color;
    Light->Attenuation = *Attenuation;

    return(Light);
}
