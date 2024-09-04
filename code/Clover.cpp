#include "Intrinsics.h"

// UTILS
#include "util/Math.h"
#include "util/Array.h"
#include "util/FileIO.h"
#include "util/CustomStrings.h"

// CLOVER HEADERS
#include "Clover.h"
#include "Clover_Globals.h"
#include "Clover_Renderer.h"
#include "Clover_Audio.h"
#include "Clover_Input.h"

// IMGUI IMPl
#include "../data/deps/ImGui/imgui.h"
#include "../data/deps/ImGui/imgui_impl_win32.h"
#include "../data/deps/ImGUI/imgui_impl_opengl3.h"

#include "../data/deps/MiniAudio/miniaudio.h"

#include "Clover_Audio.cpp"

#define WORLD_SIZE 100

// NOTE(Sleepster): Random Number Generation stuff
#define RAND_MAX_64 0xFFFFFFFFFFFFFFFFull
#define MULTIPLIER 6364136223846793005ull
#define INCREMENT 1442695040888963407ull 


global_variable entity *Player = {};


internal inline uint64 
GetRandom()
{
    // NOTE(Sleepster): Look Ma, the only local_perist(s) in the entire program!
    local_persist uint64 rng_state = 1;
    uint64_t x = rng_state;
    x ^= x << 13;
    x ^= x >> 7;
    x ^= x << 17;
    rng_state = x;
    return x;
}

internal inline real32
GetRandomReal32()
{
    return((real32)GetRandom()/(real32)UINT64_MAX);
}

internal inline real32
GetRandomReal32_Range(real32 Minimum, real32 Maximum)
{
    return((Maximum - Minimum)*GetRandomReal32() + Minimum);
}

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
    RenderData->DrawFrame.VertexBufferptr->Position = mat4Transform(*Transform, Quad->Elements[1].Position);
    RenderData->DrawFrame.VertexBufferptr->TextureCoords = Quad->TopRight.TextureCoords;
    RenderData->DrawFrame.VertexBufferptr->DrawColor     = Quad->DrawColor;
    RenderData->DrawFrame.VertexBufferptr->TextureIndex  = Quad->TextureIndex;
    RenderData->DrawFrame.VertexBufferptr++;
    
    // BOTTOM RIGHT
    RenderData->DrawFrame.VertexBufferptr->Position = mat4Transform(*Transform, Quad->Elements[2].Position);
    RenderData->DrawFrame.VertexBufferptr->TextureCoords = Quad->BottomRight.TextureCoords;
    RenderData->DrawFrame.VertexBufferptr->DrawColor     = Quad->DrawColor;
    RenderData->DrawFrame.VertexBufferptr->TextureIndex  = Quad->TextureIndex;
    RenderData->DrawFrame.VertexBufferptr++;
    
    // BOTTOM LEFT
    RenderData->DrawFrame.VertexBufferptr->Position = mat4Transform(*Transform, Quad->Elements[3].Position);
    RenderData->DrawFrame.VertexBufferptr->TextureCoords = Quad->BottomLeft.TextureCoords;
    RenderData->DrawFrame.VertexBufferptr->DrawColor     = Quad->DrawColor;
    RenderData->DrawFrame.VertexBufferptr->TextureIndex  = Quad->TextureIndex;
    RenderData->DrawFrame.VertexBufferptr++;
    
    RenderData->DrawFrame.QuadCount++;
}

internal void
DrawQuadProjected(gl_render_data *RenderData, quad *Quad)
{
    mat4 Translation = mat4Multiply(mat4Identity(1.0f), mat4Translate(v2Expand(Quad->Position, 0.0f)));
    mat4 Rotation    = mat4Multiply(mat4Identity(1.0f), mat4RHRotate(AngleRad(Quad->Rotation), vec3{0.0f, 0.0f, 1.0f}));
    mat4 Scale       = mat4MakeScale(v2Expand(Quad->Size, 1.0f));
    
    mat4 Transform = Translation * Rotation * Scale;
    
    return(DrawQuadXForm(RenderData, Quad, &Transform));
}

internal void
DrawQuad(gl_render_data *RenderData, vec2 Position, vec2 Size, real32 Rotation, vec4 Color)
{
    quad Quad = CreateDrawQuad(RenderData, Position, Size, ivec2{16, 16}, ivec2{0, 0}, Rotation, Color, 0);
    return(DrawQuadProjected(RenderData, &Quad));
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

internal inline void
LoadSpriteData(gl_render_data *RenderData)
{
    RenderData->GameData.Sprites[SPRITE_Nil]    = {.AtlasOffset = {0,  0}, .SpriteSize = {16, 16}};
    RenderData->GameData.Sprites[SPRITE_Player] = {.AtlasOffset = {17, 0}, .SpriteSize = {12, 11}};
    RenderData->GameData.Sprites[SPRITE_Rock]   = {.AtlasOffset = {32, 0}, .SpriteSize = {12, 8}};
}

internal inline static_sprite_data *
GetSprite(gl_render_data *RenderData, sprite_type Sprite)
{
    return(&RenderData->GameData.Sprites[Sprite]);
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
    return(DrawQuadTextured(RenderData, Position, RenderSize, SpriteData.AtlasOffset, SpriteData.SpriteSize, Rotation, Color, TextureIndex));
}

internal void
DrawEntity(gl_render_data *RenderData, entity *Entity, vec4 Color)
{
    static_sprite_data SpriteData = RenderData->GameData.Sprites[Entity->Sprite];
    return(DrawSprite(RenderData, SpriteData, Entity->Position + v2Cast(SpriteData.SpriteSize) * 0.5f, v2Cast(SpriteData.SpriteSize), Color, Entity->Rotation, 0));
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
        
        vec2 WorldPosition = {(Position.X + Glyph.GlyphOffset.X) * FontScale, (Position.Y) * FontScale};
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
    ImGui::Separator();
    ImGui::SameLine();
    
    ImGui::Text("Clear Color:");
    ImGui::Separator();
    ImGui::ColorPicker4( "ClearColor", &RenderData->ClearColor.R, ImGuiColorEditFlags_PickerHueWheel);
    ImGui::SameLine();
    ImGui::End();
    //RenderData->GameCamera.Position += vec2{0.001f, 0.0} * Time.Delta;
}

internal entity *
CreateEntity(game_state *State)
{
    entity *Result = {};
    
    for(uint32 EntityIndex = 1;
        EntityIndex < MAX_ENTITIES;
        ++EntityIndex)
    {
        entity *Found = &State->World.Entities[EntityIndex]; 
        if(!(Found->Flags & IS_VALID))
        {
            Result = Found;            
            break;
        }
    }
    Assert(Result);
    
    ++State->World.EntityCounter;
    Result->Flags = IS_VALID;
    return(Result);
}

internal inline void
DeleteEntity(entity *Entity)
{
    memset(Entity, 0, sizeof(struct entity));
}

internal void
HandleInput(game_state *State, entity *PlayerIn, time Time)
{
    vec2 InputAxis = {};
    if(IsGameKeyDown(MOVE_UP, &State->GameInput))
    {
        InputAxis.Y += 1.0f;
    }
    else if(IsGameKeyDown(MOVE_DOWN, &State->GameInput))
    {
        InputAxis.Y -= 1.0f;
    }
    
    if(IsGameKeyDown(MOVE_LEFT, &State->GameInput))
    {
        InputAxis.X -= 1.0f;
    }
    else if(IsGameKeyDown(MOVE_RIGHT, &State->GameInput))
    {
        InputAxis.X += 1.0f;
    }
    
    v2Normalize(InputAxis);
    vec2 OldPlayerP = PlayerIn->Position;
    real32 Friction = 0.9f;
    
    vec2 NextPos = {PlayerIn->Position.X + (PlayerIn->Position.X - OldPlayerP.X) + (PlayerIn->Speed * InputAxis.X) * Square(Time.Delta),
        PlayerIn->Position.Y + (PlayerIn->Position.Y - OldPlayerP.Y) + (PlayerIn->Speed * InputAxis.Y) * Square(Time.Delta)};
    PlayerIn->Position = v2Lerp(NextPos, Time.Delta, OldPlayerP);
}

internal void
SetupPlayer(entity *Entity)
{
    Entity->Archetype = PLAYER;
    Entity->Sprite    = SPRITE_Player; 
    Entity->Flags    += IS_ACTIVE|IS_ACTOR;
    Entity->Position = {};
    Entity->Rotation = 0;
    Entity->Speed    = 400.0f;              // PIXELS PER SECOND
}

internal void
SetupRock(entity *Entity)
{
    Entity->Archetype = ROCK;
    Entity->Sprite    = SPRITE_Rock; 
    Entity->Flags    += IS_ACTIVE|IS_SOLID;
    Entity->Position  = {};
    Entity->Rotation = 0;
    Entity->Speed     = 1.0f;
}

internal void
ResetGame(gl_render_data *RenderData, game_state *State)
{
    for(uint32 i = 0; i < MAX_ENTITIES; i++)
    {
        entity *Temp = &State->World.Entities[i];
        DeleteEntity(Temp);
    }
    
    for(uint32 i = 0; i < SPRITE_Count; i++)
    {
        RenderData->GameData.Sprites[i] = {};
    }
}

extern
GAME_ON_AWAKE(GameOnAwake)
{
    ResetGame(RenderData, State);
    LoadSpriteData(RenderData);
    //PlaySound(&Memory->TemporaryStorage, State, STR("boop.wav"), 1);
    //PlayTrackFromDisk(&Memory->TemporaryStorage, State, STR("Test.mp3"), 0.5f);
    
    Player = CreateEntity(State);
    SetupPlayer(Player);
}

extern
GAME_FIXED_UPDATE(GameFixedUpdate)
{
    for(uint32 EntityIndex = 0;
        EntityIndex <= State->World.EntityCounter;
        ++EntityIndex)
    {
        entity *Temp = &State->World.Entities[EntityIndex];
        if((Temp->Flags & IS_VALID) && (Temp->Archetype == PLAYER))
        {
            HandleInput(State, Temp, Time);
            v2Approach(&RenderData->GameCamera.Position, RenderData->GameCamera.Target, 0.5f, Time.Delta);
        }
    }
}

extern
GAME_UPDATE_AND_RENDER(GameUpdateAndRender)
{
    DrawImGui(RenderData, Time);
    
    HandleLoadedSounds(State);
    HandleLoadedTracks(State);
    
    RenderData->GameCamera.Target = Player->Position;
    RenderData->GameCamera.Zoom = 5.3f;
    
    RenderData->GameCamera.ViewMatrix           = mat4Identity(1.0f);
    mat4 ScaleMatrix                            = mat4MakeScale(vec3{1.0f * RenderData->GameCamera.Zoom, 1.0f * RenderData->GameCamera.Zoom, 1.0f});
    mat4 TranslationMatrix                      = mat4Translate(v2Expand(-RenderData->GameCamera.Position, 0.0f));
    
    RenderData->GameCamera.ViewMatrix           = mat4Multiply(TranslationMatrix, RenderData->GameCamera.ViewMatrix);
    RenderData->GameCamera.ViewMatrix           = mat4Multiply(ScaleMatrix, RenderData->GameCamera.ViewMatrix);
    
    RenderData->GameCamera.ProjectionMatrix     = mat4RHGLOrtho((real32)SizeData.Width * -0.5f, (real32)SizeData.Width * 0.5f, (real32)SizeData.Height * -0.5f, (real32)SizeData.Height * 0.5f, -1.0f, 1.0f); 
    RenderData->GameCamera.ProjectionViewMatrix = mat4Multiply(RenderData->GameCamera.ProjectionMatrix, RenderData->GameCamera.ViewMatrix);
    
    for(uint32 EntityIndex = 0;
        EntityIndex <= State->World.EntityCounter;
        ++EntityIndex)
    {
        entity *Temp = &State->World.Entities[EntityIndex];
        if(Temp->Flags & IS_VALID)
        {
            switch(Temp->Archetype)
            {
                default:
                {
                    DrawEntity(RenderData, Temp, WHITE);
                }break;
            }
        }
    }
    DrawGameText(RenderData, STR("Test!"), {0, -100}, 0.5f, UBUNTU_MONO, BLACK);
    DrawQuad(RenderData, vec2{-10.0f, 10.0f}, vec2{16, 16}, 0.0f, RED);
}
