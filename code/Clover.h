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


internal void DeleteSound(sound_instance *Instance);
internal void PlayTrackFromDisk(memory_arena *Memory, game_state *State, string Filepath, real32 Volume);
internal void PlaySound(memory_arena *Memory, game_state *State, string Filepath, real32 Volume);
internal void HandleLoadedSounds(game_state *State);
internal void HandleLoadedTracks(game_state *State);

internal sound_instance* CreateSound(game_state *State);
internal sound_instance* CreateTrack(game_state *State);

#endif // _CLOVER_H
