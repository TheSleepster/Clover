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

internal inline texture2d*
GetTextureFromID(asset_manager *AssetManager, texture_id ID)
{
    texture2d *Result = &AssetManager->GameTextures[ID];
    if(Result->IsLoaded)
    {
        return(Result);
    }
    else
    {
        Result->IsLoaded = false;
        Result->LoadRequested = true;
    }
    return(0);
}

internal inline shader*
GetShaderFromID(asset_manager *AssetManager, shader_id ID)
{
    shader *Result = &AssetManager->Shaders[ID];
    if(Result)
    {
        return(Result);
    }
    return(0);
}

internal inline font_data*
GetFontFromID(asset_manager *AssetManager, font_id ID)
{
    font_data *Result = &AssetManager->Fonts[ID];
    if(Result)
    {
        return(Result);
    }
    return(0);
}

internal inline loaded_sound*
GetSoundFromId(asset_manager *AssetManager, soundfx_id ID)
{
    loaded_sound *Result = &AssetManager->Sounds[ID];
    if(Result)
    {
        return(Result);
    }
    return(0);
}
