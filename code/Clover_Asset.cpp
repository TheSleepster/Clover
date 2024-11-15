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

struct load_sound_job
{
    transient_state *TransientState;
    loaded_sound *Sound;
    string Filepath;
};

struct load_font_job
{
    game_assets *Assets;
    font_data   *Font;

    uint32       FontSize;
    string       Filepath;
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
PLATFORM_JOB_ENTRY_CALLBACK(LoadSoundCallback)
{
    load_sound_job *SoundJob = (load_sound_job *)Data;
    WriteBarrier;
    CloverLoadWAVFile(&SoundJob->TransientState->Garbage, SoundJob->Sound, SoundJob->Filepath);
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

internal inline void
CloverLoadAssetTexture(game_memory *GameMemory, asset_slot *TextureSlot, texture_id ID)
{
    transient_state *TransientState = (transient_state *)GameMemory->TransientStorage.MemoryBlock;

    TextureSlot->Texture = PushStruct(&TransientState->GameAssets->AssetArena, texture2d);

    load_texture_job *TextureLoadJob = PushStruct(&TransientState->Garbage, load_texture_job);
    TextureLoadJob->Texture = TextureSlot->Texture;
    TextureLoadJob->Filepath = TextureFilepaths[ID].Second;
    TextureLoadJob->TransientState = TransientState;

    GameMemory->AddWorkQueueEntry(GameMemory->HighPriorityQueue, LoadTextureCallback, (void *)TextureLoadJob);
}

internal inline texture2d*
PushTextureForRendering(game_memory *GameMemory, texture_id ID)
{
    transient_state *TransientState = (transient_state *)GameMemory->TransientStorage.MemoryBlock;
    asset_slot *TextureSlot = &TransientState->GameAssets->Textures[ID];

    CloverLoadAssetTexture(GameMemory, TextureSlot, ID);
    TextureSlot->SlotState = AssetState_Queued;
    return(TextureSlot->Texture);
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
    else
    {
        TextureSlot->Texture = PushTextureForRendering(GameMemory, ID);
        return(TextureSlot->Texture);
    }
}

    
internal void
CloverLoadAssetFont(game_memory *GameMemory, asset_slot *FontSlot, uint32 Size, font_id ID)
{
    transient_state *TransientState = (transient_state *)GameMemory->TransientStorage.MemoryBlock;
    FontSlot->Font = PushStruct(&TransientState->GameAssets->AssetArena, font_data);

    load_font_job *FontJob = PushStruct(&TransientState->Garbage, load_font_job);
    FontJob->Assets = TransientState->GameAssets;
    FontJob->Font = FontSlot->Font;
    FontJob->Filepath = FontFilepaths[ID].Second;
    FontJob->FontSize = Size;

    GameMemory->AddWorkQueueEntry(GameMemory->HighPriorityQueue, LoadFontDataCallback, (void *)FontJob);
}

internal inline font_data*
PushFontForUse(game_memory *GameMemory, uint32 Size, font_id ID)
{
    transient_state *TransientState = (transient_state *)GameMemory->TransientStorage.MemoryBlock;
    asset_slot *FontSlot = &TransientState->GameAssets->Fonts[ID];
    if(FontSlot->Font && FontSlot->SlotState == AssetState_Loaded)
    {
        return(FontSlot->Font);
    }
    else
    {
        CloverLoadAssetFont(GameMemory, FontSlot, Size, ID);
        FontSlot->SlotState = AssetState_Queued;
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
    else
    {
        FontSlot->Font = PushFontForUse(GameMemory, Size, ID);
        return(FontSlot->Font);
    }
}

internal inline void
PushSound(game_assets *Assets, soundfx_id ID)
{
}

internal inline shader *
PushShader(game_assets *Assets, shader_id ID)
{
}
