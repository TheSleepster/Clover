/* date = August 27 2024 06:00 am*/

#ifndef CLOVER_H
#define CLOVER_H

#include "Intrinsics.h"

#include "util/Math.h"
#include "util/Array.h"
#include "util/FileIO.h"
#include "util/String.h"
#include "util/Pairs.h"

#include "Clover_Input.h"
#include "Clover_Renderer.h"
#include "Clover_UI.h"
#include "Clover_Asset.h"
#include "Clover_Audio.h"

struct box2D : range_v2
{
    bool32 IsActive;
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
    SPRITE_SapphireOre          = 9,
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
    SPRITE_Workbench           = 20, 
    SPRITE_Furnace             = 21, 
    SPRITE_Outline             = 22,
    SPRITE_Count
};

enum item_id
{
    ITEM_Nil = 0,
    
    // CRAFTABLE
    ITEM_Pebbles           = 1,
    ITEM_Branches          = 2,
    ITEM_Trunk             = 3,
    ITEM_ToolPickaxe       = 4,
    ITEM_ToolWoodAxe       = 5,
    ITEM_Workbench         = 6,
    ITEM_Furnace           = 7,
    
    // NON-CRAFTABLE
    ITEM_RubyOreChunk      = 8,
    ITEM_SapphireOreChunk  = 9,
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
    IS_BUILDABLE            = 1 << 10,
    IS_PLACED               = 1 << 11,
    IS_INTERACTING          = 1 << 12,
    ENTITY_FLAGS_COUNT
};

enum entity_arch_id
{
    ARCH_Nil = 0,
    ARCH_Player,
    ARCH_Rock,
    ARCH_Tree00,
    ARCH_Tree01,
    ARCH_SapphireNode,
    ARCH_RubyNode,
    ARCH_Workbench,
    ARCH_Furnace,

    // ITEMS
    ARCH_Pebbles,
    ARCH_Branches,
    ARCH_Trunk,
    ARCH_SapphireOreChunk,
    ARCH_RubyOreChunk,
    ARCH_SimplePickaxe,
    ARCH_SimpleWoodAxe,

    ARCH_ID_MAX,
};

enum game_ui_state
{
    UI_State_Nil,
    UI_State_Building,
    UI_State_Crafting,
};

struct crafting_material
{
    item_id CraftingMaterial;
    int32   RequiredCount;
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
    
    crafting_material CraftingFormula[MAX_CRAFTING_ELEMENTS];
    int32 UniqueMaterialCount;
    int32 FormulaResultCount;
    
    bool32 Craftable;
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

struct entity_item_drop
{
    item_id DroppedItem;
    int32 DropAmount;
};

struct entity_sound
{
    bool32 	   	   IsActive;
    soundfx_id     IDToPlay;
    playing_sound *Sound;
};

struct entity
{
    int32                 EntityID;
    uint32                Archetype;
    uint32                Flags;
    sprite_type           Sprite;


    vec2                  Position;
    vec2                  Target;

	vec2 				  Size;
    real32				  Rotation;
    real32 				  Speed;

    uint32 				  Health;

    range_v2	 		  SelectionBox;
    range_v2 			  BoxCollider;

    entity_sound 		  WhenStruck;

    item_id               DroppedFromInventoryItemID;
    int32                 DroppedFromInventoryItemCount;

    entity_item_drop 	  EntityDrops[MAX_ENTITY_DROPS];
    int32 				  UniqueDropCount;

    entity_item_inventory Inventory;
};

struct persisant_game_data 
{ 
    static_sprite_data          Sprites[SPRITE_Count];
    item                        GameItems[ITEM_IDCount];
    pair <item_id, sprite_type> ItemSprites[ITEM_IDCount];

    entity                      DefaultEntityData[ARCH_ID_MAX];
};

struct game_world_data
{
    memory_arena WorldArena;

    entity *Entities;  
    item   *Items;
    uint32 EntityCounter;
};

struct game_state
{
    bool32            IsInitialized;
    
    input             GameInput;
    
    clover_ui_context UIContext;
    game_ui_state     GameUIState;
    
    // TODO(Sleepster): Get Rid of these bool32s 
    bool32            DisplayPlayerHotbar;
    bool32            DisplayPlayerInventory;
    bool32            DisplayCraftingMenu;
    bool32            DisplayBuildMenu;
    bool32            DrawDebug;
    
    entity           *ActiveCraftingStation;
    item             *ActiveRecipe;
    item             *ActiveBlueprint;

    clover_audio_state AudioState;

    persisant_game_data GameData;
    game_world_data    	World;
};

struct transient_state
{
    memory_arena TransientArena;
    memory_arena Garbage;

    game_assets *GameAssets;

    entity *SelectedEntityThisFrame;
    // RENDERER
    struct
    {
        vertex     *Vertices;
        vertex     *VertexBufferptr;
        vertex     *TransparentVertexBufferptr;

        uint32      OpaqueQuadCount;
        uint32      TransparentQuadCount;
        uint32      LastFrameQuadCount;

        vertex     *UIVertices;
        vertex     *UIVertexBufferptr;
        vertex     *TransparentUIVertexBufferptr;

        uint32      OpaqueUIElementCount;
        uint32      TransparentUIElementCount;

        uint32      TotalQuadCount;
        uint32      TotalUIElementCount;

        point_light PointLights[MAX_POINT_LIGHTS];
        spot_light  SpotLights [MAX_SPOT_LIGHTS];

        int32       PointLightCount;
        int32       SpotLightCount;
    }DrawFrameData;
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

internal inline bool32
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

// NOTE(Sleepster): EASINGS
internal inline real32
EaseOutQuad(real32 X)
{
    return(1 - (1 - X) * (1 - X));
}

internal inline real32
SinBreatheNormalized(real32 Time, real32 Modifier, real32 Min, real32 Max)
{
    real32 SineValue = (sinf(Modifier * 2 * PI32 * Time) + 1.0f) / 2.0f;
    return(Min + (Max - Min) * SineValue);
}

internal inline real32
SinBreathe(real32 Time, real32 Modifier)
{
    return(sinf(Time * Modifier));
}

bool32
operator!=(static_sprite_data A, static_sprite_data B)
{
    bool32 Result = {};
    v2Cast(A.AtlasOffset) != v2Cast(B.AtlasOffset) ? Result = true : Result = false;
    
    return(Result);
}

#endif // _CLOVER_H
