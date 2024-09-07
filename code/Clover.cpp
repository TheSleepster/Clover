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
LoadSpriteData(game_state *State)
{
    State->GameData.Sprites[SPRITE_Nil]      = {.AtlasOffset = {0,   0}, .SpriteSize = {16, 16}};
    State->GameData.Sprites[SPRITE_Player]   = {.AtlasOffset = {17,  0}, .SpriteSize = {12, 11}};
    State->GameData.Sprites[SPRITE_Rock]     = {.AtlasOffset = {32,  0}, .SpriteSize = {12,  8}};
    State->GameData.Sprites[SPRITE_Pebbles]  = {.AtlasOffset = {32, 16}, .SpriteSize = { 6,  5}};
    
    State->GameData.Sprites[SPRITE_Tree00]   = {.AtlasOffset = {48,  0}, .SpriteSize = {11, 14}};
    State->GameData.Sprites[SPRITE_Branches] = {.AtlasOffset = {48, 16}, .SpriteSize = { 7,  7}};
    
    State->GameData.Sprites[SPRITE_Tree01]   = {.AtlasOffset = {64,  0}, .SpriteSize = { 9, 12}};
    State->GameData.Sprites[SPRITE_Trunk]    = {.AtlasOffset = {64, 16}, .SpriteSize = { 6,  6}};
    
    
}

internal inline static_sprite_data *
GetSprite(game_state *State, sprite_type Sprite)
{
    return(&State->GameData.Sprites[Sprite]);
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
DrawEntity(gl_render_data *RenderData, game_state *State, entity *Entity, vec2 Position, vec4 Color)
{
    static_sprite_data SpriteData = State->GameData.Sprites[Entity->Sprite];
    return(DrawSprite(RenderData, SpriteData, Entity->Position, v2Cast(SpriteData.SpriteSize), Color, Entity->Rotation, 0));
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
    Entity->Archetype   = PLAYER;
    Entity->Sprite      = SPRITE_Player; 
    Entity->Flags      += IS_ACTIVE|IS_ACTOR;
    Entity->Size        = {12, 11};
    Entity->Health      = PlayerHealth;
    Entity->Position    = {};
    Entity->Rotation    = 0;
    Entity->Speed       = 100.0f;              // PIXELS PER SECOND
    Entity->BoxCollider = {};
}

internal void
SetupRock(entity *Entity)
{
    Entity->Archetype   = ROCK;
    Entity->Sprite      = SPRITE_Rock; 
    Entity->Flags      += IS_ACTIVE|IS_SOLID|IS_DESTRUCTABLE;
    Entity->Size        = {12, 8};
    Entity->Health      = RockHealth;
    Entity->Position    = {};
    Entity->Rotation    = 0;
    Entity->Speed       = 1.0f;
    Entity->BoxCollider = {};
}

internal void
SetupTree00(entity *Entity)
{
    Entity->Archetype   = TREE;
    Entity->Sprite      = SPRITE_Tree00; 
    Entity->Flags      += IS_ACTIVE|IS_SOLID|IS_DESTRUCTABLE;
    Entity->Size        = {11, 14};
    Entity->Health      = TreeHealth;
    Entity->Position    = {};
    Entity->Rotation    = 0;
    Entity->Speed       = 1.0f;
    Entity->BoxCollider = {};
}

internal void
SetupItemPebbles(entity *Entity)
{
    Entity->Archetype = ITEM;
    Entity->Sprite    = SPRITE_Pebbles;
    Entity->Flags    += IS_ACTIVE|IS_ITEM;
    Entity->Size      = {6, 5};
    Entity->ItemID    = ITEM_Pebbles;
}


internal void
SetupItemBranches(entity *Entity)
{
    Entity->Archetype = ITEM;
    Entity->Sprite    = SPRITE_Branches;
    Entity->Flags    += IS_ACTIVE|IS_ITEM;
    Entity->Size      = {7, 7};
    Entity->ItemID    = ITEM_Branches;
}

internal void
SetupItemTrunk(entity *Entity)
{
    Entity->Archetype = ITEM;
    Entity->Sprite    = SPRITE_Trunk;
    Entity->Flags    += IS_ACTIVE|IS_ITEM;
    Entity->Size      = {6, 6};
    Entity->ItemID    = ITEM_Trunk;
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
        State->GameData.Sprites[i] = {};
    }
}

internal vec2
ConvertMouseToWorldPos(gl_render_data *RenderData, ivec2 MousePos, ivec4 WindowSizeData)
{
    vec2 TransformedMousePos  = {};
    
    vec2 NDC = {(MousePos.X / (WindowSizeData.Width * 0.5f)) - 1.0f, 1.0f - (MousePos.Y / (WindowSizeData.Height * 0.5f))};
    vec4 NDCPosition = v2Expand(NDC, 0.0f, 1.0f);
    
    mat4 InverseProjection = mat4Inverse(RenderData->GameCamera.ProjectionMatrix);
    mat4 InverseViewMatrix = mat4Inverse(RenderData->GameCamera.ViewMatrix);
    
    NDCPosition = mat4Transform(InverseProjection, NDCPosition);
    NDCPosition = mat4Transform(InverseViewMatrix, NDCPosition);
    
    TransformedMousePos = {NDCPosition.X, NDCPosition.Y};
    
    return(TransformedMousePos);
}

internal real32
TileToWorldInt32(real32 Tile)
{
    return(Tile * (real32)TILE_SIZE);
}

internal vec2
TileToWorldPos(vec2 TilePos)
{
    vec2 Result = {};
    Result.X = TileToWorldInt32(TilePos.X);
    Result.Y = TileToWorldInt32(TilePos.Y);
    
    return(Result);
}

internal int32
WorldToTileReal32(real32 World)
{
    return(int32(World) / int32(TILE_SIZE));
}

internal vec2
WorldToTilePos(vec2 WorldPosition)
{
    vec2 Result = {};
    Result.X = (real32)WorldToTileReal32(WorldPosition.X);
    Result.Y = (real32)WorldToTileReal32(WorldPosition.Y);
    
    return(Result);
}

internal inline range_v2
CreateRange(vec2 Min, vec2 Max)
{
    range_v2 Result = {};
    Result.Min = Min;
    Result.Max = Max;
    
    return(Result);
}

internal inline range_v2
RangeShift(range_v2 Range, vec2 Shift)
{
    range_v2 NewRange = {};
    NewRange.Min = Range.Min + Shift;
    NewRange.Max = Range.Max + Shift;
    
    return(NewRange);
}

internal inline vec2 
MakeCentered(vec2 Position, vec2 Size)
{
    vec2 Centered = {};
    
    Centered.X = Position.X + (Size.X * 0.5f);
    Centered.Y = Position.Y + (Size.Y * 0.5f);
    
    return(Centered);
}

internal inline range_v2
RangeMakeCentered(vec2 Size)
{
    range_v2 CenteredRange = {};
    
    CenteredRange.Max = Size;
    CenteredRange = RangeShift(CenteredRange, vec2{Size.X * -0.5f, 0.0f});
    
    return(CenteredRange);
}

internal inline vec2
RangeSize(range_v2 Range)
{
    vec2 Size = {};
    
    Size = Range.Min - Range.Max;
    Size.X = fabsf(Size.X);
    Size.Y = fabsf(Size.Y);
    
    return(Size);
}

internal inline bool
IsRangeWithinBounds(range_v2 Bounds, vec2 Test)
{
    return (Test.X >= Bounds.Min.X && Test.X <= Bounds.Max.X && 
            Test.Y >= Bounds.Min.Y && Test.Y <= Bounds.Max.Y);
}

extern
GAME_ON_AWAKE(GameOnAwake)
{
    ResetGame(RenderData, State);
    LoadSpriteData(State);
    
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
        En->Position = TileToWorldPos(WorldToTilePos(En->Position));
        En->Position.Y += En->Size.Y * -0.15f;
        
        entity *En2 = CreateEntity(State);
        SetupTree00(En2);
        En2->Position = vec2{GetRandomReal32_Range(-WORLD_SIZE * 10, WORLD_SIZE * 10), GetRandomReal32_Range(-WORLD_SIZE * 20, WORLD_SIZE * 20)};
        En2->Position = TileToWorldPos(WorldToTilePos(En2->Position));
        En2->Position.Y += En2->Size.Y * 0.15f;
    }
    
}

extern
GAME_UPDATE_AND_DRAW(GameUpdateAndDraw)
{
    DrawImGui(RenderData, Time);
    
    State->World.WorldFrame = {};
    
    HandleLoadedSounds(State);
    HandleLoadedTracks(State);
    
    RenderData->GameCamera.Zoom = 5.3f;
    
    RenderData->GameCamera.ViewMatrix           = mat4Identity(1.0f);
    mat4 ScaleMatrix                            = mat4MakeScale(vec3{1.0f * RenderData->GameCamera.Zoom, 1.0f * RenderData->GameCamera.Zoom, 1.0f});
    mat4 TranslationMatrix                      = mat4Translate(v2Expand(-RenderData->GameCamera.Position, 0.0f));
    
    RenderData->GameCamera.ViewMatrix           = mat4Multiply(TranslationMatrix, RenderData->GameCamera.ViewMatrix);
    RenderData->GameCamera.ViewMatrix           = mat4Multiply(ScaleMatrix, RenderData->GameCamera.ViewMatrix);
    
    RenderData->GameCamera.ProjectionMatrix     = mat4RHGLOrtho((real32)SizeData.Width * -0.5f, (real32)SizeData.Width * 0.5f, (real32)SizeData.Height * -0.5f, (real32)SizeData.Height * 0.5f, -1.0f, 1.0f); 
    RenderData->GameCamera.ProjectionViewMatrix = mat4Multiply(RenderData->GameCamera.ProjectionMatrix, RenderData->GameCamera.ViewMatrix);
    
    vec2 MouseToWorld = ConvertMouseToWorldPos(RenderData, State->GameInput.Keyboard.CurrentMouse, SizeData);
    // TODO(Sleepster): Perhaps make it where we have solids, and actors. Solids will be placed without matrix transforms.
    //                  STATIC SOLIDS if you would. Actors will be Dynamic and will require matrix calculations.
    
    real32 MinimumDistance = 0;
    
    for(uint32 EntityIndex = 0;
        EntityIndex <= State->World.EntityCounter;
        ++EntityIndex)
    {
        entity *Temp = &State->World.Entities[EntityIndex];
        
        static_sprite_data *Sprite = GetSprite(State, Temp->Sprite);
        range_v2 Bounds = RangeMakeCentered(v2Cast(Sprite->SpriteSize));
        Bounds = RangeShift(Bounds, Temp->Position - (Temp->Size * 0.5f));
        Bounds.Min = Bounds.Min - vec2{20, 20};
        Bounds.Max = Bounds.Max + vec2{20, 20};
        
        real32 Distance = fabsf(v2Distance(Temp->Position, MouseToWorld));
        
        if(IsRangeWithinBounds(Bounds, MouseToWorld))
        {
            if(!State->World.WorldFrame.SelectedEntity || (Distance < MinimumDistance))
            {
                State->World.WorldFrame.SelectedEntity = Temp;
                MinimumDistance = Distance;
            }
            
            if(IsGameKeyPressed(ATTACK, &State->GameInput) && (Temp->Flags & IS_DESTRUCTABLE))
            {
                entity *SelectedEntity = State->World.WorldFrame.SelectedEntity;
                --Temp->Health;
                if(Temp->Health <= 0)
                {
                    switch(SelectedEntity->Archetype)
                    {
                        case ROCK:
                        {
                            entity *Pebbles = CreateEntity(State);
                            SetupItemPebbles(Pebbles);
                            Pebbles->Position = SelectedEntity->Position;
                        }break;
                        
                        case TREE:
                        {
                            switch(SelectedEntity->Sprite)
                            {
                                case SPRITE_Tree00:
                                {
                                    entity *Branches = CreateEntity(State);
                                    SetupItemBranches(Branches);
                                    Branches->Position = SelectedEntity->Position;
                                }break;
                                
                                case SPRITE_Tree01:
                                {
                                    entity *Trunk = CreateEntity(State);
                                    SetupItemBranches(Trunk);
                                    Trunk->Position = SelectedEntity->Position;
                                }break;
                                
                                default: { }break;
                            }break;
                            
                            default: { }break;
                        }break;
                    }
                    
                    //PlaySound(&Memory->TemporaryStorage, State, STR("boop.wav"), 1);
                    DeleteEntity(Temp);
                }
            }
        }
    }
    
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
                    HandleInput(State, Temp, Time);
                    RenderData->GameCamera.Target = Temp->Position;
                    v2Approach(&RenderData->GameCamera.Position, RenderData->GameCamera.Target, 5.0f, Time.Delta);
                    DrawEntity(RenderData, State, Temp, Temp->Position, WHITE);
                }break;
                
                default:
                {
                    vec4 Color = WHITE;
                    if(State->World.WorldFrame.SelectedEntity == Temp)
                    {
                        Color = BLUE;
                    }
                    
                    ivec2 EntityPosInt = iv2Cast(Temp->Position);
                    DrawGameText(RenderData, sprints(&Memory->TemporaryStorage, STR("%d, %d"), EntityPosInt.X, EntityPosInt.Y), {Temp->Position.X - 5, Temp->Position.Y - 10}, 0.10f, UBUNTU_MONO, BLACK);
                    DrawEntity(RenderData, State, Temp, Temp->Position, Color);
                }break;
            }
        }
    }
    
    // NOTE(Sleepster): World Text and Quad at {0,0}
    DrawGameText(RenderData, sprints(&Memory->TemporaryStorage, STR("%f, %f"), MouseToWorld.X, MouseToWorld.Y), {-100, 0}, 0.25f, UBUNTU_MONO, BLACK);
    DrawQuad(RenderData, {0, 0}, {10, 10}, 0, vec4{1.0f, 0.0, 0.0, 1.0f});
    
    // NOTE(Sleepster): Draw the Tiles
    ivec2 PlayerOffset = iv2Cast(WorldToTilePos(Player->Position));
    ivec2  TileRadius   = {15, 20};
    
    for(int32 TileX = PlayerOffset.X - TileRadius.X;
        TileX < PlayerOffset.X + TileRadius.Y;
        ++TileX)
    {
        for(int32 TileY = PlayerOffset.Y - TileRadius.Y;
            TileY < PlayerOffset.Y + TileRadius.Y;
            ++TileY)
        {
            if((TileX + (TileY % 2 == 0)) % 2 == 0)
            {
                real32 X = TileX * TILE_SIZE;
                real32 Y = TileY * TILE_SIZE;
                DrawQuad(RenderData, {X, Y - (TILE_SIZE * -0.20f)}, {TILE_SIZE, TILE_SIZE}, 0, DARK_GRAY);
            }
        }
    }
}

extern
GAME_FIXED_UPDATE(GameFixedUpdate)
{
}