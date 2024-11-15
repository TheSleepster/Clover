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
    string Filepath;
    texture2d *Texture;
};

struct load_font_job
{
    game_assets *Assets;
    font_data   *Font;

    uint32       FontSize;
    string       Filepath;
};

struct load_sound_job
{
    transient_state *TransientState;
    asset_slot       *SoundSlot;
    string           Filepath;
};

internal
PLATFORM_JOB_ENTRY_CALLBACK(LoadTextureCallback)
{
    load_texture_job *LoadTextureJob = (load_texture_job *)Data;

    ReadWriteBarrier;
    LoadTextureJob->Texture->RawData = (char *)stbi_load((const char *)LoadTextureJob->Filepath.Data, 
                                                       &LoadTextureJob->Texture->TextureData.Width, 
                                                       &LoadTextureJob->Texture->TextureData.Height, 
                                                       &LoadTextureJob->Texture->TextureData.Channels, 
                                                       4);
    LoadTextureJob->Texture->Filepath = LoadTextureJob->Filepath;
}

internal
PLATFORM_JOB_ENTRY_CALLBACK(LoadSoundDataCallback)
{
    load_sound_job *SoundJob = (load_sound_job *)Data;
    WriteBarrier;
    CloverLoadWAVFile(&SoundJob->TransientState->Garbage, SoundJob->SoundSlot->Sound, SoundJob->Filepath);
    SoundJob->SoundSlot->SlotState = AssetState_Loaded;
}

internal
PLATFORM_JOB_ENTRY_CALLBACK(LoadFontDataCallback)
{
    load_font_job *FontJob = (load_font_job *)Data;

    ReadWriteBarrier;
    FontJob->Font->RawData = 
        CloverLoadSDFFontData(FontJob->Assets->TransientState, FontJob->Font, FontJob->Filepath, FontJob->FontSize);
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
        TextureLoadJob->Texture = TextureSlot->Texture;
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
        FontJob->Font = FontSlot->Font;
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
LoadSoundFromID(game_memory *GameMemory, asset_slot *SoundSlot, soundfx_id ID)
{
    if(SoundSlot->SlotState == AssetState_Unloaded)
    {
        transient_state *TransientState = (transient_state *)GameMemory->TransientStorage.MemoryBlock;
        SoundSlot->Sound = PushStruct(&TransientState->GameAssets->AssetArena, loaded_sound);

        load_sound_job *LoadSoundJob = PushStruct(&TransientState->Garbage, load_sound_job);
        LoadSoundJob->TransientState = TransientState;
        LoadSoundJob->SoundSlot      = SoundSlot;
        LoadSoundJob->Filepath       = SoundFilepaths[ID].Second;

        GameMemory->AddWorkQueueEntry(GameMemory->HighPriorityQueue, LoadSoundDataCallback, (void *)LoadSoundJob);
        SoundSlot->SlotState = AssetState_Queued;

        return(SoundSlot->Sound);
    }
    else
    {
        return(SoundSlot->Sound);
    }
}

internal loaded_sound*
GetSoundFromID(game_memory *GameMemory, soundfx_id ID)
{
    transient_state *TransientState = (transient_state *)GameMemory->TransientStorage.MemoryBlock;
    asset_slot *SoundSlot = &TransientState->GameAssets->Sounds[ID];
    if(SoundSlot->Sound && SoundSlot->SlotState == AssetState_Loaded)
    {
        return(SoundSlot->Sound);
    }
    else if(ID != GSFX_NullSound)
    {
        SoundSlot->Sound = LoadSoundFromID(GameMemory, SoundSlot, ID);
        return(SoundSlot->Sound);
    }
    return(0);
}
