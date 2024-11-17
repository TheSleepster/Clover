/* ========================================================================
   $File: Clover_Asset.cpp $
   $Date: November 11 2024 09:34 am $
   $Revision: $
   $Creator: Justin Lewis $
   ======================================================================== */

#include "Intrinsics.h"

// UTILS
#include "util/Math.h"
#include "util/Array.h"
#include "util/FileIO.h"
#include "util/String.h"
#include "util/Pairs.h"

#include "Clover_Asset.h"
#include "Clover_Platform.h"
#include "Clover_Renderer.h"
#include "Clover.h"

struct load_texture_job
{
    transient_state *TransientState;
    string           Filepath;
    asset_slot      *TextureSlot;
};

struct load_font_job
{
    game_assets *Assets;
    asset_slot  *FontSlot;

    uint32       FontSize;
    string       Filepath;
};

struct load_sound_job
{
    game_state       *GameState;
    asset_slot       *SoundSlot;

    int32             StreamingSampleIndex;
    int32             SamplesToStream;
    string            Filepath;
};

internal
PLATFORM_JOB_ENTRY_CALLBACK(LoadTextureCallback)
{
    load_texture_job *LoadTextureJob = (load_texture_job *)Data;
    ReadWriteBarrier;
    if(LoadTextureJob && LoadTextureJob->TextureSlot->SlotState == AssetState_Queued)
    {
        LoadTextureJob->TextureSlot->Texture->RawData = (char *)stbi_load((const char *)LoadTextureJob->Filepath.Data, 
                &LoadTextureJob->TextureSlot->Texture->TextureData.Width, 
                &LoadTextureJob->TextureSlot->Texture->TextureData.Height, 
                &LoadTextureJob->TextureSlot->Texture->TextureData.Channels, 
                4);
        LoadTextureJob->TextureSlot->Texture->Filepath = LoadTextureJob->Filepath;
    }
}

internal
PLATFORM_JOB_ENTRY_CALLBACK(LoadFontDataCallback)
{
    load_font_job *FontJob = (load_font_job *)Data;
    ReadWriteBarrier;
    if(FontJob && FontJob->FontSlot->SlotState == AssetState_Queued)
    {
        asset_slot *FontSlot = FontJob->FontSlot;
        FontSlot->Font->RawData = 
            CloverLoadSDFFontData(FontJob->Assets->TransientState, FontJob->FontSlot->Font, FontJob->Filepath, FontJob->FontSize);
    }
}

internal
PLATFORM_JOB_ENTRY_CALLBACK(LoadSoundDataCallback)
{
    load_sound_job *SoundJob = (load_sound_job *)Data;
    ReadWriteBarrier;

    uint32 ID = SoundJob->SoundSlot->Sound->ID;
    AtomicCompareExchangei32((volatile int32 *)&SoundJob->SoundSlot->SlotState, AssetState_Loaded, AssetState_Queued);
    if(SoundJob->SoundSlot->SlotState == AssetState_Loaded)
    {
        SoundJob->SoundSlot->Sound->ID = ID;
        CloverLoadWAVFile(&SoundJob->GameState->SoundArena, SoundJob->SoundSlot->Sound, 
                           SoundJob->Filepath, SoundJob->StreamingSampleIndex, SoundJob->SamplesToStream);
    }
}

internal inline shader*
GetShaderFromID(game_memory *GameMemory, shader_id ID)
{
    transient_state *TransientState = (transient_state *)GameMemory->TransientStorage.MemoryBlock;
    shader *ShaderSlot = &TransientState->GameAssets->Shaders[ID];
    if(ShaderSlot->ShaderState == AssetState_Loaded)
    {
        return(ShaderSlot);
    }
    else
    {
        return(0);
    }
}

internal inline texture2d*
PushTextureForRendering(game_memory *GameMemory, asset_slot *TextureSlot, texture_id ID)
{
    if(TextureSlot->SlotState == AssetState_Unloaded)
    {
        transient_state *TransientState = (transient_state *)GameMemory->TransientStorage.MemoryBlock;
        TextureSlot->Texture = PushStruct(&TransientState->GameAssets->AssetArena, texture2d);

        load_texture_job *TextureLoadJob = PushStruct(&TransientState->Garbage, load_texture_job);
        TextureLoadJob->TextureSlot = TextureSlot;
        TextureLoadJob->Filepath = TextureFilepaths[ID].Second;
        TextureLoadJob->TransientState = TransientState;

        GameMemory->AddWorkQueueEntry(GameMemory->HighPriorityQueue, LoadTextureCallback, (void *)TextureLoadJob);

        TextureSlot->SlotState = AssetState_Queued;
        return(TextureSlot->Texture);
    }
    else
    {
        return(TextureSlot->Texture);
    }
}

internal inline texture2d*
GetTextureFromID(game_memory *GameMemory, texture_id ID)
{
    transient_state *TransientState = (transient_state *)GameMemory->TransientStorage.MemoryBlock;
    asset_slot *TextureSlot = &TransientState->GameAssets->Textures[ID];
    if(TextureSlot->Texture)
    {
        return(TextureSlot->Texture);
    }
    else if(ID != GT_NullAtlas)
    {
        TextureSlot->Texture = PushTextureForRendering(GameMemory, TextureSlot, ID);
        return(TextureSlot->Texture);
    }
    return(0);
}

internal inline font_data*
CloverLoadFontForUse(game_memory *GameMemory, asset_slot *FontSlot, uint32 Size, font_id ID)
{
    if(FontSlot->SlotState == AssetState_Unloaded)
    {
        transient_state *TransientState = (transient_state *)GameMemory->TransientStorage.MemoryBlock;
        FontSlot->Font = PushStruct(&TransientState->GameAssets->AssetArena, font_data);

        load_font_job *FontJob = PushStruct(&TransientState->Garbage, load_font_job);
        FontJob->Assets = TransientState->GameAssets;
        FontJob->FontSlot = FontSlot;
        FontJob->Filepath = FontFilepaths[ID].Second;
        FontJob->FontSize = Size;

        GameMemory->AddWorkQueueEntry(GameMemory->HighPriorityQueue, LoadFontDataCallback, (void *)FontJob);
        FontSlot->SlotState = AssetState_Queued;

        return(FontSlot->Font);
    }
    else
    {
        return(FontSlot->Font);
    }
}

internal font_data*
GetFontFromID(game_memory *GameMemory, int32 Size, font_id ID)
{
    transient_state *TransientState = (transient_state *)GameMemory->TransientStorage.MemoryBlock;
    asset_slot *FontSlot = &TransientState->GameAssets->Fonts[ID];
    if(FontSlot->Font)
    {
        return(FontSlot->Font);
    }
    else if(ID != GF_NullFont)
    {
        FontSlot->Font = CloverLoadFontForUse(GameMemory, FontSlot, Size, ID);
        return(FontSlot->Font);
    }
    return(0);
}

internal loaded_sound*
LoadSoundFromID(game_memory *GameMemory, asset_slot *SoundSlot, soundfx_id ID, 
                int32 StreamingSampleIndex = 0, int32 SamplesToStream = 0)
{
    transient_state *TransientState = (transient_state *)GameMemory->TransientStorage.MemoryBlock;
    game_state *GameState = (game_state *)GameMemory->PermanentStorage.MemoryBlock;
    SoundSlot->Sound = PushStruct(&TransientState->GameAssets->AssetArena, loaded_sound);

    load_sound_job *LoadSoundJob = PushStruct(&TransientState->Garbage, load_sound_job);
    if(SoundSlot->SlotState == AssetState_Unloaded)
    {
        LoadSoundJob->GameState      = GameState;
        LoadSoundJob->SoundSlot      = SoundSlot;
        LoadSoundJob->Filepath       = SoundFilepaths[ID].Second;
        LoadSoundJob->SoundSlot->Sound->ID                   = ID;

        GameMemory->AddWorkQueueEntry(GameMemory->HighPriorityQueue, LoadSoundDataCallback, (void *)LoadSoundJob);
        SoundSlot->SlotState = AssetState_Queued;
    }

    return(SoundSlot->Sound);
}

internal loaded_sound*
GetSoundFromID(game_memory *GameMemory, soundfx_id ID,
               int32 StartingIndex = 0, int32 SamplesToStream = 0)
{
    transient_state *TransientState = (transient_state *)GameMemory->TransientStorage.MemoryBlock;
    asset_slot *SoundSlot = &TransientState->GameAssets->Sounds[ID];
    if(ID != GSFX_NullSound && SoundSlot->SlotState != AssetState_Loaded)
    {
        SoundSlot->Sound = LoadSoundFromID(GameMemory, SoundSlot, ID, 
                                           StartingIndex, SamplesToStream);
    }
    return(SoundSlot->Sound);
}

#if 0
internal loaded_sound *
DEBUGLoadSoundFromID(game_memory *GameMemory, asset_slot *SoundSlot, playing_sound *PlayingSound,
                     int32 StreamingIndex = 0, int32 SamplesToStream = 0)
{
    transient_state *TransientState = (transient_state *)GameMemory->TransientStorage.MemoryBlock;
    game_state *GameState = (game_state *)GameMemory->PermanentStorage.MemoryBlock;
    SoundSlot->Sound = PushStruct(&TransientState->GameAssets->AssetArena, loaded_sound);

    load_sound_job *LoadSoundJob = PushStruct(&TransientState->Garbage, load_sound_job);
    if(SoundSlot->SlotState == AssetState_Unloaded)
    {
        LoadSoundJob->GameState      = GameState;
        LoadSoundJob->SoundSlot      = SoundSlot;
        LoadSoundJob->Filepath       = SoundFilepaths[ID].Second;
        LoadSoundJob->SoundSlot->Sound->ID                   = ID;

        GameMemory->AddWorkQueueEntry(GameMemory->HighPriorityQueue, LoadSoundDataCallback, (void *)LoadSoundJob);
        SoundSlot->SlotState = AssetState_Queued;
    }

    return(SoundSlot->Sound);
}

internal loaded_sound *
DEBUGGetSoundFromID(game_memory *GameMemory, playing_sound *PlayingSound,
                    int32 StartingIndex = 0, int32 SamplesToStream = 0)
{
    transient_state *TransientState = (transient_state *)GameMemory->TransientStorage.MemoryBlock;
    asset_slot *SoundSlot = &TransientState->GameAssets->Sounds[PlayingSound->ID];
    if(PlayingSound->ID != GSFX_NullSound && SoundSlot->SlotState != AssetState_Loaded)
    {
        SoundSlot->Sound = DEBUGLoadSoundFromID(GameMemory, SoundSlot, PlayingSound, StartingIndex, SamplesToStream);
    }
    return(SoundSlot->Sound);
}
#endif
