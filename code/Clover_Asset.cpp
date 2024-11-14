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
}

internal
PLATFORM_JOB_ENTRY_CALLBACK(LoadSoundCallback)
{
    load_sound_job *SoundJob = (load_sound_job *)Data;
    CloverLoadWAVFile(&SoundJob->TransientState->Garbage, SoundJob->Sound, SoundJob->Filepath);
}

internal void
CloverLoadQueuedGLAssets(game_memory *GameMemory)
{
}

internal inline shader*
GetShaderFromID(game_assets *Assets, shader_id ID)
{
    shader *ShaderSlot = &Assets->Shaders[ID];
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
CloverLoadAssetTexture(game_assets *Assets, asset_slot *TextureSlot, texture_id ID)
{
    TextureSlot->Texture = PushStruct(&Assets->AssetArena, texture2d);
    CloverLoadTexture(Assets->TransientState, TextureSlot->Texture, TextureFilepaths[ID].Second);
}

internal inline texture2d*
PushTextureForRendering(game_assets *Assets, texture_id ID)
{
    asset_slot *TextureSlot = &Assets->Textures[ID];

    CloverLoadAssetTexture(Assets, TextureSlot, ID);
    TextureSlot->SlotState = AssetState_Loaded;
    return(TextureSlot->Texture);
}

internal inline texture2d*
GetTextureFromID(game_assets *Assets, texture_id ID)
{
    asset_slot *TextureSlot = &Assets->Textures[ID];
    if(TextureSlot->Texture)
    {
        return(TextureSlot->Texture);
    }
    else
    {
        TextureSlot->Texture = PushTextureForRendering(Assets, ID);
        return(TextureSlot->Texture);
    }
}

internal void
CloverLoadAssetFont(game_assets *Assets, asset_slot *FontSlot, uint32 Size, font_id ID)
{
    FontSlot->Font = PushStruct(&Assets->AssetArena, font_data);
    CloverLoadSDFFont(Assets->TransientState, FontSlot->Font, FontFilepaths[ID].Second, Size);
}

internal inline font_data*
PushFontForUse(game_assets *Assets, uint32 Size, font_id ID)
{
    asset_slot *FontSlot = &Assets->Fonts[ID];
    if(FontSlot->Font && FontSlot->SlotState == AssetState_Loaded)
    {
        return(FontSlot->Font);
    }
    else
    {
        CloverLoadAssetFont(Assets, FontSlot, Size, ID);
        FontSlot->SlotState = AssetState_Loaded;
        return(FontSlot->Font);
    }
}

internal font_data*
GetFontFromID(game_assets *Assets, int32 Size, font_id ID)
{
    asset_slot *FontSlot = &Assets->Fonts[ID];
	if(FontSlot->Font)
    {
        return(FontSlot->Font);
    }
    else
    {
        FontSlot->Font = PushFontForUse(Assets, Size, ID);
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
