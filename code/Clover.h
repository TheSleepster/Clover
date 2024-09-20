/* date = August 27 2024 06:00 am*/

#ifndef CLOVER_H
#define CLOVER_H

#include "Intrinsics.h"

#include "util/Math.h"
#include "util/Array.h"
#include "util/FileIO.h"
#include "util/CustomStrings.h"

#include "Clover_Input.h"
#include "Clover_Audio.h"
#include "Clover_Renderer.h"

struct sound_instance
{
    ma_sound Sound;
    bool     IsPlaying;
    bool     IsActive;
};

struct game_memory
{
    memory_arena PermanentStorage;
    memory_arena TemporaryStorage;
};

struct time
{
    real32 Delta;
    real32 Current;
    real64 Alpha;
    real64 Next;
    
    int32  FPSCounter;
    real32 MSPerFrame;
    real32 CurrentTimeInSeconds;
};

struct box2D : range_v2
{
    bool IsActive;
};

enum sprite_type
{
    SPRITE_Nil           = 0,
    SPRITE_Player        = 1,
    SPRITE_Rock          = 2,
    SPRITE_Tree00        = 3,
    SPRITE_Tree01        = 4,
    SPRITE_Pebbles       = 5,
    SPRITE_Branches      = 6,
    SPRITE_Trunk         = 7,
    SPRITE_RubyOre       = 8,
    SPRITE_SaphireOre    = 9,
    SPRITE_RubyChunk     = 10,
    SPRITE_SapphireChunk = 11,
    SPRITE_UIItemBox     = 12,
    SPRITE_ToolPickaxe   = 13,
    SPRITE_ToolWoodAxe   = 14,
    SPRITE_Count
};

enum item_id
{
    ITEM_Nil,
    ITEM_Pebbles,
    ITEM_Branches,
    ITEM_Trunk,
    ITEM_RubyOreChunk,
    ITEM_SapphireOreChunk,
    ITEM_ToolPickaxe,
    ITEM_ToolWoodAxe,
    ITEM_IDCount
};

enum entity_flags
{
    IS_VALID        = 1 << 0,
    IS_SOLID        = 1 << 1,
    IS_ACTOR        = 1 << 2,
    IS_TILE         = 1 << 3,
    IS_ACTIVE       = 1 << 4,
    IS_ITEM         = 1 << 5,
    IS_DESTRUCTABLE = 1 << 6,
    IS_IN_INVENTORY = 1 << 7,
    IS_UI           = 1 << 8,
    ENTITY_FLAGS_COUNT
};

enum entity_arch
{
    NIL    = 0,
    PLAYER = 1,
    ROCK   = 2,
    TREE   = 3,
    NODE   = 4,
    ITEM   = 5,
    UI     = 6,
    COUNT
};

struct item
{
    uint32      Archetype;
    uint32      Flags;
    sprite_type Sprite;
    item_id     ItemID;
    
    int32       MaxStackCount;
    int32       CurrentStack;

    string      ItemName;
    string      ItemDesc;
};

struct entity_item_inventory
{
    item  Items[ITEM_IDCount];
    uint32 CurrentItemCount;
};

struct entity
{
    sprite_type Sprite;
    item_id     ItemID;
    
    uint32      Archetype;
    uint32      Flags;
    uint32      Health;
    
    vec2        Position;
    vec2        Size;
    real32      Speed;
    real32      Rotation;
    
    box2D       SelectionBox;
    box2D       BoxCollider;
    
    uint32      UsedInventorySlots;
    entity_item_inventory Inventory;
};



enum ui_type
{
    UIELEMENT_Nil,
    UIELEMENT_Button,
    UIELEMENT_DescBox,
    UIELEMENT_ElementBox,
    UIElementCount,
};

enum ui_layer : uint16
{
    UI_LAYER_0  = 1 << 0,
    UI_LAYER_1  = 1 << 1,
    UI_LAYER_2  = 1 << 2,
    UI_LAYER_3  = 1 << 3,
    UI_LAYER_4  = 1 << 4,
    UI_LAYER_5  = 1 << 5,
    UI_LAYER_6  = 1 << 6,
    UI_LAYER_7  = 1 << 7,
    UI_LAYER_8  = 1 << 8,
    UI_LAYER_9  = 1 << 9,
    UI_LAYER_10 = 1 << 10,
    UI_LAYER_11 = 1 << 11,
    UI_LAYER_12 = 1 << 12,
    UI_LAYER_13 = 1 << 13,
    UI_LAYER_14 = 1 << 14,
    UI_LAYER_15 = 1 << 15,
};

struct ui_id
{
    int32    ID;
    ui_layer LayerIdx = UI_LAYER_0;
};

struct ui_element
{
    ui_type Type;
    ui_id UIID;  

    static_sprite_data Sprite;
    string ElementText;

    vec2 Position;
    vec2 Size;
    vec4 DrawColor;

    range_v2 OccupiedRange;

    bool IsHot;
    bool IsActive;
    bool IsDisplayed;
    bool Captured;
    
    mat4 XForm;
};



struct game_state
{
    KeyCodeID KeyCodeLookup[KEY_COUNT];
    Input GameInput;
    
    // NOTE(Sleepster): World Data
    struct
    {
        entity Entities[MAX_ENTITIES];  
        item   Items[1000];
        
        uint32 EntityCounter;
        
        struct 
        {
            entity *SelectedEntity;
        }WorldFrame;
    }World;
    
    // NOTE(Sleepster): Audio Stuffs
    struct
    {   
        ma_engine AudioEngine;
        
        sound_instance Instances[MAX_SOUNDS];
        sound_instance SoundTracks[MAX_TRACKS];
        
        uint32 ActiveInstances;
        uint32 LoadedTracks;
    }SFXData;
    
    // NOTE(Sleepster): Visual Assets
    struct 
    { 
        static_sprite_data     Sprites[SPRITE_Count];
        item                   GameItems[ITEM_IDCount];

        struct
        {
            ui_id      HotLastFrame;
            ui_id      ActiveLastFrame;
            uint32     ElementCount;
            ui_element UIElements[MAX_UI_ELEMENTS];
        }UIState;
    }GameData;
};

#define GAME_ON_AWAKE(name) void name(game_memory *Memory, gl_render_data *RenderData, game_state *State)
typedef GAME_ON_AWAKE(game_on_awake);
GAME_ON_AWAKE(GameOnAwakeStub)
{
}

#define GAME_FIXED_UPDATE(name) void name(game_memory *Memory, gl_render_data *RenderData, game_state *State, time Time)
typedef GAME_FIXED_UPDATE(game_fixed_update);
GAME_FIXED_UPDATE(GameFixedUpdateStub)
{
}

#define GAME_UPDATE_AND_DRAW(name) void name(game_memory *Memory, gl_render_data *RenderData, game_state *State, time Time, ivec4 SizeData)
typedef GAME_UPDATE_AND_DRAW(game_update_and_draw);
GAME_UPDATE_AND_DRAW(GameUpdateAndDrawStub)
{
}

struct game_functions
{
    HMODULE  GameCodeDLL;
    FILETIME LastWriteTime;
    
    game_on_awake          *OnAwake;
    game_fixed_update      *FixedUpdate;
    game_update_and_draw   *UpdateAndDraw;
    
    bool IsLoaded;
    bool IsValid;
};

bool
operator!=(static_sprite_data A, static_sprite_data B)
{
    bool Result = {};
    v2Cast(A.AtlasOffset) != v2Cast(B.AtlasOffset) ? Result = true : Result = false;

    return(Result);
}

#endif // _CLOVER_H
