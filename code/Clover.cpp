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

global_variable real32 FPSAccumulator = 0;
global_variable int32 FPSF = 0;

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

internal void
DrawTexturedQuad(gl_render_data *RenderData, 
                            vec2 Position, 
                            vec2 Size, 
                           ivec2 AtlasOffset, 
                           ivec2 SpriteSize, 
                            vec4 Color, 
                          uint32 TextureIndex)
{
    if(RenderData->DrawFrame.QuadCount >= MAX_QUADS)
    {
        CloverRender(RenderData);
    }

    real32 X      = Position.X;
    real32 Y      = Position.Y;
    real32 Width  = Size.X;
    real32 Height = Size.Y;

    real32 OffsetX = (real32)AtlasOffset.X;
    real32 OffsetY = (real32)AtlasOffset.Y;
    real32 SizeX   = (real32)SpriteSize.X;
    real32 SizeY   = (real32)SpriteSize.Y;
    // NOTE(Sleepster): We do the UV's backwards on the Y axis here because the image will be filpped otherwise.

    // TOP LEFT
    RenderData->DrawFrame.VertexBufferptr->Position      = {X, Y, 0};
    RenderData->DrawFrame.VertexBufferptr->TextureCoords = {OffsetX, OffsetY + SizeY};
    RenderData->DrawFrame.VertexBufferptr->MatColor      = Color;
    RenderData->DrawFrame.VertexBufferptr->TextureIndex  = (real32)TextureIndex;
    RenderData->DrawFrame.VertexBufferptr++;

    // TOP RIGHT
    RenderData->DrawFrame.VertexBufferptr->Position      = {X + Width, Y, 0};
    RenderData->DrawFrame.VertexBufferptr->TextureCoords = {OffsetX + SizeX, OffsetY + SizeY};
    RenderData->DrawFrame.VertexBufferptr->MatColor      = Color;
    RenderData->DrawFrame.VertexBufferptr->TextureIndex  = (real32)TextureIndex;
    RenderData->DrawFrame.VertexBufferptr++;

    //BOTTOM RIGHT
    RenderData->DrawFrame.VertexBufferptr->Position      = {X + Width, Y + Height, 0};
    RenderData->DrawFrame.VertexBufferptr->TextureCoords = {OffsetX + SizeX, OffsetY};
    RenderData->DrawFrame.VertexBufferptr->MatColor      = Color;
    RenderData->DrawFrame.VertexBufferptr->TextureIndex  = (real32)TextureIndex;
    RenderData->DrawFrame.VertexBufferptr++;

    //BOTTOM LEFT
    RenderData->DrawFrame.VertexBufferptr->Position      = {X, Y + Height, 0};
    RenderData->DrawFrame.VertexBufferptr->TextureCoords = {OffsetX, OffsetY};
    RenderData->DrawFrame.VertexBufferptr->MatColor      = Color;
    RenderData->DrawFrame.VertexBufferptr->TextureIndex  = (real32)TextureIndex;
    RenderData->DrawFrame.VertexBufferptr++;

    RenderData->DrawFrame.QuadCount++;
}

internal void
DrawQuad(gl_render_data *RenderData, vec2 Position, vec2 Size, vec4 Color)
{
    if(RenderData->DrawFrame.QuadCount >= MAX_QUADS)
    {
        CloverRender(RenderData);
    }

    real32 X      = Position.X;
    real32 Y      = Position.Y;
    real32 Width  = Size.X;
    real32 Height = Size.Y;

    // TOP LEFT
    RenderData->DrawFrame.VertexBufferptr->Position      = {X, Y, 0};
    RenderData->DrawFrame.VertexBufferptr->TextureCoords = {0, 0};
    RenderData->DrawFrame.VertexBufferptr->MatColor      = Color;
    RenderData->DrawFrame.VertexBufferptr->TextureIndex  = 0;
    RenderData->DrawFrame.VertexBufferptr++;

    // TOP RIGHT
    RenderData->DrawFrame.VertexBufferptr->Position      = {X + Width, Y, 0};
    RenderData->DrawFrame.VertexBufferptr->TextureCoords = {0, 0};
    RenderData->DrawFrame.VertexBufferptr->MatColor      = Color;
    RenderData->DrawFrame.VertexBufferptr->TextureIndex  = 0;
    RenderData->DrawFrame.VertexBufferptr++;

    //BOTTOM RIGHT
    RenderData->DrawFrame.VertexBufferptr->Position      = {X + Width, Y + Height, 0};
    RenderData->DrawFrame.VertexBufferptr->TextureCoords = {0, 0};
    RenderData->DrawFrame.VertexBufferptr->MatColor      = Color;
    RenderData->DrawFrame.VertexBufferptr->TextureIndex  = 0;
    RenderData->DrawFrame.VertexBufferptr++;

    //BOTTOM LEFT
    RenderData->DrawFrame.VertexBufferptr->Position      = {X, Y + Height, 0};
    RenderData->DrawFrame.VertexBufferptr->TextureCoords = {0, 0};
    RenderData->DrawFrame.VertexBufferptr->MatColor      = Color;
    RenderData->DrawFrame.VertexBufferptr->TextureIndex  = 0;
    RenderData->DrawFrame.VertexBufferptr++;

    RenderData->DrawFrame.QuadCount++;
}

internal inline void
LoadSpriteData(gl_render_data *RenderData)
{
    RenderData->GameData.Sprites[SPRITE_Nil]    = {.AtlasOffset = {0,  0}, .SpriteSize = {16, 16}};
    RenderData->GameData.Sprites[SPRITE_Player] = {.AtlasOffset = {16, 0}, .SpriteSize = {12, 11}};
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
                      vec2    Position, 
                      vec2    RenderSize, 
                      vec4    Color, 
                    uint32    TextureIndex)
{    
    return(DrawTexturedQuad(RenderData, Position, RenderSize, SpriteData.AtlasOffset, SpriteData.SpriteSize, Color, TextureIndex));
}

internal void
DrawEntity(gl_render_data *RenderData, entity *Entity, vec4 Color)
{
    static_sprite_data SpriteData = RenderData->GameData.Sprites[Entity->Sprite];
    return(DrawSprite(RenderData, SpriteData, Entity->Position + v2Cast(SpriteData.SpriteSize) * 0.5f, v2Cast(SpriteData.SpriteSize), Color, 0));
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
        vec2 RenderScale   = {Glyph.GlyphSize.X * FontScale, (real32)Glyph.GlyphSize.Y};
        ivec2 AtlasOffset   = Glyph.GlyphUVs;
        ivec2 GlyphSize     = Glyph.GlyphSize;

        DrawTexturedQuad(RenderData, WorldPosition, RenderScale, AtlasOffset, GlyphSize, Color, 1);
        Position.X += Glyph.GlyphAdvance.X;
    }
}

internal void
DrawImGui(gl_render_data *RenderData, time Time)
{
    FPSAccumulator += Time.Delta;

    if(FPSAccumulator >= Time.Delta * 2000)
    {
        FPSF = int32(PerfCountFrequency / DeltaCounter);
        FPSAccumulator = 0;
    }
    
    ImGui::Begin("Render Quad Color Picker");
    ImGui::Text("Famerate: %i", FPSF);
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
    PlayerIn->Acceleration = {};
    if(IsGameKeyDown(MOVE_UP, &State->GameInput))
    {
        PlayerIn->Acceleration.Y += 1.0f;
    }
    else if(IsGameKeyDown(MOVE_DOWN, &State->GameInput))
    {
        PlayerIn->Acceleration.Y -= 1.0f;
    }

    if(IsGameKeyDown(MOVE_LEFT, &State->GameInput))
    {
        PlayerIn->Acceleration.X -= 1.0f;
    }
    else if(IsGameKeyDown(MOVE_RIGHT, &State->GameInput))
    {
        PlayerIn->Acceleration.X += 1.0f;
    }

    v2Normalize(PlayerIn->Acceleration);
    vec2 OldPlayerP = PlayerIn->Position;
    real32 Friction = 0.9f;

    vec2 NextPos = {PlayerIn->Position.X + (PlayerIn->Position.X - OldPlayerP.X) + (PlayerIn->Speed * PlayerIn->Acceleration.X) * Square(Time.Delta),
                    PlayerIn->Position.Y + (PlayerIn->Position.Y - OldPlayerP.Y) + (PlayerIn->Speed * PlayerIn->Acceleration.Y) * Square(Time.Delta)};
    PlayerIn->Position = v2Lerp(NextPos, Time.Delta, OldPlayerP);
}

internal void
SetupPlayer(entity *Entity)
{
   Entity->Archetype = PLAYER;
   Entity->Sprite    = SPRITE_Player; 
   Entity->Flags    += IS_ACTIVE|IS_ACTOR;
   Entity->Position = {};
   Entity->Speed    = 400.0f;              // PIXELS PER SECOND
}

internal void
SetupRock(entity *Entity)
{
   Entity->Archetype = ROCK;
   Entity->Sprite    = SPRITE_Rock; 
   Entity->Flags    += IS_ACTIVE|IS_SOLID;
   Entity->Position  = {};
   Entity->Speed     = 1.0f;
}

internal void
GameOnAwake(game_memory *Memory, gl_render_data *RenderData, game_state *State)
{
    LoadSpriteData(RenderData);
//    PlaySound(&Memory->TemporaryStorage, State, STR("boop"), 1);
//    PlayTrackFromDisk(&Memory->TemporaryStorage, State, STR("Test"), 0.5f);

    Player = CreateEntity(State);
    SetupPlayer(Player);

    for(uint32 Index = 0;
        Index < 10;
        ++Index)
    {
        entity *En = CreateEntity(State);
        SetupRock(En);
        En->Position = vec2{GetRandomReal32_Range(-WORLD_SIZE, WORLD_SIZE), GetRandomReal32_Range(-WORLD_SIZE, WORLD_SIZE)};
    }
}

internal void
FixedUpdate(game_memory *Memory, gl_render_data *RenderData, game_state *State, time Time)
{
    for(uint32 EntityIndex = 0;
        EntityIndex < State->World.EntityCounter;
        ++EntityIndex)
    {
        entity *Temp = &State->World.Entities[EntityIndex];
        if((Temp->Flags & IS_VALID) && (Temp->Archetype == PLAYER))
        {
            HandleInput(State, Temp, Time);
            RenderData->GameCamera.Position = vec2{Temp->Position.X, Temp->Position.Y};
        }
    }
}

void PrintMatrix(const mat4& Matrix) {
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            printm("%f ", Matrix.Elements[i][j]);
        }
        printm("\n");
    }
    printm("\n");
}


internal void
UpdateGame(game_memory *Memory, gl_render_data *RenderData, game_state *State, time Time)
{
    DrawImGui(RenderData, Time);

    HandleLoadedSounds(State);
    HandleLoadedTracks(State);

    RenderData->GameCamera.Zoom = 5.3f;

    RenderData->GameCamera.ViewMatrix            = mat4Identity(1.0f);
    mat4 ScaleMatrix                             = mat4MakeScale(vec3{1.0f / RenderData->GameCamera.Zoom, 1.0f / RenderData->GameCamera.Zoom, 1.0f});
    mat4 TranslationMatrix                       = mat4Translate(v2Expand(Player->Position, 0.0f));

    RenderData->GameCamera.ViewMatrix            = mat4Multiply(ScaleMatrix, RenderData->GameCamera.ViewMatrix);
    RenderData->GameCamera.ViewMatrix            = mat4Multiply(TranslationMatrix, RenderData->GameCamera.ViewMatrix);
    RenderData->GameCamera.ViewMatrix            = mat4Inverse(RenderData->GameCamera.ViewMatrix);
    
    RenderData->GameCamera.ProjectionMatrix      = mat4RHGLOrtho((real32)SizeData.Width * -0.5f, (real32)SizeData.Width * 0.5f, (real32)SizeData.Height * -0.5f, (real32)SizeData.Height * 0.5f, -1.0f, 1.0f); 
    RenderData->GameCamera.ProjectionViewMatrix  = mat4Multiply(RenderData->GameCamera.ProjectionMatrix, RenderData->GameCamera.ViewMatrix);

    for(uint32 EntityIndex = 0;
        EntityIndex < State->World.EntityCounter;
        ++EntityIndex)
    {
        entity *Temp = &State->World.Entities[EntityIndex];
        if(Temp->Flags & IS_VALID)
        {
            switch(Temp->Archetype)
            {
                case PLAYER:
                {
                    DrawEntity(RenderData, Temp, WHITE);
                }break;
                case ROCK:
                {
                    DrawEntity(RenderData, Temp, WHITE);
                }break;
            }
        }
    }
    DrawQuad(RenderData, vec2{100, 100}, vec2{16, 16}, RED);
    DrawGameText(RenderData, STR("The Quick Brown fox jumped over the fence"), {-400, 0}, 0.25f, UBUNTU_MONO, BLACK);
}
