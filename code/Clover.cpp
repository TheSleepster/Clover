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
#include "Clover_Draw.cpp"

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

internal inline void
LoadSpriteData(gl_render_data *RenderData)
{
    RenderData->GameData.Sprites[SPRITE_Nil]    = {.AtlasOffset = {0,  0}, .SpriteSize = {16, 16}};
    RenderData->GameData.Sprites[SPRITE_Player] = {.AtlasOffset = {17, 0}, .SpriteSize = {12, 11}};
    RenderData->GameData.Sprites[SPRITE_Rock]   = {.AtlasOffset = {32, 0}, .SpriteSize = {12, 8} };
    RenderData->GameData.Sprites[SPRITE_Tree00] = {.AtlasOffset = {48, 0}, .SpriteSize = {12, 12}};
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
    
    vec2 OldPlayerP = PlayerIn->Position;
    
    vec2 NextPos = {PlayerIn->Position.X + (PlayerIn->Position.X - OldPlayerP.X) + (PlayerIn->Speed * InputAxis.X) * (Time.Delta),
        PlayerIn->Position.Y + (PlayerIn->Position.Y - OldPlayerP.Y) + (PlayerIn->Speed * InputAxis.Y) * (Time.Delta)};
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
    Entity->Speed    = 100.0f;              // PIXELS PER SECOND
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
SetupTree00(entity *Entity)
{
    Entity->Archetype = TREE00;
    Entity->Sprite    = SPRITE_Tree00; 
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
    
    // TODO(Sleepster): Write a proper implementation of Mini Audio's low level API so that 
    //                  hotreloading the engine doesn't just crash the program
    
    //PlaySound(&Memory->TemporaryStorage, State, STR("boop.wav"), 1);
    //PlayTrackFromDisk(&Memory->TemporaryStorage, State, STR("Test.mp3"), 0.5f);
    
    Player = CreateEntity(State);
    SetupPlayer(Player);
    
    for(uint32 EntityIndex = 0;
        EntityIndex < 100;
        ++EntityIndex)
    {
        entity *En = CreateEntity(State);
        SetupRock(En);
        En->Position = vec2{GetRandomReal32_Range(-WORLD_SIZE * 10, WORLD_SIZE * 10), GetRandomReal32_Range(-WORLD_SIZE * 20, WORLD_SIZE * 20)};
        
        entity *En2 = CreateEntity(State);
        SetupTree00(En2);
        En2->Position = vec2{GetRandomReal32_Range(-WORLD_SIZE * 10, WORLD_SIZE * 10), GetRandomReal32_Range(-WORLD_SIZE * 20, WORLD_SIZE * 20)};
    }
    
}

extern
GAME_UPDATE_AND_DRAW(GameUpdateAndRender)
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
    
    // TODO(Sleepster): Perhaps make it where we have solids, and actors. Solids will be placed without matrix transforms.
    //                  STATIC SOLIDS if you would. Actors will be Dynamic and will require matrix calculations.
    for(uint32 EntityIndex = 0;
        EntityIndex <= State->World.EntityCounter;
        ++EntityIndex)
    {
        entity *Temp = &State->World.Entities[EntityIndex];
        if(Temp->Flags & IS_VALID)
        {
            switch(Temp->Archetype)
            {
                case PLAYER:
                {
                    HandleInput(State, Player, Time);
                    v2Approach(&RenderData->GameCamera.Position, RenderData->GameCamera.Target, 5.0f, Time.Delta);
                    DrawEntity(RenderData, Temp, WHITE);
                }break;
                default:
                {
                    DrawEntity(RenderData, Temp, WHITE);
                }break;
            }
        }
    }
    DrawGameText(RenderData, STR("This is an incredibly long string that I am using to test the engine's font rendering capabities oh god oh fuck oh shit running out of characters can it even render this?"), {-300, -100}, 0.5f, UBUNTU_MONO, BLACK);
}

extern
GAME_FIXED_UPDATE(GameFixedUpdate)
{
}