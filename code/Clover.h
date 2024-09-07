/* date = August 27 2024 06:00 am*/

#ifndef CLOVER_H
#define CLOVER_H

#include "Intrinsics.h"

#include "util/Math.h"
#include "util/Array.h"
#include "util/FileIO.h"
#include "util/CustomStrings.h"

#include "Clover.h"
#include "Clover_Input.h"
#include "Clover_Globals.h"
#include "Clover_Renderer.h"
#include "Clover_Audio.h"

#include "../data/deps/MiniAudio/miniaudio.h"

struct sound_instance
{
    ma_sound Sound;
    bool     IsPlaying;
    bool     IsActive;
};

struct game_memory
{
    memory_arena PermanentStorage;
    memory_arena TemporaryStorage;
};

struct time
{
    real32 Delta;
    real32 Current;
    real64 Alpha;
    real64 Next;
    
    int32  FPSCounter;
    real32 MSPerFrame;
    real32 CurrentTimeInSeconds;
};

struct range_r32
{
    real32 Minimum;
    real32 Maximum;
};

struct range_v2
{
    vec2 Min;
    vec2 Max;
};

struct box2d : range_v2
{
    bool IsActive;
};

enum sprite_type
{
    SPRITE_Nil      = 0,
    SPRITE_Player   = 1,
    SPRITE_Rock     = 2,
    SPRITE_Tree00   = 3,
    SPRITE_Tree01   = 4,
    SPRITE_Pebbles  = 5,
    SPRITE_Branches = 6,
    SPRITE_Trunk    = 7,
    SPRITE_Count,
};


enum entity_flags
{
    IS_VALID        = 1 << 0,
    IS_SOLID        = 1 << 1,
    IS_ACTOR        = 1 << 2,
    IS_TILE         = 1 << 3,
    IS_ACTIVE       = 1 << 4,
    IS_ITEM         = 1 << 5,
    IS_DESTRUCTABLE = 1 << 6,
    IS_IN_INVENTORY = 1 << 7,
    ENTITY_FLAGS_COUNT,
};

enum entity_arch
{
    NIL    = 0,
    PLAYER = 1,
    ROCK   = 2,
    TREE   = 3,
    ITEM   = 5,
    COUNT
};

enum item_id
{
    ITEM_Nil,
    ITEM_Pebbles,
    ITEM_Branches,
    ITEM_Trunk,
    ITEM_IDCount
};

// NOTE(Sleepster): Probably don't need this
struct item
{
    uint32      Archetype;
    uint32      Flags;
    sprite_type Sprite;
    item_id     ItemID;
    
    int32       StackCount;
    int32       CurrentStack;
};

struct entity_item_inventory
{
    item Items[ITEM_IDCount];
};

struct entity
{
    sprite_type Sprite;
    item_id     ItemID;
    
    uint32      Archetype;
    uint32      Flags;
    uint32      Health;
    
    vec2        Position;
    vec2        Size;
    real32      Speed;
    real32      Rotation;
    
    box2d       SelectionBox;
    box2d       BoxCollider;
    
    entity_item_inventory Inventory;
};

struct game_state
{
    KeyCodeID KeyCodeLookup[KEY_COUNT];
    Input GameInput;
    
    
    
    // NOTE(Sleepster): World Data
    struct
    {
        entity Entities[MAX_ENTITIES];  
        item   Items[1000];
        
        uint32 EntityCounter;
        
        struct 
        {
            entity *SelectedEntity;
        }WorldFrame;
    }World;
    
    // NOTE(Sleepster): Audio Stuffs
    struct
    {   
        ma_engine AudioEngine;
        
        sound_instance Instances[MAX_SOUNDS];
        sound_instance SoundTracks[MAX_TRACKS];
        
        uint32 ActiveInstances;
        uint32 LoadedTracks;
    }SFXData;
    
    // NOTE(Sleepster): Visual Assets
    struct 
    { 
        static_sprite_data Sprites[20];
    }GameData;
};

#define GAME_ON_AWAKE(name) void name(game_memory *Memory, gl_render_data *RenderData, game_state *State)
typedef GAME_ON_AWAKE(game_on_awake);
GAME_ON_AWAKE(GameOnAwakeStub)
{
}

#define GAME_FIXED_UPDATE(name) void name(game_memory *Memory, gl_render_data *RenderData, game_state *State, time Time)
typedef GAME_FIXED_UPDATE(game_fixed_update);
GAME_FIXED_UPDATE(GameFixedUpdateStub)
{
}

#define GAME_UPDATE_AND_DRAW(name) void name(game_memory *Memory, gl_render_data *RenderData, game_state *State, time Time, ivec4 SizeData)
typedef GAME_UPDATE_AND_DRAW(game_update_and_draw);
GAME_UPDATE_AND_DRAW(GameUpdateAndDrawStub)
{
}

struct game_functions
{
    HMODULE  GameCodeDLL;
    FILETIME LastWriteTime;
    
    game_on_awake          *OnAwake;
    game_fixed_update      *FixedUpdate;
    game_update_and_draw   *UpdateAndDraw;
    
    bool IsLoaded;
    bool IsValid;
};

#endif // _CLOVER_H
