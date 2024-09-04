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

constexpr uint32 MAX_ENTITIES = 1000;

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
    real32 Alpha;
    real32 Current;
    real32 Next;
    
    int32 FPSCounter;
};

enum entity_flags
{
    IS_VALID  = 1 << 0,
    IS_SOLID  = 1 << 1,
    IS_ACTOR  = 1 << 2,
    IS_TILE   = 1 << 3,
    IS_ACTIVE = 1 << 4,
    ENTITY_FLAGS_COUNT
};

enum entity_arch
{
    NIL    = 0,
    PLAYER = 1,
    ROCK   = 2,
    COUNT
};

enum sprite_type
{
    SPRITE_Nil    = 0,
    SPRITE_Player = 1,
    SPRITE_Rock   = 2,
    SPRITE_Count  = 3,
};

struct entity
{
    sprite_type Sprite;
    uint32      Archetype;
    uint32      Flags;
    
    vec2        Position;
    vec2        Acceleration;
    vec2        Size;
    real32      Speed;
    real32      Rotation;
};

struct world
{
    entity Entities[MAX_ENTITIES];  
    uint32 EntityCounter;
};

struct game_state
{
    KeyCodeID KeyCodeLookup[KEY_COUNT];
    Input GameInput;
    
    world World;
    
    struct
    {   
        ma_engine AudioEngine;
        
        sound_instance Instances[MAX_SOUNDS];
        sound_instance SoundTracks[MAX_TRACKS];
        
        uint32 ActiveInstances;
        uint32 LoadedTracks;
    }SFXData;
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

#define GAME_UPDATE_AND_RENDER(name) void name(game_memory *Memory, gl_render_data *RenderData, game_state *State, time Time, ivec4 SizeData)
typedef GAME_UPDATE_AND_RENDER(game_update_and_render);
GAME_UPDATE_AND_RENDER(GameUpdateAndRenderStub)
{
}

struct game_functions
{
    HMODULE  GameCodeDLL;
    FILETIME LastWriteTime;
    
    game_on_awake          *OnAwake;
    game_fixed_update      *FixedUpdate;
    game_update_and_render *UpdateAndRender;
    
    bool IsLoaded;
    bool IsValid;
};

#endif // _CLOVER_H
