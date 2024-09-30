#include "Intrinsics.h"

// UTILS
#include "util/Math.h"
#include "util/Array.h"
#include "util/FileIO.h"
#include "util/CustomStrings.h"

// CLOVER HEADERS
#include "Clover.h"
#include "Clover_Globals.h"
#include "Clover_Audio.h"
#include "Clover_Input.h"
#include "Clover_Renderer.h"
#include "shader/ShaderHeader.h"

// IMGUI IMPl
#include "../data/deps/ImGui/imgui.h"
#include "../data/deps/ImGui/imgui_impl_win32.h"
#include "../data/deps/ImGUI/imgui_impl_opengl3.h"

// NOTE(Sleepster): If MiniAudio starts acting weird, I compiled it in a non debug mode.
#include "Clover_Audio.cpp"
#include "Clover_Draw.cpp"
#include "Clover_UI.cpp"


global_variable entity *Player = {};


internal inline uint64
GetRandomSeed()
{
    return(__rdtsc());
}

// NOTE(Sleepster): Random Number Generation stuff
#define RAND_MAX_64 0xFFFFFFFFFFFFFFFFull
#define MULTIPLIER 6364136223846793005ull
#define INCREMENT 1442695040888963407ull

global_variable uint64 RDTSCRandomSeed = GetRandomSeed();

internal inline uint64
PeekRandom()
{
    RDTSCRandomSeed = RDTSCRandomSeed * MULTIPLIER + INCREMENT;
    return(RDTSCRandomSeed);
}

internal inline uint64 
GetRandom(void)
{
    uint64 RandomSeed = PeekRandom();
    return(RandomSeed);
}

internal inline real32
GetRandomReal32(void)
{
    return((real32)GetRandom() / (real32)UINT64_MAX);
}

internal inline real32
GetRandomReal32_Range(real32 Minimum, real32 Maximum)
{
    return((Maximum - Minimum)*GetRandomReal32() + Minimum);
}

internal inline void
LoadSpriteData(game_state *State)
{
    State->GameData.Sprites[SPRITE_Nil]                   = {.AtlasOffset = {  0,  0}, .SpriteSize = {16, 16}};
    State->GameData.Sprites[SPRITE_Player]                = {.AtlasOffset = { 17,  0}, .SpriteSize = {12, 11}};
    State->GameData.Sprites[SPRITE_Rock]                  = {.AtlasOffset = { 32,  0}, .SpriteSize = {12,  8}};
    State->GameData.Sprites[SPRITE_Pebbles]               = {.AtlasOffset = { 32, 16}, .SpriteSize = { 6,  5}};
    State->GameData.Sprites[SPRITE_Tree00]                = {.AtlasOffset = { 48,  0}, .SpriteSize = {11, 14}};
    State->GameData.Sprites[SPRITE_Branches]              = {.AtlasOffset = { 48, 16}, .SpriteSize = { 7,  7}};
    State->GameData.Sprites[SPRITE_Tree01]                = {.AtlasOffset = { 64,  0}, .SpriteSize = { 9, 12}};
    State->GameData.Sprites[SPRITE_Trunk]                 = {.AtlasOffset = { 64, 16}, .SpriteSize = { 6,  6}};
    State->GameData.Sprites[SPRITE_RubyOre]               = {.AtlasOffset = { 80,  0}, .SpriteSize = {14, 11}};
    State->GameData.Sprites[SPRITE_RubyChunk]             = {.AtlasOffset = { 80, 16}, .SpriteSize = { 8,  6}};
    State->GameData.Sprites[SPRITE_SaphireOre]            = {.AtlasOffset = { 96,  0}, .SpriteSize = {11, 11}};
    State->GameData.Sprites[SPRITE_SapphireChunk]         = {.AtlasOffset = { 96, 16}, .SpriteSize = { 8,  6}};
    State->GameData.Sprites[SPRITE_UIItemBox]             = {.AtlasOffset = {112,  0}, .SpriteSize = {16, 16}};
    State->GameData.Sprites[SPRITE_ToolPickaxe]           = {.AtlasOffset = {  0, 48}, .SpriteSize = {11, 13}};
    State->GameData.Sprites[SPRITE_ToolWoodAxe]           = {.AtlasOffset = { 16, 48}, .SpriteSize = {11, 13}};
    State->GameData.Sprites[SPRITE_FullHeartContainer]    = {.AtlasOffset = {  0, 64}, .SpriteSize = {11,  9}};
    State->GameData.Sprites[SPRITE_HalfHeartContainer]    = {.AtlasOffset = { 16, 64}, .SpriteSize = {11,  9}};
    State->GameData.Sprites[SPRITE_EmptyHeartContainer]   = {.AtlasOffset = { 32, 64}, .SpriteSize = {11,  9}};
    State->GameData.Sprites[SPRITE_TestEnemyUnit]         = {.AtlasOffset = { 16, 16}, .SpriteSize = {13, 11}};
    State->GameData.Sprites[SPRITE_SelectionBox]          = {.AtlasOffset = {112, 16}, .SpriteSize = {16, 16}};
}

internal inline void
LoadItemData(game_state *State)
{
    State->GameData.GameItems[ITEM_Nil]              = {};
    State->GameData.GameItems[ITEM_Pebbles]          = {.Archetype = ITEM, .Flags = IS_VALID|IS_ITEM|IS_IN_INVENTORY, .Sprite = SPRITE_Pebbles,       .ItemID = ITEM_Pebbles,          .MaxStackCount = 64, .ItemName = STR("Pebbles"),         .ItemDesc = STR("These are some pebbles!")};
    State->GameData.GameItems[ITEM_Branches]         = {.Archetype = ITEM, .Flags = IS_VALID|IS_ITEM|IS_IN_INVENTORY, .Sprite = SPRITE_Branches,      .ItemID = ITEM_Branches,         .MaxStackCount = 64, .ItemName = STR("Branches"),        .ItemDesc = STR("These are some branches!")};
    State->GameData.GameItems[ITEM_Trunk]            = {.Archetype = ITEM, .Flags = IS_VALID|IS_ITEM|IS_IN_INVENTORY, .Sprite = SPRITE_Trunk,         .ItemID = ITEM_Trunk,            .MaxStackCount = 64, .ItemName = STR("Pine Logs"),       .ItemDesc = STR("This is a bundle of logs!")};
    State->GameData.GameItems[ITEM_RubyOreChunk]     = {.Archetype = ITEM, .Flags = IS_VALID|IS_ITEM|IS_IN_INVENTORY, .Sprite = SPRITE_RubyChunk,     .ItemID = ITEM_RubyOreChunk,     .MaxStackCount = 64, .ItemName = STR("Ruby Chunks"),     .ItemDesc = STR("These are chunks of Ruby Rock!")};
    State->GameData.GameItems[ITEM_SapphireOreChunk] = {.Archetype = ITEM, .Flags = IS_VALID|IS_ITEM|IS_IN_INVENTORY, .Sprite = SPRITE_SapphireChunk, .ItemID = ITEM_SapphireOreChunk, .MaxStackCount = 64, .ItemName = STR("Sapphire Chunks"), .ItemDesc = STR("These are soem chunks of Sapphire Rock!")};
    State->GameData.GameItems[ITEM_ToolPickaxe]      = {.Archetype = ITEM, .Flags = IS_VALID|IS_ITEM|IS_IN_INVENTORY, .Sprite = SPRITE_ToolPickaxe,   .ItemID = ITEM_ToolPickaxe,      .MaxStackCount = 1,  .ItemName = STR("Simple Pickaxe"),  .ItemDesc = STR("This a pickaxe, It can be used to mine ores!")};
    State->GameData.GameItems[ITEM_ToolWoodAxe]      = {.Archetype = ITEM, .Flags = IS_VALID|IS_ITEM|IS_IN_INVENTORY, .Sprite = SPRITE_ToolWoodAxe,   .ItemID = ITEM_ToolWoodAxe,      .MaxStackCount = 1,  .ItemName = STR("Simple Wood Axe"), .ItemDesc = STR("This is a wood axe, It can be used to cut trees!")};
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
    // NOTE(Sleepster): Player Position 
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

    // NOTE(Sleepster): Game Update Stuff 
    if(IsKeyPressed(KEY_ESCAPE, &State->GameInput))
    {
        Player->Inventory.SelectedInventoryItem = {};
    }
    if(IsGameKeyPressed(INVENTORY, &State->GameInput))
    {
        State->DisplayPlayerInventory = !State->DisplayPlayerInventory;
    }
    if(IsGameKeyPressed(SHOW_HOTBAR, &State->GameInput))
    {
        State->DisplayPlayerHotbar = !State->DisplayPlayerHotbar;
    }
    if(IsGameKeyPressed(HOTBAR_01, &State->GameInput))
    {
        if(Player->Inventory.CurrentInventorySlot == 0) 
        {
            Player->Inventory.CurrentInventorySlot = NULLSLOT;
            return;
        }

        Player->Inventory.CurrentInventorySlot = 0;
    }
    if(IsGameKeyPressed(HOTBAR_02, &State->GameInput))
    {
        if(Player->Inventory.CurrentInventorySlot == 1) 
        {
            Player->Inventory.CurrentInventorySlot = NULLSLOT;
            return;
        }
        Player->Inventory.CurrentInventorySlot = 1;
    }
    if(IsGameKeyPressed(HOTBAR_03, &State->GameInput))
    {
        if(Player->Inventory.CurrentInventorySlot == 2) 
        {
            Player->Inventory.CurrentInventorySlot = NULLSLOT;
            return;
        }
        Player->Inventory.CurrentInventorySlot = 2;
    }
    if(IsGameKeyPressed(HOTBAR_04, &State->GameInput))
    {
        if(Player->Inventory.CurrentInventorySlot == 3) 
        {
            Player->Inventory.CurrentInventorySlot = NULLSLOT;
            return;
        }
        Player->Inventory.CurrentInventorySlot = 3;
    }
    if(IsGameKeyPressed(HOTBAR_05, &State->GameInput))
    {
        if(Player->Inventory.CurrentInventorySlot == 4) 
        {
            Player->Inventory.CurrentInventorySlot = NULLSLOT;
            return;
        }
        Player->Inventory.CurrentInventorySlot = 4;
    }
    if(IsGameKeyPressed(HOTBAR_06, &State->GameInput))
    {
        if(Player->Inventory.CurrentInventorySlot == 5) 
        {
            Player->Inventory.CurrentInventorySlot = NULLSLOT;
            return;
        }
        Player->Inventory.CurrentInventorySlot = 5;
    }
    if(IsGameKeyPressed(HOTBAR_07, &State->GameInput))
    {
        if(Player->Inventory.CurrentInventorySlot == 6) 
        {
            Player->Inventory.CurrentInventorySlot = NULLSLOT;
            return;
        }
        Player->Inventory.CurrentInventorySlot = 6;
    }
    if(IsKeyPressed(KEY_HOME, &State->GameInput))
    {
        State->DrawDebug = !State->DrawDebug;
    }
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
    Entity->ItemID      = ITEM_Nil;            // DROPS
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
    
    Entity->ItemID      = ITEM_Pebbles;
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
    
    Entity->ItemID      = ITEM_Branches;
}

internal void
SetupTree01(entity *Entity)
{
    Entity->Archetype   = TREE;
    Entity->Sprite      = SPRITE_Tree01; 
    Entity->Flags      += IS_ACTIVE|IS_SOLID|IS_DESTRUCTABLE;
    Entity->Size        = {9, 12};
    Entity->Health      = TreeHealth;
    Entity->Position    = {};
    Entity->Rotation    = 0;
    Entity->Speed       = 1.0f;
    Entity->BoxCollider = {};
    
    Entity->ItemID      = ITEM_Trunk;
}

internal void
SetupRubyNode(entity *Entity)
{
    Entity->Archetype   = NODE;
    Entity->Sprite      = SPRITE_RubyOre; 
    Entity->Flags      += IS_ACTIVE|IS_SOLID|IS_DESTRUCTABLE;
    Entity->Size        = {14, 11};
    Entity->Health      = NodeHealth;
    Entity->Position    = {};
    Entity->Rotation    = 0;
    Entity->Speed       = 1.0f;
    Entity->BoxCollider = {};
    
    Entity->ItemID      = ITEM_RubyOreChunk;
}

internal void
SetupSaphireNode(entity *Entity)
{
    Entity->Archetype   = NODE;
    Entity->Sprite      = SPRITE_SaphireOre; 
    Entity->Flags      += IS_ACTIVE|IS_SOLID|IS_DESTRUCTABLE;
    Entity->Size        = {11, 11};
    Entity->Health      = NodeHealth;
    Entity->Position    = {};
    Entity->Rotation    = 0;
    Entity->Speed       = 1.0f;
    Entity->BoxCollider = {};
    
    Entity->ItemID      = ITEM_SapphireOreChunk;
}

internal void
SetupItemPebbles(entity *Entity)
{
    Entity->Archetype = ITEM;
    Entity->Sprite    = SPRITE_Pebbles;
    Entity->Flags    += IS_ACTIVE|IS_ITEM|CAN_BE_PICKED_UP;
    Entity->Size      = {6, 5};
    Entity->ItemID    = ITEM_Pebbles;
}

internal void
SetupItemBranches(entity *Entity)
{
    Entity->Archetype = ITEM;
    Entity->Sprite    = SPRITE_Branches;
    Entity->Flags    += IS_ACTIVE|IS_ITEM|CAN_BE_PICKED_UP;
    Entity->Size      = {7, 7};
    Entity->ItemID    = ITEM_Branches;
}

internal void
SetupItemTrunk(entity *Entity)
{
    Entity->Archetype = ITEM;
    Entity->Sprite    = SPRITE_Trunk;
    Entity->Flags    += IS_ACTIVE|IS_ITEM|CAN_BE_PICKED_UP;
    Entity->Size      = {6, 6};
    Entity->ItemID    = ITEM_Trunk;
}

internal void
SetupItemRubyChunk(entity *Entity)
{
    Entity->Archetype = ITEM;
    Entity->Sprite    = SPRITE_RubyChunk;
    Entity->Flags    += IS_ACTIVE|IS_ITEM|CAN_BE_PICKED_UP;
    Entity->Size      = {6, 5};
    Entity->ItemID    = ITEM_RubyOreChunk;
}

internal void
SetupItemSaphireChunk(entity *Entity)
{
    Entity->Archetype = ITEM;
    Entity->Sprite    = SPRITE_SapphireChunk;
    Entity->Flags    += IS_ACTIVE|IS_ITEM|CAN_BE_PICKED_UP;
    Entity->Size      = {6, 5};
    Entity->ItemID    = ITEM_SapphireOreChunk;
}

internal void
SetupItemToolPickaxe(entity *Entity)
{
    Entity->Archetype = ITEM;
    Entity->Sprite    = SPRITE_ToolPickaxe;
    Entity->Flags    += IS_ACTIVE|IS_ITEM|CAN_BE_PICKED_UP;
    Entity->Size      = {11, 13};
    Entity->ItemID    = ITEM_ToolPickaxe;
}

internal void
SetupUIBoxElement(entity *Entity)
{
    Entity->Archetype   = UI;
    Entity->Sprite      = SPRITE_UIItemBox; 
    Entity->Flags      += IS_ACTIVE|IS_UI;
    Entity->Size        = {15, 15};
}

internal inline void
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

internal inline real32
SinBreatheNormalized(real32 Time, real32 Modifier, real32 Min, real32 Max)
{
    real32 SineValue = (sinf(Modifier * 2 * PI32 * Time) + 1.0f) / 2.0f;
    return(Min + (Max - Min) * SineValue);
}

internal inline real32
SinBreathe(real32 Time, real32 Modifier)
{
    return(sinf(Time * Modifier));
}

inline int
CompareEntityYAxis(const void *A, const void *B)
{
    const entity *EntityA = (const entity*)A;
    const entity *EntityB = (const entity*)B;
    
    return((EntityA->Position.Y > EntityB->Position.Y) ? -1 :
           (EntityA->Position.Y < EntityB->Position.Y) ?  1 : 0);
}

internal bool
SwapInventoryItems(entity_item_inventory *Inventory, item *ItemA, item *ItemB)
{   
    item TempItem = *ItemA;
    Inventory->Items[ItemA->OccupiedInventorySlot] = *ItemB;
    Inventory->Items[ItemB->OccupiedInventorySlot] = TempItem;

    return(true);
}

internal void
SetupDroppedEntity(gl_render_data *RenderData, game_state *State, item *Selection, entity *SpawnedItem)
{
    if(Selection->Sprite != SPRITE_Nil)
    {
        switch(Selection->Sprite)
        {
            case SPRITE_Pebbles:
            {
                SetupItemPebbles(SpawnedItem);
            }break;
            case SPRITE_Trunk:
            {
                SetupItemTrunk(SpawnedItem);
            }break;
            case SPRITE_Branches:
            {
                SetupItemBranches(SpawnedItem);
            }break;
            case SPRITE_RubyChunk:
            {
                SetupItemRubyChunk(SpawnedItem);
            }break;
            case SPRITE_SapphireChunk:
            {
                SetupItemSaphireChunk(SpawnedItem);
            }break;
            case SPRITE_ToolPickaxe:
            {
                SetupItemToolPickaxe(SpawnedItem);
            }break;
        }

        SpawnedItem->Flags -= CAN_BE_PICKED_UP;
        SpawnedItem->DroppedItemCount = Selection->CurrentStack;

        vec2 WorldMouseCoords = TransformMouseCoords(RenderData->GameCamera.ViewMatrix, RenderData->GameCamera.ProjectionMatrix, State->GameInput.Keyboard.CurrentMouse, SizeData);
        real32 Distance = fabsf(v2Distance(Player->Position, WorldMouseCoords));
        vec2 Direction = v2Normalize(WorldMouseCoords - Player->Position);

        SpawnedItem->Position = Player->Position;
        if(Distance <= MaxDropDistance)
        {
            vec2 DropPosition = Player->Position + Direction * Distance;
            SpawnedItem->Target = DropPosition;
        }
        else
        {
            vec2 MaxDropPosition = Player->Position + Direction * MaxDropDistance;
            SpawnedItem->Target = MaxDropPosition;
        }
    }
}

internal real32
r32Clamp(real32 Value, real32 Min, real32 Max)
{
    if (Value <= Min) return Min;
    if (Value >= Max) return Max;
    return Value;
}

internal vec2 
v2Clamp(vec2 Value, vec2 Min, vec2 Max)
{
    Value.X = r32Clamp(Value.X, Min.X, Max.X);
    Value.Y = r32Clamp(Value.Y, Min.Y, Max.Y);
    return(Value);
}

internal void
ResetItemSlotState(item *Item)
{
    Item->Sprite = {};
    Item->CurrentStack = 0;
    Item->ItemName = {};
    Item->ItemDesc = {};
    Item->ItemID   = {};
    Item->MaxStackCount = {};
}

extern
GAME_ON_AWAKE(GameOnAwake)
{
    ResetGame(RenderData, State);
    LoadSpriteData(State);
    LoadItemData(State);
    
    // TODO(Sleepster): Write a proper implementation of Mini Audio's low level API so that 
    //                  hotreloading the engine doesn't just crash the program
    
    //PlaySound(&Memory->TemporaryStorage, State, STR("boop.wav"), 1);
    //PlayTrackFromDisk(&Memory->TemporaryStorage, State, STR("Test.mp3"), 0.5f);

    real32 SizeScaler = WORLD_SIZE * 10;
    for(uint32 EntityIndex = 0;
        EntityIndex < 50;
        ++EntityIndex)
    {
        entity *En = CreateEntity(State);
        SetupRock(En);
        En->Position = vec2{GetRandomReal32_Range(-SizeScaler, SizeScaler), GetRandomReal32_Range(-SizeScaler, SizeScaler)};
        En->Position = TileToWorldPos(WorldToTilePos(En->Position));
        
        
        entity *En2 = CreateEntity(State);
        SetupTree00(En2);
        En2->Position = vec2{GetRandomReal32_Range(-SizeScaler, SizeScaler), GetRandomReal32_Range(-SizeScaler, SizeScaler)};
        En2->Position = TileToWorldPos(WorldToTilePos(En2->Position));
        
        
        entity *En3 = CreateEntity(State);
        SetupTree01(En3);
        En3->Position = vec2{GetRandomReal32_Range(-SizeScaler, SizeScaler), GetRandomReal32_Range(-SizeScaler, SizeScaler)};
        En3->Position = TileToWorldPos(WorldToTilePos(En3->Position));
        
        
        entity *En4 = CreateEntity(State);
        SetupRubyNode(En4);
        En4->Position = vec2{GetRandomReal32_Range(-SizeScaler, SizeScaler), GetRandomReal32_Range(-SizeScaler, SizeScaler)};
        En4->Position = TileToWorldPos(WorldToTilePos(En4->Position));
        
        
        entity *En5 = CreateEntity(State);
        SetupSaphireNode(En5);
        En5->Position = vec2{GetRandomReal32_Range(-SizeScaler, SizeScaler), GetRandomReal32_Range(-SizeScaler, SizeScaler)};
        En5->Position = TileToWorldPos(WorldToTilePos(En5->Position));
    }
    
    entity *Pickaxe = CreateEntity(State);
    SetupItemToolPickaxe(Pickaxe);
    Pickaxe->Position = {0, 150};
    Pickaxe->Target  = {0, 150};
    
    
    entity *Pickaxe2 = CreateEntity(State);
    SetupItemToolPickaxe(Pickaxe2);
    Pickaxe2->Position = {32, 150};
    Pickaxe2->Target = {32, 150};

    Player = CreateEntity(State);
    SetupPlayer(Player);

    State->DisplayPlayerHotbar = true;
}

extern
GAME_UPDATE_AND_DRAW(GameUpdateAndDraw)
{
    DrawImGui(State, RenderData, Time);
    
    State->World.WorldFrame = {};
    
    HandleLoadedSounds(State);
    HandleLoadedTracks(State);
    
    // MATRICES
    {
        // NOTE(Sleepster): GAME 
        RenderData->GameCamera.Zoom = 5.3f;

        mat4 ScaleMatrix                              = mat4MakeScale(vec3{1.0f * RenderData->GameCamera.Zoom, 1.0f * RenderData->GameCamera.Zoom, 1.0f});
        mat4 TranslationMatrix                        = mat4Translate(vec3{-RenderData->GameCamera.Position.X, -RenderData->GameCamera.Position.Y, 0.0f});

        RenderData->GameCamera.ViewMatrix             = mat4Identity(1.0f);
        RenderData->GameCamera.ViewMatrix             = mat4Multiply(TranslationMatrix, RenderData->GameCamera.ViewMatrix);
        RenderData->GameCamera.ViewMatrix             = mat4Multiply(ScaleMatrix, RenderData->GameCamera.ViewMatrix);
        RenderData->GameCamera.ProjectionMatrix       = mat4RHGLOrtho((real32)SizeData.Width * -0.5f, (real32)SizeData.Width * 0.5f, (real32)SizeData.Height * -0.5f, (real32)SizeData.Height * 0.5f, -1.0f, 1.0f); 
        RenderData->GameCamera.ProjectionViewMatrix   = mat4Multiply(RenderData->GameCamera.ProjectionMatrix, RenderData->GameCamera.ViewMatrix);        
        
        // NOTE(Sleepster): UI
        RenderData->GameUICamera.ProjectionMatrix     = mat4RHGLOrtho((real32)SizeData.Width * -0.5f, (real32)SizeData.Width * 0.5f, (real32)SizeData.Height * -0.5f, (real32)SizeData.Height * 0.5f, -1.0f, 1.0f); 
        RenderData->GameUICamera.ViewMatrix           = mat4Multiply(mat4Identity(1.0f), ScaleMatrix);
        RenderData->GameUICamera.ProjectionViewMatrix = mat4Multiply(RenderData->GameUICamera.ProjectionMatrix, RenderData->GameUICamera.ViewMatrix);
    }
    
    vec2 MouseToWorld  = TransformMouseCoords(RenderData->GameCamera.ViewMatrix, 
                                              RenderData->GameCamera.ProjectionMatrix, 
                                              State->GameInput.Keyboard.CurrentMouse, 
                                              SizeData);
    
    vec2 MouseToScreen = TransformMouseCoords(RenderData->GameUICamera.ViewMatrix,
                                              RenderData->GameUICamera.ProjectionMatrix, 
                                              State->GameInput.Keyboard.CurrentMouse, 
                                              SizeData);
    
    // NOTE(Sleepster): SELECTED ENTITY
    real32 SelectionDistance = 32.0f;
    real32 MinimumDistance = 0;
    for(uint32 EntityIndex = 0;
        EntityIndex <= State->World.EntityCounter;
        ++EntityIndex)
    {
        // TODO(Sleepster): Fix this, Sorting breaky :( 
        entity *Temp = &State->World.Entities[EntityIndex];
        if((Temp->Flags & IS_VALID))
        {
            real32 Distance = fabsf(v2Distance(Temp->Position, MouseToWorld));
            real32 PlayerToObjectDistance = fabsf(v2Distance(Temp->Position, Player->Position));
            if(Distance <= SelectionDistance && PlayerToObjectDistance <= MaxHitRange)
            {
                if(!State->World.WorldFrame.SelectedEntity || (Distance < MinimumDistance))
                {
                    State->World.WorldFrame.SelectedEntity = Temp;
                    MinimumDistance = Distance;
                }
                
                entity *SelectedEntity = State->World.WorldFrame.SelectedEntity;

                if(IsGameKeyPressed(ATTACK, &State->GameInput) && (Temp->Flags & IS_DESTRUCTABLE) && !(Temp->Flags & IS_UI) && PlayerToObjectDistance <= MaxHitRange)
                {
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
                                Pebbles->Target = SelectedEntity->Position;
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
                                        Branches->Target = SelectedEntity->Position;
                                    }break;
                                    
                                    case SPRITE_Tree01:
                                    {
                                        entity *Trunk = CreateEntity(State);
                                        SetupItemTrunk(Trunk);
                                        Trunk->Position = SelectedEntity->Position;
                                        Trunk->Target = SelectedEntity->Position;
                                    }break;
                                }break;
                            }break;
                            
                            case NODE:
                            {
                                switch(SelectedEntity->Sprite)
                                {
                                    case SPRITE_SaphireOre:
                                    {
                                        entity *SaphireChunk = CreateEntity(State);
                                        SetupItemSaphireChunk(SaphireChunk);
                                        SaphireChunk->Position = SelectedEntity->Position;
                                        SaphireChunk->Target = SelectedEntity->Position;
                                    }break;
                                    
                                    case SPRITE_RubyOre:
                                    {
                                        entity *RubyChunk = CreateEntity(State);
                                        SetupItemRubyChunk(RubyChunk);
                                        RubyChunk->Position = SelectedEntity->Position;
                                        RubyChunk->Target = SelectedEntity->Position;
                                    }break;
                                }
                            }
                        }
                        //PlaySound(&Memory->TemporaryStorage, State, STR("boop.wav"), 1);
                        State->World.WorldFrame.SelectedEntity = {};
                        DeleteEntity(Temp);
                    }
                }
            }
            
            // NOTE(Sleepster): Inventory
            if(Temp->Flags & IS_ITEM && Temp->Archetype == ITEM)
            {
                real32 ItemDistance = fabsf(v2Distance(Temp->Position, Player->Position));
                if(ItemDistance <= ItemPickupDist)
                {
                    if(Temp->Flags & CAN_BE_PICKED_UP)
                    {
                        item NewItem = State->GameData.GameItems[Temp->ItemID];
                        for(int32 InventoryIndex = 0;
                                InventoryIndex < TOTAL_INVENTORY_SIZE;
                                ++InventoryIndex)
                        {
                            if(Player->Inventory.Items[InventoryIndex].ItemID == NewItem.ItemID && 
                                    Player->Inventory.Items[InventoryIndex].CurrentStack < 
                                    Player->Inventory.Items[InventoryIndex].MaxStackCount)
                            {
                                Player->Inventory.Items[InventoryIndex].CurrentStack++;
                                // NOTE(Sleepster): If two matching IDs are found, skip to the deletion 
                                goto Deletion;
                            }
                        }

                        if(Player->UsedInventorySlots < TOTAL_INVENTORY_SIZE)
                        {
                            for(int32 InventoryIndex = 0;
                                    InventoryIndex < TOTAL_INVENTORY_SIZE;
                                    ++InventoryIndex)
                            {
                                if(Player->Inventory.Items[InventoryIndex].ItemID == 0)
                                {
                                    NewItem.CurrentStack = Temp->DroppedItemCount;
                                    if(NewItem.CurrentStack == 0)
                                    {
                                        NewItem.CurrentStack = 1;
                                    }
                                    Player->Inventory.Items[InventoryIndex] = NewItem;
                                    Player->Inventory.Items[InventoryIndex].OccupiedInventorySlot = InventoryIndex;
                                    Player->UsedInventorySlots++;
                                    break;
                                }
                            }
                        }
                        // TODO(Sleepster): Investigate a lambda, even though this is simple as shit, it's kinda grody to some people
Deletion:
                        DeleteEntity(Temp);
                    }
                }
            }
        }
    }
    

    // NOTE(Sleepster): Draw the Tiles
    ivec2  PlayerOffset = iv2Cast(WorldToTilePos(Player->Position));
    ivec2  TileRadius   = {14, 14};
    
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
                DrawQuad(RenderData, {X, Y - (TILE_SIZE)}, {TILE_SIZE, TILE_SIZE}, 0, DARK_GRAY, 0);
            }
        }
    }

    // NOTE(Sleepster): New Hotbar UI 
    if(State->DisplayPlayerHotbar)
    {
        State->UIContext.UICameraViewMatrix       = RenderData->GameUICamera.ViewMatrix;
        State->UIContext.UICameraProjectionMatrix = RenderData->GameUICamera.ProjectionMatrix;
        State->UIContext.GameInput                = &State->GameInput;
        State->UIContext.ActiveFont               = &RenderData->LoadedFonts[UBUNTU_MONO];
        State->UIContext.ActiveFontIndex          = UBUNTU_MONO;

        const real32 Width = SizeData.X * 0.25f;
        const real32 Padding = 4.0f;
        const real32 IconSize = 16.0f;
        const real32 TotalWidth = (PLAYER_HOTBAR_COUNT * IconSize) + ((PLAYER_HOTBAR_COUNT - 1) * Padding);
        const real32 StartingX = (Width / 2.0f) - (TotalWidth / 2.0f);

        real32 YOffset = -90.0f;
        
        for(uint32 InventorySlot = 0;
            InventorySlot < PLAYER_HOTBAR_COUNT;
            ++InventorySlot)
        {
            real32 SlotOffset = (IconSize + Padding) * InventorySlot;
            
            mat4 XForm = mat4Identity(1.0f);
                 XForm = mat4Multiply(XForm, mat4Translate(vec3{StartingX + SlotOffset, YOffset, 0.0}));
                 XForm = mat4Multiply(XForm, mat4Translate(vec3{IconSize * -0.55f, 0.0f, 0.0f}));
                 XForm = mat4Multiply(XForm, mat4MakeScale(vec3{IconSize, IconSize, 1.0}));

            vec4 MatrixPosition = XForm.Columns[3];
            vec2 SlotPosition   = {MatrixPosition.X, MatrixPosition.Y};
            
            static_sprite_data Sprite = GetSprite(State, SPRITE_UIItemBox);
            ui_element_state HotbarSlotState = CloverUIButton(&State->UIContext, STR("HotbarSlot"), SlotPosition, {IconSize, IconSize}, Sprite, WHITE);
            ui_element *HotbarSlot = &State->UIContext.UIElements[HotbarSlotState.UIID.ID];

            Player->Inventory.InventorySlotButtons[InventorySlot] = HotbarSlot;

            HotbarSlot->XForm = XForm;
            HotbarSlot->Sprite = Sprite;
            HotbarSlot->DrawColor = WHITE;

            item *Item = &Player->Inventory.Items[InventorySlot];
            Item->OccupiedInventorySlot = InventorySlot;

            Sprite = GetSprite(State, Item->Sprite);
            if(Sprite != State->GameData.Sprites[SPRITE_Nil])
            {
                if(!HotbarSlotState.IsHot)
                {
                    XForm = mat4Multiply(XForm, mat4MakeScale(vec3{0.65, 0.65, 1.0}));
                }

                if(InventorySlot == Player->Inventory.CurrentInventorySlot)
                {
                    XForm = mat4Multiply(XForm, mat4MakeScale(vec3{1.2, 1.2, 1.0}));
                }
                DrawUISpriteXForm(RenderData, XForm, Sprite, 0, WHITE);
            }

            if(HotbarSlotState.IsHot)
            {
                HotbarSlot->DrawColor = RED;
                if(Item->Sprite != SPRITE_Nil)
                {
                    XForm = mat4Multiply(XForm, mat4MakeScale(vec3{0.8, 0.8, 1.0}));
                    
                    real32 NewUIYOffset;
                    real32 NewUIYDescOffset;
                    real32 SpriteYOffset;
                    if(State->DisplayPlayerInventory)
                    {
                        NewUIYOffset = 24.0f;
                        NewUIYDescOffset = 8.0f;
                        SpriteYOffset = 25.0f;
                    }
                    else
                    {
                        NewUIYOffset = 16.0f;
                        NewUIYDescOffset = 3.0f;
                        SpriteYOffset = 20.0f;
                    }

                    vec2 UIBoxSize = {10, 25};

                    mat4 UIXForm = mat4Identity(1.0f);
                         UIXForm = mat4Multiply(UIXForm, mat4Translate(vec3{StartingX + SlotOffset, YOffset, 0.0}));
                         UIXForm = mat4Multiply(UIXForm, mat4Translate(vec3{IconSize * -0.55f, NewUIYOffset, 0.0f}));

                    // NOTE(Sleepster): String Size to influence the BosSize
                    vec4 UIMatrixPosition = UIXForm.Columns[3];
                    vec2 Position = vec2{UIMatrixPosition.X, UIMatrixPosition.Y};

                    static_sprite_data SpriteData = GetSprite(State, Item->Sprite);
                    vec2 SpriteSize = v2Cast(SpriteData.SpriteSize);

                    ui_element *ItemDescData = CloverUIMakeTextElement(&State->UIContext, Item->ItemDesc, {Position.X, Position.Y + NewUIYDescOffset}, 10, TEXT_ALIGNMENT_Center, GREEN);
                    CloverUIMakeTextElement(&State->UIContext, Item->ItemName, {Position.X + 4, Position.Y + NewUIYOffset}, 10, TEXT_ALIGNMENT_Center, GREEN);

                    mat4 SpriteXForm = UIXForm;
                    UIXForm = mat4Multiply(UIXForm, mat4Translate(vec3{0, NewUIYOffset, 0}));
                    UIXForm = mat4Multiply(UIXForm, mat4MakeScale(v2Expand(UIBoxSize + ItemDescData->Size, 1.0f)));
                    DrawUISpriteXForm(RenderData, UIXForm, GetSprite(State, SPRITE_Nil), 0, vec4{0.2f, 0.2f, 0.2f, 0.2f});

                    SpriteXForm = mat4Multiply(SpriteXForm, mat4Translate(vec3{UIBoxSize.X + ItemDescData->Size.X * -0.5f, SpriteYOffset, 0}));
                    SpriteXForm = mat4Multiply(SpriteXForm, mat4MakeScale(vec3{IconSize, IconSize, 1.0f}));
                    DrawUISpriteXForm(RenderData, SpriteXForm, SpriteData, 0, WHITE);

                    SpriteXForm = mat4Multiply(SpriteXForm, mat4MakeScale(vec3{1 / IconSize, 1 / IconSize, 1.0f}));
                    SpriteXForm = mat4Multiply(SpriteXForm, mat4Translate(vec3{-(UIBoxSize.X + ItemDescData->Size.X * -0.5f), 0, 0}));
                    SpriteXForm = mat4Multiply(SpriteXForm, mat4Translate(vec3{(UIBoxSize.X + ItemDescData->Size.X * -0.5f), 19, 0}));
                    SpriteXForm = mat4Multiply(SpriteXForm, mat4Translate(vec3{-7, 0, 0}));
                    SpriteXForm = mat4Multiply(SpriteXForm, mat4MakeScale(vec3{16, 16, 1.0f}));

                    DrawUISpriteXForm(RenderData, SpriteXForm, GetSprite(State, SPRITE_Nil), 0, vec4{0.1f, 0.1f, 0.1f, 0.4f});

                    SpriteXForm = mat4Multiply(SpriteXForm, mat4MakeScale(vec3{1 / IconSize, 1 / IconSize, 1.0f}));
                    SpriteXForm = mat4Multiply(SpriteXForm, mat4Translate(vec3{2, -6, 0}));
                    SpriteXForm = mat4Multiply(SpriteXForm, mat4MakeScale(vec3{16, 16, 1.0f}));

                    vec4 ItemCountTextPosition = SpriteXForm.Columns[3];
                    Position = vec2{ItemCountTextPosition.X, ItemCountTextPosition.Y};
                    CloverUIMakeTextElement(&State->UIContext, sprints(&Memory->TemporaryStorage, STR("x%d"), Item->CurrentStack), {Position.X + 3, Position.Y}, 15, TEXT_ALIGNMENT_Center, GREEN);
                }
            }
            
            // SELECTED INVENTORY ITEM
            if(HotbarSlot->IsActive)
            {
                if(Item->Sprite != SPRITE_Nil && !Player->Inventory.SelectedInventoryItem)
                {
                    Player->Inventory.SelectedInventoryItem = Item;
                }
            }

            if(Player->Inventory.CurrentInventorySlot == InventorySlot)
            {
                Player->Inventory.SelectedHotbarItem = &Player->Inventory.Items[InventorySlot];
            }
        }
    }

    // NOTE(Sleepster): Large Inventory UI 
    const real32 InventoryYOffset = -70.0f;
    if(State->DisplayPlayerInventory)
    {
        // TODO(Sleepster): This is all copy-pasted, clean it up and move common code into a seperate function 
        State->UIContext.UICameraViewMatrix       = RenderData->GameUICamera.ViewMatrix;
        State->UIContext.UICameraProjectionMatrix = RenderData->GameUICamera.ProjectionMatrix;
        State->UIContext.GameInput                = &State->GameInput;
        State->UIContext.ActiveFont               = &RenderData->LoadedFonts[UBUNTU_MONO];
        State->UIContext.ActiveFontIndex          = UBUNTU_MONO;

        const real32 Width = SizeData.X * 0.25f;
        const real32 Padding = 1.00f;
        const real32 IconSize = 16.0f;
        const real32 TotalWidth = ((PLAYER_INVENTORY_SIZE * IconSize) + (IconSize * 2) - 7) + ((PLAYER_INVENTORY_SIZE - 1) * Padding);
        const real32 StartingX = (Width / 2.0f) - (TotalWidth / 2.0f);
        
        for(uint32 InventorySlot = PLAYER_HOTBAR_COUNT;
            InventorySlot < TOTAL_INVENTORY_SIZE;
            ++InventorySlot)
        {
            real32 SlotOffset = (IconSize + Padding) * (InventorySlot - 6);
            
            mat4 XForm = mat4Identity(1.0f);
                 XForm = mat4Multiply(XForm, mat4Translate(vec3{StartingX + SlotOffset, InventoryYOffset, 0.0}));
                 XForm = mat4Multiply(XForm, mat4Translate(vec3{IconSize * -0.55f, 0.0f, 0.0f}));
                 XForm = mat4Multiply(XForm, mat4MakeScale(vec3{IconSize, IconSize, 1.0}));

            vec4 MatrixPosition = XForm.Columns[3];
            vec2 SlotPosition   = {MatrixPosition.X, MatrixPosition.Y};
            
            static_sprite_data Sprite = GetSprite(State, SPRITE_UIItemBox);
            ui_element_state HotbarSlotState = CloverUIButton(&State->UIContext, STR("InventorySlot"), SlotPosition, {IconSize, IconSize}, Sprite, WHITE);
            ui_element *HotbarSlot = &State->UIContext.UIElements[HotbarSlotState.UIID.ID];

            Player->Inventory.InventorySlotButtons[InventorySlot] = HotbarSlot;

            HotbarSlot->XForm = XForm;
            HotbarSlot->Sprite = Sprite;
            HotbarSlot->DrawColor = WHITE;

            item *Item = &Player->Inventory.Items[InventorySlot];
            Item->OccupiedInventorySlot = InventorySlot;

            Sprite = GetSprite(State, Item->Sprite);
            if(Sprite != State->GameData.Sprites[SPRITE_Nil] && !HotbarSlotState.IsHot)
            {
                XForm = mat4Multiply(XForm, mat4MakeScale(vec3{0.65, 0.65, 1.0}));
                DrawUISpriteXForm(RenderData, XForm, Sprite, 0, WHITE);
            }

            if(HotbarSlotState.IsHot)
            {
                HotbarSlot->DrawColor = RED;
                if(Item->Sprite != SPRITE_Nil)
                {
                    XForm = mat4Multiply(XForm, mat4MakeScale(vec3{1.0, 1.0, 1.0}));
                    DrawUISpriteXForm(RenderData, XForm, Sprite, 0, WHITE);
                    
                    real32 NewUIYOffset;
                    real32 NewUIYDescOffset;
                    real32 SpriteYOffset;
                    if(State->DisplayPlayerInventory)
                    {
                        NewUIYOffset = 24.0f;
                        NewUIYDescOffset = 8.0f;
                        SpriteYOffset = 25.0f;
                    }
                    else
                    {
                        NewUIYOffset = 16.0f;
                        NewUIYDescOffset = 3.0f;
                        SpriteYOffset = 20.0f;
                    }

                    vec2 UIBoxSize = {10, 25};

                    mat4 UIXForm = mat4Identity(1.0f);
                         UIXForm = mat4Multiply(UIXForm, mat4Translate(vec3{StartingX + SlotOffset, -90, 0.0}));
                         UIXForm = mat4Multiply(UIXForm, mat4Translate(vec3{IconSize * -0.55f, NewUIYOffset, 0.0f}));

                    // NOTE(Sleepster): String Size to influence the BosSize
                    vec4 UIMatrixPosition = UIXForm.Columns[3];
                    vec2 Position = vec2{UIMatrixPosition.X, UIMatrixPosition.Y};

                    static_sprite_data SpriteData = GetSprite(State, Item->Sprite);
                    vec2 SpriteSize = v2Cast(SpriteData.SpriteSize);

                    ui_element *ItemDescData = CloverUIMakeTextElement(&State->UIContext, Item->ItemDesc, {Position.X, Position.Y + NewUIYDescOffset}, 10, TEXT_ALIGNMENT_Center, GREEN);
                    CloverUIMakeTextElement(&State->UIContext, Item->ItemName, {Position.X + 4, Position.Y + NewUIYOffset}, 10, TEXT_ALIGNMENT_Center, GREEN);

                    mat4 SpriteXForm = UIXForm;
                    UIXForm = mat4Multiply(UIXForm, mat4Translate(vec3{0, NewUIYOffset, 0}));
                    UIXForm = mat4Multiply(UIXForm, mat4MakeScale(v2Expand(UIBoxSize + ItemDescData->Size, 1.0f)));
                    DrawUISpriteXForm(RenderData, UIXForm, GetSprite(State, SPRITE_Nil), 0, vec4{0.2f, 0.2f, 0.2f, 0.2f});

                    SpriteXForm = mat4Multiply(SpriteXForm, mat4Translate(vec3{UIBoxSize.X + ItemDescData->Size.X * -0.5f, SpriteYOffset, 0}));
                    SpriteXForm = mat4Multiply(SpriteXForm, mat4MakeScale(vec3{IconSize, IconSize, 1.0f}));
                    DrawUISpriteXForm(RenderData, SpriteXForm, SpriteData, 0, WHITE);

                    SpriteXForm = mat4Multiply(SpriteXForm, mat4MakeScale(vec3{1 / IconSize, 1 / IconSize, 1.0f}));
                    SpriteXForm = mat4Multiply(SpriteXForm, mat4Translate(vec3{-(UIBoxSize.X + ItemDescData->Size.X * -0.5f), 0, 0}));
                    SpriteXForm = mat4Multiply(SpriteXForm, mat4Translate(vec3{(UIBoxSize.X + ItemDescData->Size.X * -0.5f), 19, 0}));
                    SpriteXForm = mat4Multiply(SpriteXForm, mat4Translate(vec3{-7, 0, 0}));
                    SpriteXForm = mat4Multiply(SpriteXForm, mat4MakeScale(vec3{16, 16, 1.0f}));

                    DrawUISpriteXForm(RenderData, SpriteXForm, GetSprite(State, SPRITE_Nil), 0, vec4{0.1f, 0.1f, 0.1f, 0.4f});

                    SpriteXForm = mat4Multiply(SpriteXForm, mat4MakeScale(vec3{1 / IconSize, 1 / IconSize, 1.0f}));
                    SpriteXForm = mat4Multiply(SpriteXForm, mat4Translate(vec3{2, -6, 0}));
                    SpriteXForm = mat4Multiply(SpriteXForm, mat4MakeScale(vec3{16, 16, 1.0f}));

                    vec4 ItemCountTextPosition = SpriteXForm.Columns[3];
                    Position = vec2{ItemCountTextPosition.X, ItemCountTextPosition.Y};
                    CloverUIMakeTextElement(&State->UIContext, sprints(&Memory->TemporaryStorage, STR("x%d"), Item->CurrentStack), {Position.X + 3, Position.Y}, 15, TEXT_ALIGNMENT_Center, GREEN);
                }
            }
        }
    }

    // NOTE(Sleepster): Swapping Inventory Positions 
    for(uint32 InventoryIndexSlot = 0;
        InventoryIndexSlot < TOTAL_INVENTORY_SIZE;
        InventoryIndexSlot++)
    {
        item *Item = &Player->Inventory.Items[InventoryIndexSlot];
        ui_element *InventoryElement = Player->Inventory.InventorySlotButtons[InventoryIndexSlot]; 
        if(Item->CurrentStack == 0 && InventoryIndexSlot == Player->Inventory.CurrentInventorySlot)
        {
            ResetItemSlotState(Item);
        }

        if(InventoryElement)
        {
            if(InventoryElement->IsActive)
            {
                if(Item->Sprite != SPRITE_Nil && !Player->Inventory.SelectedInventoryItem)
                {
                    Player->Inventory.SelectedInventoryItem = Item;
                }
            }
            if(InventoryIndexSlot == Player->Inventory.CurrentInventorySlot)
            {
                InventoryElement->XForm = mat4Multiply(InventoryElement->XForm, mat4MakeScale(vec3{1.20f, 1.20f, 1.0f}));
            }

            if((Player->Inventory.SelectedInventoryItem && Player->Inventory.SelectedInventoryItem->Sprite != SPRITE_Nil))
            {
                item *Selection = Player->Inventory.SelectedInventoryItem;
                vec2 MousePos = TransformMouseCoords(RenderData->GameUICamera.ViewMatrix, 
                        RenderData->GameUICamera.ProjectionMatrix, 
                        State->GameInput.Keyboard.CurrentMouse, 
                        SizeData);

                static_sprite_data SelectionSprite = GetSprite(State, Selection->Sprite);
                CloverUIPushLayer(&State->UIContext, 1);
                CloverUISpriteElement(&State->UIContext, MousePos + vec2{1, 0}, v2Cast(SelectionSprite.SpriteSize), NULLMATRIX, SelectionSprite, WHITE);
                CloverUIPushLayer(&State->UIContext, 0);

                // NOTE(Sleepster): Dropping Selected inventory Items 
                if(IsGameKeyPressed(DROP_ITEM, &State->GameInput))
                {
                    entity *SpawnedItem = CreateEntity(State);
                    SetupDroppedEntity(RenderData, State, Selection, SpawnedItem);

                    Player->Inventory.Items[Player->Inventory.SelectedInventoryItem->OccupiedInventorySlot] = {};
                    Player->Inventory.SelectedInventoryItem = {};
                }
            }

            if(!Player->Inventory.SwapItem)
            {
                item *Check = &Player->Inventory.Items[InventoryIndexSlot];
                if(InventoryElement->IsActive && Player->Inventory.SelectedInventoryItem && Check->OccupiedInventorySlot != Player->Inventory.SelectedInventoryItem->OccupiedInventorySlot)
                {
                    Player->Inventory.SwapItem = Check;
                }
            }

            if(Player->Inventory.SwapItem && Player->Inventory.SelectedInventoryItem)
            {
                SwapInventoryItems(&Player->Inventory, Player->Inventory.SelectedInventoryItem, Player->Inventory.SwapItem); 
                Player->Inventory.SwapItem = NULL;
                Player->Inventory.SelectedInventoryItem = NULL;
            }
        }
    }

    // NOTE(Sleepster): Quickdropping HotbarItem
    if(IsGameKeyPressed(DROP_HELD, &State->GameInput))
    {
        item *HotbarItem = Player->Inventory.SelectedHotbarItem;
        if(HotbarItem && Player->Inventory.CurrentInventorySlot != NULLSLOT)
        {
            if(HotbarItem->CurrentStack != 0)
            {
                if(IsKeyDown(KEY_CONTROL, &State->GameInput))
                {
                    for(uint32 ItemCount = 0;
                        ItemCount < HotbarItem->CurrentStack;
                        ItemCount++)
                    {
                        entity *DroppedEntity = CreateEntity(State);
                        SetupDroppedEntity(RenderData, State, HotbarItem, DroppedEntity);
                        HotbarItem->CurrentStack = 0;
                    }
                }
                else
                {
                    --HotbarItem->CurrentStack;
                    entity *DroppedEntity = CreateEntity(State);
                    SetupDroppedEntity(RenderData, State, HotbarItem, DroppedEntity);
                    DroppedEntity->DroppedItemCount = 1;
                }
            }
            else
            {
                Player->Inventory.SelectedHotbarItem = {};
            }
        }
    }


    // NOTE(Sleepster): Crafting Menu 
    {
                    
    }

    // NOTE(Sleepster): UI Rendering 
    CloverUIDrawWidgets(RenderData, &State->UIContext);
    CloverUIResetState(&State->UIContext); 

    // TRANSPARENCY TEST
    {
        mat4 Identity  = mat4Identity(1.0f);
        mat4 Translate = mat4Multiply(Identity, mat4Translate(vec3{10.0f, 10.0f, 0.0f}));
        mat4 Scale     = mat4Multiply(Identity, mat4MakeScale(vec3{100.0f, 100.0f, 1.0f}));
        
        mat4 Total = Translate * Scale;
        DrawRectXForm(RenderData, Total, {16, 16}, 0, vec4{1.0f, 0.0f, 1.0f, 0.3f});
    }
    

    // NOTE(Sleepster): Sorting, off for now. Breaks too much 
    {
//        qsort(State->World.Entities, State->World.EntityCounter, sizeof(struct entity), CompareEntityYAxis); 
    }
        

    vec2 SelectionBoxDrawSize = {16, 16};
    // NOTE(Sleepster): DRAW ENTITIES
    for(uint32 EntityIndex = 0;
        EntityIndex <= State->World.EntityCounter;
        ++EntityIndex)
    {
        entity *Temp = &State->World.Entities[EntityIndex];
        SelectionBoxDrawSize.X = SinBreatheNormalized(Time.CurrentTimeInSeconds, 0.5f, 13.0f, 15.0f);
        SelectionBoxDrawSize.Y = SinBreatheNormalized(Time.CurrentTimeInSeconds, 0.5f, 13.0f, 15.0f);

        if(Temp->Flags & IS_VALID)
        {
            switch(Temp->Archetype)
            {
                case PLAYER:
                {
                    Player = Temp;
                    HandleInput(State, Temp, Time);
                    RenderData->GameCamera.Target = Temp->Position;

                    v2Approach(&RenderData->GameCamera.Position, RenderData->GameCamera.Target, 5.0f, Time.Delta);
                    DrawEntity(RenderData, State, Temp, Temp->Position, WHITE);
                }break;
                
                case ITEM:
                {
                    v2Approach(&Temp->Position, Temp->Target, 5.0f, Time.Delta);
                    Temp->Position.Y += 0.01f * SinBreathe(Time.CurrentTimeInSeconds, 1.25f);
                    if(v2Distance(Temp->Target, Temp->Position) <= PickupEpsilon)
                    {
                        Temp->Flags += CAN_BE_PICKED_UP;
                    }

                    DrawEntity(RenderData, State, Temp, Temp->Position, WHITE);
                    if(State->World.WorldFrame.SelectedEntity == Temp)
                    {
                        State->World.WorldFrame.SelectedEntity = Temp;
                        static_sprite_data SelectionBoxSprite = GetSprite(State, SPRITE_SelectionBox);
                        static_sprite_data EntitySprite = GetSprite(State, Temp->Sprite);
                    }
                }break;
                
                default:
                {
                    if(State->World.WorldFrame.SelectedEntity == Temp)
                    {
                        State->World.WorldFrame.SelectedEntity = Temp;
                        static_sprite_data SelectionBoxSprite = GetSprite(State, SPRITE_SelectionBox);
                        static_sprite_data EntitySprite = GetSprite(State, Temp->Sprite);

                        DrawSprite(RenderData, 
                                   SelectionBoxSprite, 
                                   Temp->Position 
                                   - vec2{0, real32(EntitySprite.SpriteSize.Y * 0.25f)},
                                   SelectionBoxDrawSize, 
                                   WHITE, 
                                   0, 
                                   1);
                    }
                    DrawEntity(RenderData, State, Temp, Temp->Position, WHITE);
                }break;
            }
        }
    }
    // NOTE(Sleepster): World Text and Quad at {0,0}
    DrawGameText(RenderData, sprints(&Memory->TemporaryStorage, STR("%f, %f"), MouseToWorld.X, MouseToWorld.Y), {-100, 0}, 15.0f, UBUNTU_MONO, GREEN);
    DrawGameText(RenderData, sprints(&Memory->TemporaryStorage, STR("%f, %f"), MouseToScreen.X, MouseToScreen.Y), {-100, 100}, 15.0f, UBUNTU_MONO, BLUE);
}

extern
GAME_FIXED_UPDATE(GameFixedUpdate)
{
}
