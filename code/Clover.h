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
#include "Clover_UI.h"

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
    SPRITE_Nil                 = 0,
    SPRITE_Player              = 1,
    SPRITE_Rock                = 2,
    SPRITE_Tree00              = 3,
    SPRITE_Tree01              = 4,
    SPRITE_Pebbles             = 5,
    SPRITE_Branches            = 6,
    SPRITE_Trunk               = 7,
    SPRITE_RubyOre             = 8,
    SPRITE_SaphireOre          = 9,
    SPRITE_RubyChunk           = 10,
    SPRITE_SapphireChunk       = 11,
    SPRITE_UIItemBox           = 12,
    SPRITE_ToolPickaxe         = 13,
    SPRITE_ToolWoodAxe         = 14,
    SPRITE_FullHeartContainer  = 15,
    SPRITE_HalfHeartContainer  = 16,
    SPRITE_EmptyHeartContainer = 17,
    SPRITE_TestEnemyUnit       = 18,
    SPRITE_SelectionBox        = 19,
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
    IS_VALID                = 1 << 0,
    IS_SOLID                = 1 << 1,
    IS_ACTOR                = 1 << 2,
    IS_TILE                 = 1 << 3,
    IS_ACTIVE               = 1 << 4,
    IS_ITEM                 = 1 << 5,
    IS_DESTRUCTABLE         = 1 << 6,
    IS_IN_INVENTORY         = 1 << 7,
    IS_UI                   = 1 << 8,
    CAN_BE_PICKED_UP        = 1 << 9,
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

    int32       OccupiedInventorySlot;

    string      ItemName;
    string      ItemDesc;
};

struct entity_item_inventory
{
    item  Items[TOTAL_INVENTORY_SIZE];
    uint32 CurrentItemCount;
    uint32 LowestAvaliableSlot;

    item *SelectedInventoryItem;
    item *SwapItem;

    item *SelectedHotbarItem;
    
    ui_element *InventorySlotButtons[TOTAL_INVENTORY_SIZE];
    uint32 CurrentInventorySlot;
};

struct crafting_item
{
    item_id Material;
    int32   MaterialCount;
};

struct crafting_formula
{
    crafting_item Materials[MAX_CRAFTING_ELEMENTS];
    string Name;
    string Desc;
};

struct entity
{
    sprite_type Sprite;
    
    uint32      Archetype;
    uint32      Flags;
    uint32      Health;
    
    vec2        Position;
    vec2        Target;
    vec2        Size;

    real32      Speed;
    real32      Rotation;
    
    box2D       SelectionBox;
    box2D       BoxCollider;
    
    entity_item_inventory Inventory;
    uint32      UsedInventorySlots;

    item_id     ItemID;
    int32       DroppedItemCount;
};

enum ui_state
{
    UI_STATE_NONE,
    UI_STATE_CRAFTING_MENU,
};

struct game_state
{
    KeyCodeID KeyCodeLookup[KEY_COUNT];
    Input GameInput;

    clover_ui_context UIContext;
    ui_state UIState;

    bool DisplayPlayerHotbar;
    bool DisplayPlayerInventory;
    bool DisplayCraftingMenu;
    
    bool DrawDebug;

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
    }GameData;
};

// NOTE(Sleepster): Utilities
internal vec2
TransformMouseCoords(mat4 ViewMatrix, mat4 ProjectionMatrix, ivec2 MousePos, ivec4 WindowSizeData)
{
    vec2 TransformedMousePos  = {};
    
    vec2 Ndc = {(MousePos.X / (WindowSizeData.Width * 0.5f)) - 1.0f, 1.0f - (MousePos.Y / (WindowSizeData.Height * 0.5f))};
    vec4 NDCPosition = v2Expand(Ndc, 0.0f, 1.0f);
    
    mat4 InverseProjection = mat4Inverse(ProjectionMatrix);
    mat4 InverseViewMatrix = mat4Inverse(ViewMatrix);
    
    NDCPosition = mat4Transform(InverseProjection, NDCPosition);
    NDCPosition = mat4Transform(InverseViewMatrix, NDCPosition);
    
    TransformedMousePos = {NDCPosition.X, NDCPosition.Y};
    return(TransformedMousePos);
}

// NOTE(Sleepster): AABB Stuff
internal inline range_v2
CreateRange(vec2 Min, vec2 Max)
{
    range_v2 Result = {};
    Result.Min = Min;
    Result.Max = Max;
    
    return(Result);
}

internal inline range_v2
RangeShift(range_v2 Range, vec2 Shift)
{
    range_v2 NewRange = {};
    NewRange.Min = Range.Min + Shift;
    NewRange.Max = Range.Max + Shift;
    
    return(NewRange);
}

internal inline vec2 
MakeCentered(vec2 Position, vec2 Size)
{
    vec2 Centered = {};
    
    Centered.X = Position.X + (Size.X * 0.5f);
    Centered.Y = Position.Y + (Size.Y * 0.5f);
    
    return(Centered);
}

internal inline range_v2
RangeMakeCentered(vec2 Size)
{
    range_v2 CenteredRange = {};
    
    CenteredRange.Max = Size;
    CenteredRange = RangeShift(CenteredRange, vec2{Size.X * -0.5f, 0.0f});
    
    return(CenteredRange);
}

internal inline vec2
RangeSize(range_v2 Range)
{
    vec2 Size = {};
    
    Size = Range.Min - Range.Max;
    Size.X = fabsf(Size.X);
    Size.Y = fabsf(Size.Y);
    
    return(Size);
}

internal inline bool
IsRangeWithinBounds(vec2 Test, range_v2 Bounds)
{
    return (Test.X >= Bounds.Min.X && Test.X <= Bounds.Max.X && 
            Test.Y >= Bounds.Min.Y && Test.Y <= Bounds.Max.Y);
}

internal inline range_v2
RangeFromQuad(quad *Quad)
{
    range_v2 Result = {};
    
    Result.Min = Quad->TopLeft.Position.XY;
    Result.Max = Quad->TopRight.Position.XY;
    
    return(Result);
}

// AABB stuff

internal inline real32
EaseOutQuad(real32 X)
{
    return 1 - (1 - X) * (1 - X);
}

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
