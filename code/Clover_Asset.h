#if !defined(CLOVER_ASSET_H)
/* ========================================================================
   $File: Clover_Asset.h $
   $Date: November 11 2024 01:46 pm $
   $Revision: $
   $Creator: Justin Lewis $
   ======================================================================== */

#define CLOVER_ASSET_H

#include "Clover_Input.h"
#include "Clover_Renderer.h"
#include "Clover_UI.h"
#include "Clover_Audio.h"

typedef uint64 asset_id;

enum asset_type
{
    ASSET_GAME_TEXTURE,
    ASSET_FONT,
    ASSET_SOUND,
    ASSET_SHADER,
    ASSET_TYPE_COUNT,
};

enum shader_id
{
    GS_NullShader,
    GS_BasicShader,
    GS_GBufferShader,
    GS_LightingShader,
    GS_ShaderIDCount,
};

enum texture_id
{
    GT_NullAtlas,
    GT_GameAtlas,
    GT_TextureIDCount,
};

enum font_id
{
    GF_NullFont,
    GF_UbuntuMono,
    GF_LiberationMono,
    GF_FontIDCount,
};

enum soundfx_id
{
    GSFX_NullSound,
    GSFX_SunkenSeaTheme,
    GSFX_RoarOfTheJungleDragon,
    GSFX_Boop,
    GSFX_Bap,
    GSFX_IDCount,
};

enum asset_state
{
    AssetState_Unloaded,
    AssetState_Queued,
    AssetState_Loaded,
};

struct asset_slot
{
    asset_state SlotState;
    union
    {
        texture2d    *Texture;
        font_data    *Font;
        loaded_sound *Sound;
        shader       *Shader;
    };
};

struct asset
{
    asset_id   ID;
};


// NOTE(Sleepster): There's only 2 ways to do our asset streaming, either set it like so, a stack array.
//                  Or to make it so that "TextureCount" is the amount that we need for a given scene, then dynamically allocate.
struct game_assets
{
    struct transient_state *TransientState;
    memory_arena            AssetArena;

    uint32                  TextureCount;
    asset_slot              Textures[GT_TextureIDCount];

    uint32                  FontCount;
    asset_slot              Fonts[GF_FontIDCount];

    uint32                  ShaderCount;
    shader                  Shaders[GS_ShaderIDCount];

    uint32                  SoundCount;
    asset_slot              Sounds[GSFX_IDCount];
};

bool IsValid(uint32 ID)
{
    return(ID != GSFX_NullSound);
}

const pair<uint32, string> TextureFilepaths[] = 
{
    pair<uint32, string>(GT_NullAtlas,               STR("")),
    pair<uint32, string>(GT_GameAtlas,               STR("../data/res/textures/TextureAtlas.png")),
};

const pair<uint32, string> FontFilepaths[] =
{
    pair<uint32, string>(GF_NullFont,                STR("")),
    pair<uint32, string>(GF_UbuntuMono,              STR("../data/res/fonts/UbuntuMono-B.ttf")),
    pair<uint32, string>(GF_LiberationMono,          STR("../data/res/fonts/LiberationMono-Regular.ttf")),
};

const pair<uint32, string>SoundFilepaths[] = 
{
    pair<uint32, string>(GSFX_NullSound,             STR("")),
    pair<uint32, string>(GSFX_SunkenSeaTheme,        STR("../data/res/sounds/Test.wav")),
    pair<uint32, string>(GSFX_RoarOfTheJungleDragon, STR("../data/res/sounds/Test2.wav")),
    pair<uint32, string>(GSFX_Boop,                  STR("../data/res/sounds/boop.wav")),
    pair<uint32, string>(GSFX_Bap,                   STR("../data/res/sounds/BAP.wav")),
};

#endif // CLOVER_ASSET_H

