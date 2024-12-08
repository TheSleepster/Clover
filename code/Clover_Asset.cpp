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
    transient_state  *TransientState;
    asset_slot       *SoundSlot;

    uint32            FirstSampleIndex;
    uint32            SamplesToRead;

    int32             SoundID;
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

    AtomicCompareExchangei32((volatile int32 *)&SoundJob->SoundSlot->SlotState, AssetState_Loaded, AssetState_Queued);
    if(SoundJob->SoundSlot->SlotState == AssetState_Loaded)
    {
        CloverLoadWAVFile(&SoundJob->TransientState->GameAssets->AssetArena, 
                           SoundJob->SoundSlot, SoundJob->Filepath, SoundJob->FirstSampleIndex, SoundJob->SamplesToRead);
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
GetFontFromID(game_memory *GameMemory, uint32 Size, font_id ID)
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

constexpr uint32 TWO_SECONDS = 48000*2;

internal loaded_sound*
LoadSoundFromID(game_memory *GameMemory, soundfx_id ID, 
                bool32 IsStreamed = 0, uint32 FirstSampleToRead = 0, uint32 SamplesToRead = 0)
{
    transient_state *TransientState = (transient_state *)GameMemory->TransientStorage.MemoryBlock;
    asset_slot *SoundSlot = &TransientState->GameAssets->Sounds[ID];
    SoundSlot->Sound = PushStruct(&TransientState->GameAssets->AssetArena, loaded_sound);

    if(SoundSlot->SlotState == AssetState_Unloaded && ID != GSFX_NullSound)
    {
        load_sound_job *SoundJob       = PushStruct(&TransientState->TransientArena, load_sound_job);
        SoundJob->SoundSlot            = SoundSlot;
        SoundJob->TransientState       = TransientState;
        SoundJob->Filepath             = SoundFilepaths[ID].Second;
        SoundJob->SoundID              = ID;
        if(IsStreamed)
        {
            SoundJob->FirstSampleIndex = FirstSampleToRead;
            SoundJob->SamplesToRead    = SamplesToRead;
        }
        

        SoundSlot->SlotState = AssetState_Queued;
        GameMemory->AddWorkQueueEntry(GameMemory->HighPriorityQueue, LoadSoundDataCallback, (void *)SoundJob);
    }
    return(SoundSlot->Sound);
}

internal loaded_sound*
GetSoundFromID(transient_state *TransientState, soundfx_id ID)
{
    asset_slot *SoundSlot = &TransientState->GameAssets->Sounds[ID];
    if(SoundSlot->Sound)
    {
        return(SoundSlot->Sound);
    }
    else
    {
        return(0);
    }
}
