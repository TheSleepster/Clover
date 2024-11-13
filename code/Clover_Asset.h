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
    GF_UbuntuMono,
    GF_LiterationMono,
    GF_FontIDCount,
};

enum soundfx_id
{
    GSFX_SunkenSeaTheme,
    GSFX_RoarOfTheJungleDragon,
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
        loaded_sound *Sound;
        font_data    *Font;
        shader       *Shader;
    };
};

struct asset
{
    asset_id   ID;
};

struct game_assets
{
    uint32      TextureCount;
    asset_slot *Textures;

    uint32      SoundCount;
    asset_slot *Sounds;

    uint32      FontCount;
    asset_slot *Fonts;

    uint32      ShaderCount;
    asset_slot *Shaders;

    asset_type AssetTypes[ASSET_TYPE_COUNT];
};

struct asset_manager
{
    texture2d    GameTextures[GT_TextureIDCount];
    font_data    Fonts[GF_FontIDCount];
    loaded_sound Sounds[MAX_SOUNDS];
    shader       Shaders[GS_ShaderIDCount];

    uint32 LoadedTextureCount;
};

#endif // CLOVER_ASSET_H

