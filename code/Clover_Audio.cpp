#include "Intrinsics.h"

// UTILS
#include "util/Math.h"
#include "util/Array.h"
#include "util/FileIO.h"
#include "util/CustomStrings.h"

// CLOVER HEADERS
#include "Clover.h"
#include "../data/deps/MiniAudio/miniaudio.h"

internal sound_instance *
CreateSound(game_state *State)
{
    sound_instance *Result = {};
    for(uint32 Instance = 0;
        Instance < MAX_SOUNDS;
        ++Instance)
    {
        sound_instance *Found = &State->SFXData.Instances[Instance];
        if(!Found->IsActive)
        {
            Result = Found;
            break;
        }
    }
    Assert(Result);
    
    Result->IsActive = 1;
    State->SFXData.ActiveInstances++;
    return(Result);
}

internal sound_instance *
CreateTrack(game_state *State)
{
    sound_instance *Result = {};
    for(uint32 Track = 0;
        Track < MAX_TRACKS;
        ++Track)
    {
        sound_instance *Found = &State->SFXData.SoundTracks[Track];
        if(!Found->IsActive)
        {
            Result = Found;
            break;
        }
    }
    Assert(Result);
    
    Result->IsActive = 1;
    State->SFXData.LoadedTracks++;
    return(Result);
}

internal void
DeleteSound(sound_instance *Instance)
{
    
    memset(Instance, 0, sizeof(struct sound_instance));
    Instance->IsActive = 0;
}

internal void
PlayTrackFromDisk(memory_arena *Memory, game_state *State, string Filepath, real32 Volume)
{
    string DataPath    = STR("../data/res/sounds/\0");
    string NewFilepath = ConcatinatePair(Memory, DataPath, Filepath);
    
    sound_instance *NewSound = CreateTrack(State);
    if(ma_sound_init_from_file(&State->SFXData.AudioEngine, 
                               (const char *)NewFilepath.Data, 
                               MA_SOUND_FLAG_ASYNC|MA_SOUND_FLAG_STREAM, 
                               0, 
                               0, 
                               &NewSound->Sound) == MA_SUCCESS)
    {
        ma_sound_set_looping(&NewSound->Sound, 1);
        ma_sound_set_volume(&NewSound->Sound, Volume);
        ma_sound_start(&NewSound->Sound);
        NewSound->IsPlaying = 1;
        return;
    }
    else
    {
        Assert(false);
    }
}

internal void
PlaySound(memory_arena *Memory, game_state *State, string Filepath, real32 Volume)
{
    // TODO(Sleepster): Weird shit going on with ConcatinatePair  
    string DataPath      = STR("../data/res/sounds/\0");
    string NewFilepath   = ConcatinatePair(Memory, DataPath, Filepath);
    
    sound_instance *NewSound = CreateSound(State);
    if(ma_sound_init_from_file(&State->SFXData.AudioEngine, 
                               (const char *)NewFilepath.Data, 
                               MA_SOUND_FLAG_ASYNC, 
                               0, 
                               0, 
                               &NewSound->Sound) == MA_SUCCESS) 
    {
        ma_sound_set_volume(&NewSound->Sound, Volume);
        ma_sound_start(&NewSound->Sound);
        
        NewSound->IsPlaying = 1;
        return;
    }
    else
    {
        Assert(0);
    }
}

// TODO(Sleepster): This is not very Efficient, oN^2
internal void
HandleLoadedSounds(game_state *State)
{
    for(uint32 Instance = 0;
        Instance < MAX_SOUNDS;
        ++Instance)
    {
        sound_instance *Found = &State->SFXData.Instances[Instance];
        if(Found->IsPlaying && !ma_sound_is_playing(&Found->Sound))
        {
            ma_sound_uninit(&Found->Sound);
            Found->IsPlaying = false;
            
            DeleteSound(Found);
        }
    }
}

internal void
HandleLoadedTracks(game_state *State)
{
    for(uint32 Track = 0;
        Track < MAX_TRACKS;
        ++Track)
    {
        sound_instance *Found = &State->SFXData.SoundTracks[Track];
        if(!Found->IsPlaying && Found->IsActive)
        {
            ma_sound_uninit(&Found->Sound);
            Found->IsPlaying = false;
            
            DeleteSound(Found);
        }
    }
}

