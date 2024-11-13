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

internal
PLATFORM_JOB_ENTRY_CALLBACK(LoadTextureCallback)
{
    load_texture_job *TextureData = (load_texture_job *)Data;
    CloverLoadTexture(TextureData->TransientState, TextureData->Texture, TextureData->Filepath);
}

internal inline texture2d*
GetTextureFromID(game_memory *GameMemory, texture_id ID)
{
    transient_state *TransientState = (transient_state *)GameMemory->TransientStorage.MemoryBlock;
    texture2d *Texture = &TransientState->GameAssets.GameTextures[ID];
    if(Texture->Filepath.Data != 0)
    {
        return(Texture);
    }
    else
    {
        load_texture_job TextureData = {};
        TextureData.Filepath = STR("../data/res/textures/TextureAtlas.png");
        TextureData.Texture = Texture;
        TextureData.TransientState = TransientState;

        GameMemory->AddWorkQueueEntry(GameMemory->HighPriorityQueue, LoadTextureCallback, (void *)&TextureData);
        return(Texture);
    }
}
