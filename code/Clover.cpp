/* =======================================================================
   $File: Clover.cpp$ 
   $Date: September 09 2024 04:51 pm $
   $Revision: $
   $Creator: Justin Lewis $
   ======================================================================== */

// NOTE(Sleepster): Freetype must come first due to the #define internal static inside of the intrinsics header
#include "../data/deps/Freetype/include/ft2build.h"
#include FT_FREETYPE_H

// TODO(Sleepster): Maybe make it so that this doesn't have to be exposed to the game layer
#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#include "../data/deps/stb/stb_image.h"
#endif

#include "Intrinsics.h"

// UTILS
#include "util/Math.h"
#include "util/Array.h"
#include "util/FileIO.h"
#include "util/String.h"
#include "util/Pairs.h"

// CLOVER HEADERS
#include "Clover.h"
#include "Clover_Platform.h"
#include "Clover_Globals.h"
#include "Clover_Input.h" 
#include "Clover_Renderer.h"
#include "Clover_Audio.h"
#include "Clover_UI.h"
#include "Clover_Asset.h"
#include "shader/CommonShader.glh"

#include "Clover_Input.cpp"
#include "Clover_Draw.cpp"
#include "Clover_UI.cpp"
#include "Clover_Audio.cpp"
#include "Clover_Asset.cpp"


global_variable entity *Player = {};


internal inline void
LoadSpriteData(game_state *GameState)
{
    GameState->GameData.Sprites[SPRITE_Nil]                   = {.AtlasOffset = {  0,  0}, .SpriteSize = {16, 16}};
    GameState->GameData.Sprites[SPRITE_Player]                = {.AtlasOffset = { 17,  0}, .SpriteSize = {12, 11}};
    GameState->GameData.Sprites[SPRITE_UIItemBox]             = {.AtlasOffset = {112,  0}, .SpriteSize = {16, 16}};
    GameState->GameData.Sprites[SPRITE_ToolPickaxe]           = {.AtlasOffset = {  0, 48}, .SpriteSize = {11, 13}};
    GameState->GameData.Sprites[SPRITE_ToolWoodAxe]           = {.AtlasOffset = { 16, 48}, .SpriteSize = {11, 13}};
    GameState->GameData.Sprites[SPRITE_FullHeartContainer]    = {.AtlasOffset = {  0, 64}, .SpriteSize = {11,  9}};
    GameState->GameData.Sprites[SPRITE_HalfHeartContainer]    = {.AtlasOffset = { 16, 64}, .SpriteSize = {11,  9}};
    GameState->GameData.Sprites[SPRITE_EmptyHeartContainer]   = {.AtlasOffset = { 32, 64}, .SpriteSize = {11,  9}};
    GameState->GameData.Sprites[SPRITE_TestEnemyUnit]         = {.AtlasOffset = { 16, 16}, .SpriteSize = {13, 11}};
    GameState->GameData.Sprites[SPRITE_SelectionBox]          = {.AtlasOffset = {112, 16}, .SpriteSize = {16, 16}};
    GameState->GameData.Sprites[SPRITE_Workbench]             = {.AtlasOffset = { 16, 80}, .SpriteSize = {16, 16}};
    GameState->GameData.Sprites[SPRITE_Furnace]               = {.AtlasOffset = {  0, 80}, .SpriteSize = {16, 16}};
    GameState->GameData.Sprites[SPRITE_Outline]               = {.AtlasOffset = {128,  0}, .SpriteSize = {16, 16}};
    
    GameState->GameData.Sprites[SPRITE_Rock]                  = {.AtlasOffset = { 48, 35}, .SpriteSize = {16, 13}};
    GameState->GameData.Sprites[SPRITE_Pebbles]               = {.AtlasOffset = { 48, 48}, .SpriteSize = {13, 12}};
    GameState->GameData.Sprites[SPRITE_Tree00]                = {.AtlasOffset = { 96, 31}, .SpriteSize = {16, 17}};
    GameState->GameData.Sprites[SPRITE_Branches]              = {.AtlasOffset = { 96, 48}, .SpriteSize = {12, 12}};
    GameState->GameData.Sprites[SPRITE_Tree01]                = {.AtlasOffset = {130, 27}, .SpriteSize = {18, 21}};
    GameState->GameData.Sprites[SPRITE_Trunk]                 = {.AtlasOffset = {128, 48}, .SpriteSize = {11, 11}};
    GameState->GameData.Sprites[SPRITE_RubyOre]               = {.AtlasOffset = { 64, 35}, .SpriteSize = {16, 13}};
    GameState->GameData.Sprites[SPRITE_RubyChunk]             = {.AtlasOffset = { 64, 48}, .SpriteSize = {13, 12}};
    GameState->GameData.Sprites[SPRITE_SapphireOre]           = {.AtlasOffset = { 80, 35}, .SpriteSize = {16, 13}};
    GameState->GameData.Sprites[SPRITE_SapphireChunk]         = {.AtlasOffset = { 80, 48}, .SpriteSize = {16, 13}};
}

internal inline void
LoadItemData(game_state *GameState)
{
    GameState->GameData.GameItems[ITEM_Nil] = {};
    GameState->GameData.GameItems[ITEM_Pebbles] = 
    {
        .Archetype = ARCH_Pebbles,      
        .Flags = IS_VALID|IS_ITEM|IS_IN_INVENTORY, 
        .Sprite = SPRITE_Pebbles,       
        .ItemID = ITEM_Pebbles,          
        .MaxStackCount = 64, 
        .ItemName = STR("Pebbles"),         
        .ItemDesc = STR("These are some pebbles!"),
        .CraftingFormula = 
        {
            {ITEM_SapphireOreChunk, 1}, 
        },
        .UniqueMaterialCount = 1,
        .FormulaResultCount  = 3,
        .Craftable = true,
    };
    
    GameState->GameData.GameItems[ITEM_Branches] = 
    {
        .Archetype = ARCH_Branches,     
        .Flags = IS_VALID|IS_ITEM|IS_IN_INVENTORY, 
        .Sprite = SPRITE_Branches,      
        .ItemID = ITEM_Branches,         
        .MaxStackCount = 64,
        .ItemName = STR("Branches"),        
        .ItemDesc = STR("These are some branches!"),
        .CraftingFormula = 
        {
            {ITEM_Trunk, 1},
        },
        .UniqueMaterialCount = 1,
        .FormulaResultCount  = 3,
        .Craftable = true,
    };
    
    GameState->GameData.GameItems[ITEM_Trunk] = 
    {
        .Archetype = ARCH_Trunk,      
        .Flags = IS_VALID|IS_ITEM|IS_IN_INVENTORY, 
        .Sprite = SPRITE_Trunk,         
        .ItemID = ITEM_Trunk,            
        .MaxStackCount = 64, 
        .ItemName = STR("Pine Logs"),       
        .ItemDesc = STR("This is a bundle of logs!"),
        .CraftingFormula = 
        {
            {ITEM_Branches, 3}
        },
        .UniqueMaterialCount = 1,
        .FormulaResultCount  = 1,
        .Craftable = true,
    };
    
    GameState->GameData.GameItems[ITEM_RubyOreChunk] = 
    {
        .Archetype = ARCH_RubyOreChunk,
        .Flags = IS_VALID|IS_ITEM|IS_IN_INVENTORY, 
        .Sprite = SPRITE_RubyChunk,     
        .ItemID = ITEM_RubyOreChunk,     
        .MaxStackCount = 64, 
        .ItemName = STR("Ruby Chunks"),     
        .ItemDesc = STR("These are chunks of Ruby Rock!"),
        .Craftable = false,
    };
    
    GameState->GameData.GameItems[ITEM_SapphireOreChunk] = 
    {
        .Archetype = ARCH_SapphireOreChunk,
        .Flags = IS_VALID|IS_ITEM|IS_IN_INVENTORY, 
        .Sprite = SPRITE_SapphireChunk, 
        .ItemID = ITEM_SapphireOreChunk, 
        .MaxStackCount = 64, 
        .ItemName = STR("Sapphire Chunks"), 
        .ItemDesc = STR("These are some chunks of Sapphire Rock!"),
        .Craftable = false,
    };
    
    GameState->GameData.GameItems[ITEM_ToolPickaxe] = 
    {
        .Archetype = ARCH_SimplePickaxe,      
        .Flags = IS_VALID|IS_ITEM|IS_IN_INVENTORY, 
        .Sprite = SPRITE_ToolPickaxe,   
        .ItemID = ITEM_ToolPickaxe,      
        .MaxStackCount = 1,  
        .ItemName = STR("Simple Pickaxe"),  
        .ItemDesc = STR("This a pickaxe, It can be used to mine ores!"),
        .CraftingFormula = 
        {
            {ITEM_Branches, 1},
            {ITEM_Pebbles, 2},
        },
        .UniqueMaterialCount = 2,
        .FormulaResultCount  = 1,
        .Craftable = true,
    };
    
    GameState->GameData.GameItems[ITEM_ToolWoodAxe] = 
    {
        .Archetype = ARCH_SimpleWoodAxe,
        .Flags = IS_VALID|IS_ITEM|IS_IN_INVENTORY, 
        .Sprite = SPRITE_ToolWoodAxe,   
        .ItemID = ITEM_ToolWoodAxe,      
        .MaxStackCount = 1,  
        .ItemName = STR("Simple Wood Axe"), 
        .ItemDesc = STR("This is a wood axe, It can be used to cut trees!"),
        .CraftingFormula = 
        {
            {ITEM_Branches, 1},
            {ITEM_Pebbles, 2},
        },
        .UniqueMaterialCount = 2,
        .FormulaResultCount  = 1,
        .Craftable = true,
    };
    
    GameState->GameData.GameItems[ITEM_Workbench] = 
    {
        .Archetype = ARCH_Workbench,  
        .Flags = IS_VALID|IS_BUILDABLE,            
        .Sprite = SPRITE_Workbench,   
        .ItemID = ITEM_Workbench,      
        .MaxStackCount = 1,  
        .ItemName = STR("Workbench"), 
        .ItemDesc = STR("This is a Workbench, it is used for crafting!"),
        .CraftingFormula = 
        {
            {ITEM_Trunk, 2},
            {ITEM_Branches, 3},
            {ITEM_Pebbles, 2},
        },
        .UniqueMaterialCount = 3,
        .FormulaResultCount  = 1,
        .Craftable = true,
    };
    
    GameState->GameData.GameItems[ITEM_Furnace] = 
    {
        .Archetype = ARCH_Furnace,
        .Flags = IS_VALID|IS_BUILDABLE,            
        .Sprite = SPRITE_Furnace,   
        .ItemID = ITEM_Furnace,      
        .MaxStackCount = 1,  
        .ItemName = STR("Furnace"), 
        .ItemDesc = STR("This is a Furnace, It can be used to smelt ores!"),
        .CraftingFormula = 
        {
            {ITEM_SapphireOreChunk, 3},
            {ITEM_Pebbles, 6},
        },
        .UniqueMaterialCount = 2,
        .FormulaResultCount  = 1,
        .Craftable = true,
    };
    
    GameState->GameData.ItemSprites[ITEM_Pebbles]          = MakePair(ITEM_Pebbles,          SPRITE_Pebbles);
    GameState->GameData.ItemSprites[ITEM_Branches]         = MakePair(ITEM_Branches,         SPRITE_Branches);
    GameState->GameData.ItemSprites[ITEM_Trunk]            = MakePair(ITEM_Trunk,            SPRITE_Trunk);
    GameState->GameData.ItemSprites[ITEM_SapphireOreChunk] = MakePair(ITEM_SapphireOreChunk, SPRITE_SapphireChunk);
    GameState->GameData.ItemSprites[ITEM_RubyOreChunk]     = MakePair(ITEM_RubyOreChunk,     SPRITE_RubyChunk);
    GameState->GameData.ItemSprites[ITEM_ToolPickaxe]      = MakePair(ITEM_ToolPickaxe,      SPRITE_ToolPickaxe);
    GameState->GameData.ItemSprites[ITEM_ToolWoodAxe]      = MakePair(ITEM_ToolWoodAxe,      SPRITE_ToolWoodAxe);
    GameState->GameData.ItemSprites[ITEM_Workbench]        = MakePair(ITEM_Workbench,        SPRITE_Workbench);
    GameState->GameData.ItemSprites[ITEM_Furnace]          = MakePair(ITEM_Furnace,          SPRITE_Furnace);
}

internal entity *
CreateEntity(game_state *GameState)
{
    entity *Result = {};
    
    for(uint32 EntityIndex = 1;
        EntityIndex < MAX_ENTITIES;
        ++EntityIndex)
    {
        entity *Found = &GameState->World.Entities[EntityIndex]; 
        if(!(Found->Flags & IS_VALID))
        {
            Result = Found;
            Result->EntityID = EntityIndex;
            break;
        }
    }
    Assert(Result);
    
    ++GameState->World.EntityCounter;
    Result->Flags = IS_VALID;
    return(Result);
}

internal inline void
DeleteEntity(entity *Entity, game_state *GameState)
{
    memset(Entity, 0, sizeof(struct entity));
}

internal void
HandleInput(game_state *GameState, entity *PlayerIn, time_data Time)
{
    vec2 InputAxis = {};
    if(IsGameKeyDown(MOVE_UP, &GameState->GameInput))
    {
        InputAxis.Y += 1.0f;
    }
    else if(IsGameKeyDown(MOVE_DOWN, &GameState->GameInput))
    {
        InputAxis.Y -= 1.0f;
    }

    if(IsGameKeyDown(MOVE_LEFT, &GameState->GameInput))
    {
        InputAxis.X -= 1.0f;
    }
    else if(IsGameKeyDown(MOVE_RIGHT, &GameState->GameInput))
    {
        InputAxis.X += 1.0f;
    }

    {
        InputAxis.X = (abs(GameState->GameInput.Controller.LeftStick.X) > GAMEPAD_LEFT_THUMB_DEADZONE) ? (GameState->GameInput.Controller.LeftStick.X / 32766.0f) : InputAxis.X;
        InputAxis.Y = (abs(GameState->GameInput.Controller.LeftStick.Y) > GAMEPAD_RIGHT_THUMB_DEADZONE) ? (GameState->GameInput.Controller.LeftStick.Y / 32767.0f) : InputAxis.Y;
        InputAxis.Y *= 1.0f;
    }

    // NOTE(Sleepster): Player Position 
    vec2 OldPlayerP = PlayerIn->Position;
    vec2 NextPos = {PlayerIn->Position.X + (PlayerIn->Position.X - OldPlayerP.X) + (PlayerIn->Speed * InputAxis.X) * (Time.Delta),
        PlayerIn->Position.Y + (PlayerIn->Position.Y - OldPlayerP.Y) + (PlayerIn->Speed * InputAxis.Y) * (Time.Delta)};
    PlayerIn->Position = v2Lerp(NextPos, Time.Delta, OldPlayerP);
    
    // NOTE(Sleepster): Game Update Stuff 
    
    if(IsKeyPressed(KEY_ESCAPE, &GameState->GameInput))
    {
        Player->Inventory.SelectedInventoryItem = {};
    }
    if(IsGameKeyPressed(INVENTORY, &GameState->GameInput))
    {
        GameState->DisplayPlayerInventory = !GameState->DisplayPlayerInventory;
    }
    if(IsGameKeyPressed(SHOW_HOTBAR, &GameState->GameInput))
    {
        GameState->DisplayPlayerHotbar = !GameState->DisplayPlayerHotbar;
    }
    if(IsGameKeyPressed(BUILD_MENU, &GameState->GameInput))
    {
        if(GameState->GameUIState == UI_State_Building)
        {
            GameState->GameUIState = UI_State_Nil;
            return;
        }
        GameState->GameUIState = UI_State_Building;
    }
    if(IsGameKeyPressed(HOTBAR_01, &GameState->GameInput))
    {
        if(Player->Inventory.CurrentInventorySlot == 0) 
        {
            Player->Inventory.CurrentInventorySlot = NULLSLOT;
            return;
        }
        Player->Inventory.CurrentInventorySlot = 0;
    }
    if(IsGameKeyPressed(HOTBAR_02, &GameState->GameInput))
    {
        if(Player->Inventory.CurrentInventorySlot == 1) 
        {
            Player->Inventory.CurrentInventorySlot = NULLSLOT;
            return;
        }
        Player->Inventory.CurrentInventorySlot = 1;
    }
    if(IsGameKeyPressed(HOTBAR_03, &GameState->GameInput))
    {
        if(Player->Inventory.CurrentInventorySlot == 2) 
        {
            Player->Inventory.CurrentInventorySlot = NULLSLOT;
            return;
        }
        Player->Inventory.CurrentInventorySlot = 2;
    }
    if(IsGameKeyPressed(HOTBAR_04, &GameState->GameInput))
    {
        if(Player->Inventory.CurrentInventorySlot == 3) 
        {
            Player->Inventory.CurrentInventorySlot = NULLSLOT;
            return;
        }
        Player->Inventory.CurrentInventorySlot = 3;
    }
    if(IsGameKeyPressed(HOTBAR_05, &GameState->GameInput))
    {
        if(Player->Inventory.CurrentInventorySlot == 4) 
        {
            Player->Inventory.CurrentInventorySlot = NULLSLOT;
            return;
        }
        Player->Inventory.CurrentInventorySlot = 4;
    }
    if(IsGameKeyPressed(HOTBAR_06, &GameState->GameInput))
    {
        if(Player->Inventory.CurrentInventorySlot == 5) 
        {
            Player->Inventory.CurrentInventorySlot = NULLSLOT;
            return;
        }
        Player->Inventory.CurrentInventorySlot = 5;
    }
    if(IsGameKeyPressed(HOTBAR_07, &GameState->GameInput))
    {
        if(Player->Inventory.CurrentInventorySlot == 6) 
        {
            Player->Inventory.CurrentInventorySlot = NULLSLOT;
            return;
        }
        Player->Inventory.CurrentInventorySlot = 6;
    }
    
    if(IsKeyPressed(KEY_HOME, &GameState->GameInput))
    {
        GameState->DrawDebug = !GameState->DrawDebug;
    }

    if(IsGamepadButtonPressed(DPAD_LEFT, &GameState->GameInput))
    {
        if(Player->Inventory.CurrentInventorySlot > 0) Player->Inventory.CurrentInventorySlot--;
    }
    if(IsGamepadButtonPressed(DPAD_RIGHT, &GameState->GameInput))
    {
        if(Player->Inventory.CurrentInventorySlot < PLAYER_HOTBAR_COUNT) Player->Inventory.CurrentInventorySlot++;
    }
}

internal void
SetupPlayer(game_state *GameState, entity *Entity)
{
    Entity->Archetype   = ARCH_Player;
    Entity->Sprite      = SPRITE_Player; 
    Entity->Flags      += IS_ACTIVE|IS_ACTOR;
    Entity->Size        = v2Cast(GameState->GameData.Sprites[SPRITE_Player].SpriteSize); 
    Entity->Health      = PlayerHealth;
    Entity->Position    = {};
    Entity->Rotation    = 0;
    Entity->Speed       = 100.0f;              // PIXELS PER SECOND
    Entity->BoxCollider = {};
    Entity->DroppedFromInventoryItemID  = ITEM_Nil;            // DROPS
}

internal void
SetupRock(game_state *GameState, entity *Entity)
{
    Entity->Archetype   = ARCH_Rock;
    Entity->Sprite      = SPRITE_Rock; 
    Entity->Flags      += IS_ACTIVE|IS_SOLID|IS_DESTRUCTABLE;
    Entity->Size        = v2Cast(GameState->GameData.Sprites[SPRITE_Rock].SpriteSize);
    Entity->Health      = RockHealth;
    Entity->Position    = {};
    Entity->Rotation    = 0;
    Entity->Speed       = 1.0f;
    Entity->BoxCollider = {};
    
    Entity->UniqueDropCount = 1;
    Entity->EntityDrops[0] = 
    {
        entity_item_drop{ITEM_Pebbles, 1},
    };
}

internal void
SetupTree00(game_state *GameState, entity *Entity)
{
    Entity->Archetype   = ARCH_Tree00;
    Entity->Sprite      = SPRITE_Tree00; 
    Entity->Flags      += IS_ACTIVE|IS_SOLID|IS_DESTRUCTABLE;
    Entity->Size        = v2Cast(GameState->GameData.Sprites[SPRITE_Tree00].SpriteSize);
    Entity->Health      = TreeHealth;
    Entity->Position    = {};
    Entity->Rotation    = 0;
    Entity->Speed       = 1.0f;
    Entity->BoxCollider = {};
    
    Entity->UniqueDropCount = 1;
    Entity->EntityDrops[0] = 
    {
        entity_item_drop{ITEM_Branches, 1},
    };
}

internal void
SetupTree01(game_state *GameState, entity *Entity)
{
    Entity->Archetype   = ARCH_Tree01;
    Entity->Sprite      = SPRITE_Tree01; 
    Entity->Flags      += IS_ACTIVE|IS_SOLID|IS_DESTRUCTABLE;
    Entity->Size        = v2Cast(GameState->GameData.Sprites[SPRITE_Tree01].SpriteSize);
    Entity->Health      = TreeHealth;
    Entity->Position    = {};
    Entity->Rotation    = 0;
    Entity->Speed       = 1.0f;
    Entity->BoxCollider = {};
    
    Entity->UniqueDropCount = 1;
    Entity->EntityDrops[0] = 
    {
        entity_item_drop{ITEM_Trunk, 1},
    };
}

internal void
SetupRubyNode(game_state *GameState, entity *Entity)
{
    Entity->Archetype   = ARCH_RubyNode;
    Entity->Sprite      = SPRITE_RubyOre; 
    Entity->Flags      += IS_ACTIVE|IS_SOLID|IS_DESTRUCTABLE;
    Entity->Size        = v2Cast(GameState->GameData.Sprites[SPRITE_RubyOre].SpriteSize);
    Entity->Health      = NodeHealth;
    Entity->Position    = {};
    Entity->Rotation    = 0;
    Entity->Speed       = 1.0f;
    Entity->BoxCollider = {};
    
    Entity->UniqueDropCount = 1;
    Entity->EntityDrops[0] = 
    {
        entity_item_drop{ITEM_RubyOreChunk, 1},
    };
}

internal void
SetupSapphireNode(game_state *GameState, entity *Entity)
{
    Entity->Archetype   = ARCH_SapphireNode;
    Entity->Sprite      = SPRITE_SapphireOre; 
    Entity->Flags      += IS_ACTIVE|IS_SOLID|IS_DESTRUCTABLE;
    Entity->Size        = v2Cast(GameState->GameData.Sprites[SPRITE_SapphireOre].SpriteSize);
    Entity->Health      = NodeHealth;
    Entity->Position    = {};
    Entity->Rotation    = 0;
    Entity->Speed       = 1.0f;
    Entity->BoxCollider = {};
    
    Entity->UniqueDropCount = 1;
    Entity->EntityDrops[0] = 
    {
        entity_item_drop{ITEM_SapphireOreChunk, 1},
    };
}

internal void
SetupItemPebbles(game_state *GameState, entity *Entity)
{
    Entity->Archetype = ARCH_Pebbles;
    Entity->Sprite    = SPRITE_Pebbles;
    Entity->Flags    += IS_ACTIVE|IS_ITEM|CAN_BE_PICKED_UP;
    Entity->Size        = v2Cast(GameState->GameData.Sprites[SPRITE_Pebbles].SpriteSize) * 0.8f;
    Entity->DroppedFromInventoryItemID    = ITEM_Pebbles;
}

internal void
SetupItemBranches(game_state *GameState, entity *Entity)
{
    Entity->Archetype = ARCH_Branches;
    Entity->Sprite    = SPRITE_Branches;
    Entity->Flags    += IS_ACTIVE|IS_ITEM|CAN_BE_PICKED_UP;
    Entity->Size        = v2Cast(GameState->GameData.Sprites[SPRITE_Branches].SpriteSize) * 0.8f;
    Entity->DroppedFromInventoryItemID    = ITEM_Branches;
}

internal void
SetupItemTrunk(game_state *GameState, entity *Entity)
{
    Entity->Archetype = ARCH_Trunk;
    Entity->Sprite    = SPRITE_Trunk;
    Entity->Flags    += IS_ACTIVE|IS_ITEM|CAN_BE_PICKED_UP;
    Entity->Size        = v2Cast(GameState->GameData.Sprites[SPRITE_Trunk].SpriteSize) * 0.8f;
    Entity->DroppedFromInventoryItemID    = ITEM_Trunk;
}

internal void
SetupItemRubyChunk(game_state *GameState, entity *Entity)
{
    Entity->Archetype = ARCH_RubyOreChunk;
    Entity->Sprite    = SPRITE_RubyChunk;
    Entity->Flags    += IS_ACTIVE|IS_ITEM|CAN_BE_PICKED_UP;
    Entity->Size        = v2Cast(GameState->GameData.Sprites[SPRITE_RubyChunk].SpriteSize) * 0.8f;
    Entity->DroppedFromInventoryItemID    = ITEM_RubyOreChunk;
}

internal void
SetupItemSapphireChunk(game_state *GameState, entity *Entity)
{
    Entity->Archetype = ARCH_SapphireOreChunk;
    Entity->Sprite    = SPRITE_SapphireChunk;
    Entity->Flags    += IS_ACTIVE|IS_ITEM|CAN_BE_PICKED_UP;
    Entity->Size        = v2Cast(GameState->GameData.Sprites[SPRITE_SapphireChunk].SpriteSize) * 0.8f;
    Entity->DroppedFromInventoryItemID    = ITEM_SapphireOreChunk;
}

internal void
SetupItemToolPickaxe(game_state *GameState, entity *Entity)
{
    Entity->Archetype = ARCH_SimplePickaxe;
    Entity->Sprite    = SPRITE_ToolPickaxe;
    Entity->Flags    += IS_ACTIVE|IS_ITEM|CAN_BE_PICKED_UP;
    Entity->Size        = v2Cast(GameState->GameData.Sprites[SPRITE_ToolPickaxe].SpriteSize);
    Entity->DroppedFromInventoryItemID    = ITEM_ToolPickaxe;
}

internal void
SetupItemToolWoodAxe(game_state *GameState, entity *Entity)
{
    Entity->Archetype = ARCH_SimpleWoodAxe;
    Entity->Sprite    = SPRITE_ToolWoodAxe;
    Entity->Flags    += IS_ACTIVE|IS_ITEM|CAN_BE_PICKED_UP;
    Entity->Size        = v2Cast(GameState->GameData.Sprites[SPRITE_ToolWoodAxe].SpriteSize);
    Entity->DroppedFromInventoryItemID    = ITEM_ToolWoodAxe;
}

internal void
SetupItemWorkbench(game_state *GameState, entity *Entity)
{
    Entity->Archetype = ARCH_Workbench;
    Entity->Sprite    = SPRITE_Workbench;
    Entity->Flags    += IS_ACTIVE|IS_ITEM|CAN_BE_PICKED_UP|IS_BUILDABLE;
    Entity->Size        = v2Cast(GameState->GameData.Sprites[SPRITE_Workbench].SpriteSize) * 0.5f;
    Entity->DroppedFromInventoryItemID    = ITEM_Workbench;
}

internal void
SetupItemFurnace(game_state *GameState, entity *Entity)
{
    Entity->Archetype = ARCH_Furnace,
    Entity->Sprite    = SPRITE_Furnace;
    Entity->Flags    += IS_ACTIVE|IS_ITEM|CAN_BE_PICKED_UP|IS_BUILDABLE;
    Entity->Size        = v2Cast(GameState->GameData.Sprites[SPRITE_Furnace].SpriteSize) * 0.5f;
    Entity->DroppedFromInventoryItemID    = ITEM_Furnace;
}

internal void
SetupBuildingWorkbench(game_state *GameState, entity *Entity)
{
    Entity->Archetype = ARCH_Workbench;
    Entity->Sprite    = SPRITE_Workbench;
    Entity->Flags    += IS_ACTIVE|IS_BUILDABLE|IS_PLACED|IS_DESTRUCTABLE;
    Entity->Size        = v2Cast(GameState->GameData.Sprites[SPRITE_Workbench].SpriteSize);
    Entity->Health      = NodeHealth;
    Entity->Rotation    = 0;
    Entity->Speed       = 1.0f;
    Entity->BoxCollider = {};
    
    Entity->UniqueDropCount = 1;
    Entity->EntityDrops[0] = 
    {
        entity_item_drop{ITEM_Workbench, 1},
    };
}

internal void
SetupBuildingFurnace(game_state *GameState, entity *Entity)
{
    Entity->Archetype = ARCH_Workbench;
    Entity->Sprite    = SPRITE_Furnace;
    Entity->Flags    += IS_ACTIVE|IS_BUILDABLE|IS_PLACED|IS_DESTRUCTABLE;
    Entity->Size        = v2Cast(GameState->GameData.Sprites[SPRITE_Furnace].SpriteSize);
    
    Entity->Health      = NodeHealth;
    Entity->Rotation    = 0;
    Entity->Speed       = 1.0f;
    Entity->BoxCollider = {};
    
    Entity->UniqueDropCount = 1;
    Entity->EntityDrops[0] = 
    {
        entity_item_drop{ITEM_Furnace, 1},
    };
}

internal inline void
ResetGame(gl_render_data *RenderData, game_state *GameState, game_memory *GameMemory)
{
    for(uint32 i = 0; i < MAX_ENTITIES; i++)
    {
        entity *Temp = &GameState->World.Entities[i];
        DeleteEntity(Temp, GameState);
    }
    GameState->World.EntityCounter = 0;
    
    for(uint32 i = 0; i < SPRITE_Count; i++)
    {
        GameState->GameData.Sprites[i] = {};
    }
    
    GameState->DisplayPlayerHotbar = true;
    GameState->DisplayPlayerInventory = false;
    GameState->DisplayCraftingMenu = false;
    GameState->ActiveCraftingStation = {};
    GameState->ActiveRecipe = {};
}

internal int32
WorldToTilei32(real32 WorldPosition)
{
    return(int32(floorf(WorldPosition / (real32)TILE_SIZE)));
}

internal real32
TileToWorldr32(int32 WorldPosition)
{
    return((real32)WorldPosition * (real32)TILE_SIZE);
}

internal ivec2 
WorldToTilePos(vec2 WorldPosition)
{
    ivec2 Result = {};
    
    Result.X = WorldToTilei32(WorldPosition.X);
    Result.Y = WorldToTilei32(WorldPosition.Y);
    
    return(Result);
}

internal vec2
TileToWorldPos(ivec2 TilePosition)
{
    vec2 Result = {};
    
    Result.X = TileToWorldr32(TilePosition.X);
    Result.Y = TileToWorldr32(TilePosition.Y);
    
    return(Result);
}

internal vec2
RoundToTile(vec2 WorldPosition)
{
    vec2 NewWorldPosition = TileToWorldPos(WorldToTilePos(WorldPosition));
    return(NewWorldPosition);
}

inline int
CompareEntityYAxis(const void *A, const void *B)
{
    const entity *EntityA = (const entity*)A;
    const entity *EntityB = (const entity*)B;
    
    return((EntityA->Position.Y > EntityB->Position.Y) ?  1 :
           (EntityA->Position.Y < EntityB->Position.Y) ? -1 : 0);
}

internal bool32
SwapInventoryItems(entity_item_inventory *Inventory, item *ItemA, item *ItemB)
{   
    item TempItem = *ItemA;
    Inventory->Items[ItemA->OccupiedInventorySlot] = *ItemB;
    Inventory->Items[ItemB->OccupiedInventorySlot] = TempItem;
    
    return(true);
}

internal void
SetupDroppedEntity(gl_render_data *RenderData, game_state *GameState, item *SelectionItem, entity *SpawnedItem)
{
    switch(SelectionItem->Archetype)
    {
        case ARCH_Pebbles:
        {
            SetupItemPebbles(GameState, SpawnedItem);
        }break;
        case ARCH_Trunk:
        {
            SetupItemTrunk(GameState, SpawnedItem);
        }break;
        case ARCH_Branches:
        {
            SetupItemBranches(GameState, SpawnedItem);
        }break;
        case ARCH_RubyOreChunk:
        {
            SetupItemRubyChunk(GameState, SpawnedItem);
        }break;
        case ARCH_SapphireOreChunk:
        {
            SetupItemSapphireChunk(GameState, SpawnedItem);
        }break;
        case ARCH_SimplePickaxe:
        {
            SetupItemToolPickaxe(GameState, SpawnedItem);
        }break;
        case ARCH_SimpleWoodAxe:
        {
            SetupItemToolWoodAxe(GameState, SpawnedItem);
        }break;
        case ARCH_Workbench:
        {
            SetupItemWorkbench(GameState, SpawnedItem);
        }break;
        case ARCH_Furnace:
        {
            SetupItemFurnace(GameState, SpawnedItem);
        }break;
    }
    
    SpawnedItem->Flags -= CAN_BE_PICKED_UP;
    SpawnedItem->DroppedFromInventoryItemCount = SelectionItem->CurrentStack;
    
    vec2 WorldMouseCoords = TransformMouseCoords(RenderData->GameCamera.ViewMatrix, RenderData->GameCamera.ProjectionMatrix, GameState->GameInput.Keyboard.CurrentMouse, SizeData);
    real32 Distance = fabsf(v2Distance(Player->Position, WorldMouseCoords));
    vec2 Direction = v2Normalize(WorldMouseCoords - Player->Position);
    
    SpawnedItem->Position = Player->Position;
    if(Distance <= MaxDropDistance)
    {
        vec2 DropPosition = Player->Position + Direction * Distance;
        SpawnedItem->Target = DropPosition;
    }
    else
    {
        vec2 MaxDropPosition = Player->Position + Direction * MaxDropDistance;
        SpawnedItem->Target = MaxDropPosition;
    }
}

internal void
ResetItemSlotState(item *Item)
{
    Item->Sprite = {};
    Item->CurrentStack = 0;
    Item->ItemName = {};
    Item->ItemDesc = {};
    Item->ItemID   = {};
    Item->MaxStackCount = {};
    Item->Flags = {};
}

internal inline sprite_type
GetSpriteFromPair(game_state *GameState, item_id ID)
{
    for(const auto &ItemData : GameState->GameData.ItemSprites)
    {
        if(ItemData.First == ID)
        {
            return(ItemData.Second);
        }
    }
    return(SPRITE_Nil);
}

internal inline item_id
GetItemIDFromPair(game_state *GameState, sprite_type Sprite)
{
    for(const auto &ItemData : GameState->GameData.ItemSprites)
    {
        if(ItemData.Second == Sprite)
        {
            return(ItemData.First);
        }
    }   
    return(ITEM_Nil);
}

internal bool32
IsItemCraftable(int *ItemCounts, item *Craft)
{
    if(Craft->CraftingFormula)
    {
        for(int32 FormulaIndex = 0;
            FormulaIndex < Craft->UniqueMaterialCount;
            FormulaIndex++)
        {
            int ItemCount = ItemCounts[FormulaIndex];
            if(ItemCount >= Craft->CraftingFormula[FormulaIndex].RequiredCount)
            {
                continue;
            }
            else
            {
                return(false);
            }
        }
        return(true);
    }
    return(false);
}

internal void
AddItemToPlayerInventory(game_state *GameState, entity *PlayerEntity, entity *Temp)
{
    if((Temp->Flags & IS_ITEM) && (Temp->Flags & CAN_BE_PICKED_UP))
    {
        real32 ItemDistance = fabsf(v2Distance(Temp->Position, PlayerEntity->Position));
        if(ItemDistance <= ItemPickupDist)
        {
            for(uint32 InventoryIndex = 0;
                InventoryIndex < TOTAL_INVENTORY_SIZE;
                ++InventoryIndex)
            {
                item NewItem = GameState->GameData.GameItems[Temp->DroppedFromInventoryItemID];
                if(PlayerEntity->Inventory.Items[InventoryIndex].ItemID == NewItem.ItemID && 
                   PlayerEntity->Inventory.Items[InventoryIndex].CurrentStack < 
                   PlayerEntity->Inventory.Items[InventoryIndex].MaxStackCount)
                {
                    PlayerEntity->Inventory.Items[InventoryIndex].CurrentStack++;
                    // NOTE(Sleepster): If two matching IDs are found, skip to the deletion 
                    DeleteEntity(Temp, GameState);
                    return;
                }
            }
            
            for(uint32 InventoryIndex = 0;
                InventoryIndex < TOTAL_INVENTORY_SIZE;
                ++InventoryIndex)
            {
                item NewItem = GameState->GameData.GameItems[Temp->DroppedFromInventoryItemID];
                if(PlayerEntity->Inventory.Items[InventoryIndex].ItemID == 0)
                {
                    NewItem.CurrentStack = Temp->DroppedFromInventoryItemCount;
                    if(NewItem.CurrentStack == 0)
                    {
                        NewItem.CurrentStack = 1;
                    }
                    PlayerEntity->Inventory.Items[InventoryIndex] = NewItem;
                    PlayerEntity->Inventory.Items[InventoryIndex].OccupiedInventorySlot = InventoryIndex;
                    DeleteEntity(Temp, GameState);
                    break;
                }
            }
        }
    }
}

extern
GAME_ON_AWAKE(GameOnAwake)
{
    ResetGame(RenderData, GameState, GameMemory);
    LoadSpriteData(GameState);
    LoadItemData(GameState);
    
    real32 SizeScaler = WORLD_SIZE * 10;
    for(uint32 EntityIndex = 0;
        EntityIndex < 50;
        ++EntityIndex)
    {
        entity *En = CreateEntity(GameState);
        SetupRock(GameState, En);
        En->Position = vec2{GetRandomReal32_Range(-SizeScaler, SizeScaler), GetRandomReal32_Range(-SizeScaler, SizeScaler)};
        En->Position = TileToWorldPos(WorldToTilePos(En->Position));
        
        
        entity *En2 = CreateEntity(GameState);
        SetupTree00(GameState, En2);
        En2->Position = vec2{GetRandomReal32_Range(-SizeScaler, SizeScaler), GetRandomReal32_Range(-SizeScaler, SizeScaler)};
        En2->Position = TileToWorldPos(WorldToTilePos(En2->Position));
        
        
        entity *En3 = CreateEntity(GameState);
        SetupTree01(GameState, En3);
        En3->Position = vec2{GetRandomReal32_Range(-SizeScaler, SizeScaler), GetRandomReal32_Range(-SizeScaler, SizeScaler)};
        En3->Position = TileToWorldPos(WorldToTilePos(En3->Position));
        
        
        entity *En4 = CreateEntity(GameState);
        SetupRubyNode(GameState, En4);
        En4->Position = vec2{GetRandomReal32_Range(-SizeScaler, SizeScaler), GetRandomReal32_Range(-SizeScaler, SizeScaler)};
        En4->Position = TileToWorldPos(WorldToTilePos(En4->Position));
        
        
        entity *En5 = CreateEntity(GameState);
        SetupSapphireNode(GameState, En5);
        En5->Position = vec2{GetRandomReal32_Range(-SizeScaler, SizeScaler), GetRandomReal32_Range(-SizeScaler, SizeScaler)};
        En5->Position = TileToWorldPos(WorldToTilePos(En5->Position));
    }
    
    entity *WorkbenchTest = CreateEntity(GameState);
    SetupBuildingWorkbench(GameState, WorkbenchTest);
    WorkbenchTest->Position = {0, -80};
    WorkbenchTest->Position = TileToWorldPos(WorldToTilePos(WorkbenchTest->Position));
    WorkbenchTest->BoxCollider = CreateRange(vec2{WorkbenchTest->Position.X - (TILE_SIZE * 0.5f), WorkbenchTest->Position.Y}, 
                                             vec2{WorkbenchTest->Position.X - (TILE_SIZE * 0.5f), WorkbenchTest->Position.Y} + WorkbenchTest->Size);
    
    entity *FurnaceTest = CreateEntity(GameState);
    SetupBuildingFurnace(GameState, FurnaceTest);
    FurnaceTest->Position = {20, -80};
    FurnaceTest->Position = TileToWorldPos(WorldToTilePos(FurnaceTest->Position));
    FurnaceTest->BoxCollider = CreateRange(vec2{FurnaceTest->Position.X - (TILE_SIZE * 0.5f), FurnaceTest->Position.Y}, 
                                           vec2{FurnaceTest->Position.X - (TILE_SIZE * 0.5f), FurnaceTest->Position.Y} + FurnaceTest->Size);
    
    entity *GroundWorkbench = CreateEntity(GameState);
    SetupItemWorkbench(GameState, GroundWorkbench);
    GroundWorkbench->Position = {0, -100};
    GroundWorkbench->Target   = {0, -100};
    GroundWorkbench->BoxCollider = CreateRange(vec2{GroundWorkbench->Position.X - (TILE_SIZE * 0.5f), GroundWorkbench->Position.Y}, 
                                               vec2{GroundWorkbench->Position.X - (TILE_SIZE * 0.5f), GroundWorkbench->Position.Y} + GroundWorkbench->Size);
    
    entity *GroundFurnace = CreateEntity(GameState);
    SetupItemFurnace(GameState, GroundFurnace);
    GroundFurnace->Position = {20, -100};
    GroundFurnace->Target   = {20, -100};
    
    entity *Pickaxe = CreateEntity(GameState);
    SetupItemToolPickaxe(GameState, Pickaxe);
    Pickaxe->Position = {0, 150};
    Pickaxe->Target  = {0, 150};
    
    
    entity *Pickaxe2 = CreateEntity(GameState);
    SetupItemToolPickaxe(GameState, Pickaxe2);
    Pickaxe2->Position = {32, 150};
    Pickaxe2->Target = {32, 150};
    
    Player = CreateEntity(GameState);
    SetupPlayer(GameState, Player);
    
    GameState->DisplayPlayerHotbar = true;
}

extern
GAME_FIXED_UPDATE(GameFixedUpdate)
{
}

extern
GAME_UPDATE_AND_DRAW(GameUpdateAndDraw)
{
    TransientState->SelectedEntityThisFrame = {};
    // MATRICES
    {
        // NOTE(Sleepster): GAME 
        RenderData->GameCamera.Zoom = 5.3f;
        
        mat4 ScaleMatrix                              = mat4MakeScale(vec3{1.0f * RenderData->GameCamera.Zoom, 1.0f * RenderData->GameCamera.Zoom, 1.0f});
        mat4 TranslationMatrix                        = mat4Translate(vec3{-RenderData->GameCamera.Position.X, -RenderData->GameCamera.Position.Y, 0.0f});
        
        RenderData->GameCamera.ViewMatrix             = mat4Identity(1.0f);
        RenderData->GameCamera.ViewMatrix             = mat4Multiply(TranslationMatrix, RenderData->GameCamera.ViewMatrix);
        RenderData->GameCamera.ViewMatrix             = mat4Multiply(ScaleMatrix, RenderData->GameCamera.ViewMatrix);
        RenderData->GameCamera.ProjectionMatrix       = mat4RHGLOrtho((real32)SizeData.Width * -0.5f, (real32)SizeData.Width * 0.5f, (real32)SizeData.Height * -0.5f, (real32)SizeData.Height * 0.5f, -1.0f, 1.0f); 
        RenderData->GameCamera.ProjectionViewMatrix   = mat4Multiply(RenderData->GameCamera.ProjectionMatrix, RenderData->GameCamera.ViewMatrix);        
        
        // NOTE(Sleepster): UI
        RenderData->GameUICamera.ProjectionMatrix     = mat4RHGLOrtho((real32)SizeData.Width * -0.5f, (real32)SizeData.Width * 0.5f, (real32)SizeData.Height * -0.5f, (real32)SizeData.Height * 0.5f, -1.0f, 1.0f); 
        RenderData->GameUICamera.ViewMatrix           = mat4Multiply(mat4Identity(1.0f), ScaleMatrix);
        RenderData->GameUICamera.ProjectionViewMatrix = mat4Multiply(RenderData->GameUICamera.ProjectionMatrix, RenderData->GameUICamera.ViewMatrix);
        
        GameState->UIContext.UICameraViewMatrix       = RenderData->GameUICamera.ViewMatrix;
        GameState->UIContext.UICameraProjectionMatrix = RenderData->GameUICamera.ProjectionMatrix;
        GameState->UIContext.GameInput                = &GameState->GameInput;
        GameState->UIContext.ActiveFont               = TransientState->GameAssets->Fonts[GF_UbuntuMono].Font;
        GameState->UIContext.ActiveFontIndex          = GF_UbuntuMono;
    }
    
    vec2 MouseToWorld  = TransformMouseCoords(RenderData->GameCamera.ViewMatrix, 
                                              RenderData->GameCamera.ProjectionMatrix, 
                                              GameState->GameInput.Keyboard.CurrentMouse, 
                                              SizeData);
    
    // vec2 MouseToScreen = TransformMouseCoords(RenderData->GameUICamera.ViewMatrix,
    //                                           RenderData->GameUICamera.ProjectionMatrix, 
    //                                           GameState->GameInput.Keyboard.CurrentMouse, 
    //                                           SizeData);
    
    // NOTE(Sleepster): SELECTED ENTITY
    real32 SelectionDistance = 32.0f;
    real32 MinimumDistance = 0;
    for(uint32 EntityIndex = 0;
        EntityIndex <= GameState->World.EntityCounter;
        ++EntityIndex)
    {
        entity *Temp = &GameState->World.Entities[EntityIndex];
        if((Temp->Flags & IS_VALID))
        {
            real32 Distance = fabsf(v2Distance(Temp->Position, MouseToWorld));
            real32 PlayerToObjectDistance = fabsf(v2Distance(Temp->Position, Player->Position));
            if(Distance <= SelectionDistance && PlayerToObjectDistance <= MaxHitRange)
            {
                if(!TransientState->SelectedEntityThisFrame || (Distance < MinimumDistance) || (Temp->EntityID != TransientState->SelectedEntityThisFrame->EntityID))
                {
                    TransientState->SelectedEntityThisFrame = Temp;
                    MinimumDistance = Distance;
                }
                
                // NOTE(Sleepster): Handle Entity Destruction 
                if(IsGameKeyPressed(ATTACK, &GameState->GameInput) && 
                   (Temp->Flags & IS_DESTRUCTABLE) && 
                   !(Temp->Flags & IS_UI) && 
                   PlayerToObjectDistance <= MaxHitRange &&
                   GameState->GameUIState == UI_State_Nil)
                {
                    --Temp->Health;
                    if(Temp->Health <= 0)
                    {
                        for(int32 DropIndex = 0;
                            DropIndex < Temp->UniqueDropCount;
                            DropIndex++)
                        {
                            for(int32 DropCount = 0;
                                DropCount < Temp->EntityDrops[DropIndex].DropAmount;
                                DropCount++)
                            {
                                entity *CreatedEntity = CreateEntity(GameState);
                                item DroppedItem = GameState->GameData.GameItems[Temp->EntityDrops[DropCount].DroppedItem];
                                
                                SetupDroppedEntity(RenderData, GameState, &DroppedItem, CreatedEntity);
                                CreatedEntity->Position = Temp->Position;
                                CreatedEntity->Target   = Temp->Position;
                            }
                        }
                        
                        TransientState->SelectedEntityThisFrame = {};
                        DeleteEntity(Temp, GameState);
                    }
                }
            }
            // NOTE(Sleepster): Add Item to Inventory
            AddItemToPlayerInventory(GameState, Player, Temp);
        }
    }
    
    // NOTE(Sleepster): New Hotbar UI 
    if(GameState->DisplayPlayerHotbar)
    {
        const real32 Width = SizeData.X * 0.25f;
        const real32 Padding = 4.0f;
        const real32 IconSize = 16.0f;
        const real32 TotalWidth = (PLAYER_HOTBAR_COUNT * IconSize) + ((PLAYER_HOTBAR_COUNT - 1) * Padding);
        const real32 StartingX = (Width / 2.0f) - (TotalWidth / 2.0f);
        
        real32 YOffset = -90.0f;
        for(uint32 InventorySlot = 0;
            InventorySlot < PLAYER_HOTBAR_COUNT;
            ++InventorySlot)
        {
            real32 SlotOffset = (IconSize + Padding) * InventorySlot;
            
            mat4 XForm = mat4Identity(1.0f);
            XForm = mat4Multiply(XForm, mat4Translate(vec3{StartingX + SlotOffset, YOffset, 0.0}));
            XForm = mat4Multiply(XForm, mat4Translate(vec3{IconSize * -0.55f, 0.0f, 0.0f}));
            XForm = mat4Multiply(XForm, mat4MakeScale(vec3{IconSize, IconSize, 1.0}));
            
            vec4 MatrixPosition = XForm.Columns[3];
            vec2 SlotPosition   = {MatrixPosition.X, MatrixPosition.Y};
            
            static_sprite_data Sprite = GetSprite(GameState, SPRITE_UIItemBox);
            ui_element_state HotbarSlotState = CloverUIButton(&GameState->UIContext, STR("HotbarSlot"), SlotPosition, {IconSize, IconSize}, Sprite, WHITE);
            ui_element *HotbarSlot = &GameState->UIContext.UIElements[HotbarSlotState.UIID.ID];
            
            Player->Inventory.InventorySlotButtons[InventorySlot] = HotbarSlot;
            
            HotbarSlot->XForm = XForm;
            HotbarSlot->Sprite = Sprite;
            HotbarSlot->DrawColor = WHITE;
            
            item *Item = &Player->Inventory.Items[InventorySlot];
            Item->OccupiedInventorySlot = InventorySlot;
            
            Sprite = GetSprite(GameState, Item->Sprite);
            if(Sprite != GameState->GameData.Sprites[SPRITE_Nil])
            {
                if(!HotbarSlotState.IsHot)
                {
                    XForm = mat4Multiply(XForm, mat4MakeScale(vec3{0.65, 0.65, 1.0}));
                }
                
                if(InventorySlot == Player->Inventory.CurrentInventorySlot)
                {
                    XForm = mat4Multiply(XForm, mat4MakeScale(vec3{1.2, 1.2, 1.0}));
                }
                DrawUISpriteXForm(TransientState, XForm, Sprite, 0, WHITE);
            }
            
            if(HotbarSlotState.IsHot)
            {
                HotbarSlot->DrawColor = RED;
                if(Item->Sprite != SPRITE_Nil)
                {
                    XForm = mat4Multiply(XForm, mat4MakeScale(vec3{0.8, 0.8, 1.0}));
                    
                    real32 NewUIYOffset;
                    real32 NewUIYDescOffset;
                    real32 SpriteYOffset;
                    if(GameState->DisplayPlayerInventory)
                    {
                        NewUIYOffset = 24.0f;
                        NewUIYDescOffset = 8.0f;
                        SpriteYOffset = 25.0f;
                    }
                    else
                    {
                        NewUIYOffset = 16.0f;
                        NewUIYDescOffset = 3.0f;
                        SpriteYOffset = 20.0f;
                    }
                    
                    vec2 UIBoxSize = {10, 25};
                    
                    mat4 UIXForm = mat4Identity(1.0f);
                    UIXForm = mat4Multiply(UIXForm, mat4Translate(vec3{StartingX + SlotOffset, YOffset, 0.0}));
                    UIXForm = mat4Multiply(UIXForm, mat4Translate(vec3{IconSize * -0.55f, NewUIYOffset, 0.0f}));
                    
                    // NOTE(Sleepster): String Size to influence the BosSize
                    vec4 UIMatrixPosition = UIXForm.Columns[3];
                    vec2 Position = vec2{UIMatrixPosition.X, UIMatrixPosition.Y};
                    
                    static_sprite_data SpriteData = GetSprite(GameState, Item->Sprite);
                    
                    ui_element *ItemDescData = CloverUIMakeTextElement(&GameState->UIContext, Item->ItemDesc, {Position.X, Position.Y + NewUIYDescOffset}, 10, TEXT_ALIGNMENT_Center, GREEN);
                    CloverUIMakeTextElement(&GameState->UIContext, Item->ItemName, {Position.X + 4, Position.Y + NewUIYOffset}, 10, TEXT_ALIGNMENT_Center, GREEN);
                    
                    mat4 SpriteXForm = UIXForm;
                    UIXForm = mat4Multiply(UIXForm, mat4Translate(vec3{0, NewUIYOffset, 0}));
                    UIXForm = mat4Multiply(UIXForm, mat4MakeScale(v2Expand(UIBoxSize + ItemDescData->Size, 1.0f)));
                    DrawUISpriteXForm(TransientState, UIXForm, GetSprite(GameState, SPRITE_Nil), 0, vec4{0.2f, 0.2f, 0.2f, 0.2f});
                    
                    SpriteXForm = mat4Multiply(SpriteXForm, mat4Translate(vec3{UIBoxSize.X + ItemDescData->Size.X * -0.5f, SpriteYOffset, 0}));
                    SpriteXForm = mat4Multiply(SpriteXForm, mat4MakeScale(vec3{IconSize, IconSize, 1.0f}));
                    DrawUISpriteXForm(TransientState, SpriteXForm, SpriteData, 0, WHITE);
                    
                    SpriteXForm = mat4Multiply(SpriteXForm, mat4MakeScale(vec3{1 / IconSize, 1 / IconSize, 1.0f}));
                    SpriteXForm = mat4Multiply(SpriteXForm, mat4Translate(vec3{-(UIBoxSize.X + ItemDescData->Size.X * -0.5f), 0, 0}));
                    SpriteXForm = mat4Multiply(SpriteXForm, mat4Translate(vec3{(UIBoxSize.X + ItemDescData->Size.X * -0.5f), 19, 0}));
                    SpriteXForm = mat4Multiply(SpriteXForm, mat4Translate(vec3{-7, 0, 0}));
                    SpriteXForm = mat4Multiply(SpriteXForm, mat4MakeScale(vec3{16, 16, 1.0f}));
                    DrawUISpriteXForm(TransientState, SpriteXForm, GetSprite(GameState, SPRITE_Nil), 0, vec4{0.1f, 0.1f, 0.1f, 0.4f});
                    
                    SpriteXForm = mat4Multiply(SpriteXForm, mat4MakeScale(vec3{1 / IconSize, 1 / IconSize, 1.0f}));
                    SpriteXForm = mat4Multiply(SpriteXForm, mat4Translate(vec3{2, -6, 0}));
                    SpriteXForm = mat4Multiply(SpriteXForm, mat4MakeScale(vec3{16, 16, 1.0f}));
                    
                    vec4 ItemCountTextPosition = SpriteXForm.Columns[3];
                    Position = vec2{ItemCountTextPosition.X, ItemCountTextPosition.Y};

                    CloverUIMakeTextElement(&GameState->UIContext, sprints(&TransientState->Garbage, STR("x%d"), Item->CurrentStack), {Position.X + 3, Position.Y}, 15, TEXT_ALIGNMENT_Center, GREEN);
                }
            }
            
            // SELECTED INVENTORY ITEM
            if(HotbarSlot->IsActive)
            {
                if(Item->Sprite != SPRITE_Nil && !Player->Inventory.SelectedInventoryItem)
                {
                    Player->Inventory.SelectedInventoryItem = Item;
                }
            }
            
            if(Player->Inventory.CurrentInventorySlot == InventorySlot)
            {
                Player->Inventory.SelectedHotbarItem = &Player->Inventory.Items[InventorySlot];
            }
        }
    }
    
    // NOTE(Sleepster): Large Inventory UI 
    const real32 InventoryYOffset = -70.0f;
    if(GameState->DisplayPlayerInventory)
    {
        const real32 Width = SizeData.X * 0.25f;
        const real32 Padding = 1.00f;
        const real32 IconSize = 16.0f;
        const real32 TotalWidth = ((PLAYER_INVENTORY_SIZE * IconSize) + (IconSize * 2) - 7) + ((PLAYER_INVENTORY_SIZE - 1) * Padding);
        const real32 StartingX = (Width / 2.0f) - (TotalWidth / 2.0f);
        
        for(uint32 InventorySlot = PLAYER_HOTBAR_COUNT;
            InventorySlot < TOTAL_INVENTORY_SIZE;
            ++InventorySlot)
        {
            real32 SlotOffset = (IconSize + Padding) * (InventorySlot - 6);
            
            mat4 XForm = mat4Identity(1.0f);
            XForm = mat4Multiply(XForm, mat4Translate(vec3{StartingX + SlotOffset, InventoryYOffset, 0.0}));
            XForm = mat4Multiply(XForm, mat4Translate(vec3{IconSize * -0.55f, 0.0f, 0.0f}));
            XForm = mat4Multiply(XForm, mat4MakeScale(vec3{IconSize, IconSize, 1.0}));
            
            vec4 MatrixPosition = XForm.Columns[3];
            vec2 SlotPosition   = {MatrixPosition.X, MatrixPosition.Y};
            
            static_sprite_data Sprite = GetSprite(GameState, SPRITE_UIItemBox);
            ui_element_state HotbarSlotState = CloverUIButton(&GameState->UIContext, STR("InventorySlot"), SlotPosition, {IconSize, IconSize}, Sprite, WHITE);
            ui_element *HotbarSlot = &GameState->UIContext.UIElements[HotbarSlotState.UIID.ID];
            
            Player->Inventory.InventorySlotButtons[InventorySlot] = HotbarSlot;
            
            HotbarSlot->XForm = XForm;
            HotbarSlot->Sprite = Sprite;
            HotbarSlot->DrawColor = WHITE;
            
            item *Item = &Player->Inventory.Items[InventorySlot];
            Item->OccupiedInventorySlot = InventorySlot;
            
            Sprite = GetSprite(GameState, Item->Sprite);
            if(Sprite != GameState->GameData.Sprites[SPRITE_Nil] && !HotbarSlotState.IsHot)
            {
                XForm = mat4Multiply(XForm, mat4MakeScale(vec3{0.65, 0.65, 1.0}));
                DrawUISpriteXForm(TransientState, XForm, Sprite, 0, WHITE);
            }
            
            if(HotbarSlotState.IsHot)
            {
                HotbarSlot->DrawColor = RED;
                if(Item->Sprite != SPRITE_Nil)
                {
                    XForm = mat4Multiply(XForm, mat4MakeScale(vec3{1.0, 1.0, 1.0}));
                    DrawUISpriteXForm(TransientState, XForm, Sprite, 0, WHITE);
                    
                    real32 NewUIYOffset;
                    real32 NewUIYDescOffset;
                    real32 SpriteYOffset;
                    if(GameState->DisplayPlayerInventory)
                    {
                        NewUIYOffset = 24.0f;
                        NewUIYDescOffset = 8.0f;
                        SpriteYOffset = 25.0f;
                    }
                    else
                    {
                        NewUIYOffset = 16.0f;
                        NewUIYDescOffset = 3.0f;
                        SpriteYOffset = 20.0f;
                    }
                    
                    vec2 UIBoxSize = {10, 25};
                    
                    mat4 UIXForm = mat4Identity(1.0f);
                    UIXForm = mat4Multiply(UIXForm, mat4Translate(vec3{StartingX + SlotOffset, -90, 0.0}));
                    UIXForm = mat4Multiply(UIXForm, mat4Translate(vec3{IconSize * -0.55f, NewUIYOffset, 0.0f}));
                    
                    vec4 UIMatrixPosition = UIXForm.Columns[3];
                    vec2 Position = vec2{UIMatrixPosition.X, UIMatrixPosition.Y};
                    
                    static_sprite_data SpriteData = GetSprite(GameState, Item->Sprite);
                    
                    ui_element *ItemDescData = CloverUIMakeTextElement(&GameState->UIContext, Item->ItemDesc, {Position.X, Position.Y + NewUIYDescOffset}, 10, TEXT_ALIGNMENT_Center, GREEN);
                    CloverUIMakeTextElement(&GameState->UIContext, Item->ItemName, {Position.X + 4, Position.Y + NewUIYOffset}, 10, TEXT_ALIGNMENT_Center, GREEN);
                    
                    mat4 SpriteXForm = UIXForm;
                    UIXForm = mat4Multiply(UIXForm, mat4Translate(vec3{0, NewUIYOffset, 0}));
                    UIXForm = mat4Multiply(UIXForm, mat4MakeScale(v2Expand(UIBoxSize + ItemDescData->Size, 1.0f)));
                    DrawUISpriteXForm(TransientState, UIXForm, GetSprite(GameState, SPRITE_Nil), 0, vec4{0.2f, 0.2f, 0.2f, 0.2f});
                    
                    SpriteXForm = mat4Multiply(SpriteXForm, mat4Translate(vec3{UIBoxSize.X + ItemDescData->Size.X * -0.5f, SpriteYOffset, 0}));
                    SpriteXForm = mat4Multiply(SpriteXForm, mat4MakeScale(vec3{IconSize, IconSize, 1.0f}));
                    DrawUISpriteXForm(TransientState, SpriteXForm, SpriteData, 0, WHITE);
                    
                    SpriteXForm = mat4Multiply(SpriteXForm, mat4MakeScale(vec3{1 / IconSize, 1 / IconSize, 1.0f}));
                    SpriteXForm = mat4Multiply(SpriteXForm, mat4Translate(vec3{-(UIBoxSize.X + ItemDescData->Size.X * -0.5f), 0, 0}));
                    SpriteXForm = mat4Multiply(SpriteXForm, mat4Translate(vec3{(UIBoxSize.X + ItemDescData->Size.X * -0.5f), 19, 0}));
                    SpriteXForm = mat4Multiply(SpriteXForm, mat4Translate(vec3{-7, 0, 0}));
                    SpriteXForm = mat4Multiply(SpriteXForm, mat4MakeScale(vec3{16, 16, 1.0f}));
                    DrawUISpriteXForm(TransientState, SpriteXForm, GetSprite(GameState, SPRITE_Nil), 0, vec4{0.1f, 0.1f, 0.1f, 0.4f});
                    
                    SpriteXForm = mat4Multiply(SpriteXForm, mat4MakeScale(vec3{1 / IconSize, 1 / IconSize, 1.0f}));
                    SpriteXForm = mat4Multiply(SpriteXForm, mat4Translate(vec3{2, -6, 0}));
                    SpriteXForm = mat4Multiply(SpriteXForm, mat4MakeScale(vec3{16, 16, 1.0f}));
                    
                    vec4 ItemCountTextPosition = SpriteXForm.Columns[3];
                    Position = vec2{ItemCountTextPosition.X, ItemCountTextPosition.Y};

                    CloverUIMakeTextElement(&GameState->UIContext, sprints(&TransientState->Garbage, STR("x%d"), Item->CurrentStack), {Position.X + 3, Position.Y}, 15, TEXT_ALIGNMENT_Center, GREEN);
                }
            }
        }
    }
    
    // NOTE(Sleepster): Swapping Inventory Positions 
    for(uint32 InventoryIndexSlot = 0;
        InventoryIndexSlot < TOTAL_INVENTORY_SIZE;
        InventoryIndexSlot++)
    {
        item *Item = &Player->Inventory.Items[InventoryIndexSlot];
        ui_element *InventoryElement = Player->Inventory.InventorySlotButtons[InventoryIndexSlot]; 
        if(Item->CurrentStack == 0 && InventoryIndexSlot == Player->Inventory.CurrentInventorySlot)
        {
            ResetItemSlotState(Item);
        }
        
        if(InventoryElement)
        {
            // NOTE(Sleepster): Inventory Item Selection first item 
            if(InventoryElement->IsActive)
            {
                if(Item->Sprite != SPRITE_Nil && !Player->Inventory.SelectedInventoryItem)
                {
                    Player->Inventory.SelectedInventoryItem = Item;
                }
            }
            
            // TODO(Sleepster): Render the Item count if it's needed 
            if(Item->CurrentStack > 1 && Item->MaxStackCount > 1)
            {
                /* mat4 TextXForm = InventoryElement->XForm; */
                /* vec2 TextPosition = TextXForm.Columns[3].XY; */
                /* CloverUIPushLayer(&GameState->UIContext, 5); */
                /* CloverUIMakeTextElement(&GameState->UIContext, sprints(&TransientState->TransientArena, STR("x%d"), Item->CurrentStack), {TextPosition.X, TextPosition.Y}, 10, TEXT_ALIGNMENT_Left, GREEN); */
                /* CloverUIPushLayer(&GameState->UIContext, 0); */
            } 
            
            // NOTE(Sleepster): Scale sprite if selected 
            if(InventoryIndexSlot == Player->Inventory.CurrentInventorySlot)
            {
                InventoryElement->XForm = mat4Multiply(InventoryElement->XForm, mat4MakeScale(vec3{1.20f, 1.20f, 1.0f}));
            }
            
            // NOTE(Sleepster): Choose selection 
            if((Player->Inventory.SelectedInventoryItem && Player->Inventory.SelectedInventoryItem->Sprite != SPRITE_Nil))
            {
                item *Selection = Player->Inventory.SelectedInventoryItem;
                vec2 MousePos = TransformMouseCoords(RenderData->GameUICamera.ViewMatrix, 
                                                     RenderData->GameUICamera.ProjectionMatrix, 
                                                     GameState->GameInput.Keyboard.CurrentMouse, 
                                                     SizeData);
                
                static_sprite_data SelectionSprite = GetSprite(GameState, Selection->Sprite);
                
                CloverUIPushLayer(&GameState->UIContext, 1);
                CloverUISpriteElement(&GameState->UIContext, MousePos + vec2{1, 0}, v2Cast(SelectionSprite.SpriteSize), NULLMATRIX, SelectionSprite, WHITE);
                CloverUIPushLayer(&GameState->UIContext, 0);
                
                // NOTE(Sleepster): Dropping Selected inventory Items 
                if(IsGameKeyPressed(DROP_ITEM, &GameState->GameInput))
                {
                    entity *SpawnedItem = CreateEntity(GameState);
                    SetupDroppedEntity(RenderData, GameState, Selection, SpawnedItem);
                    
                    Player->Inventory.Items[Player->Inventory.SelectedInventoryItem->OccupiedInventorySlot] = {};
                    Player->Inventory.SelectedInventoryItem = {};
                }
            }
            
            // NOTE(Sleepster): Swap item stuff 
            if(!Player->Inventory.SwapItem)
            {
                item *Check = &Player->Inventory.Items[InventoryIndexSlot];
                if(InventoryElement->IsActive && Player->Inventory.SelectedInventoryItem && Check->OccupiedInventorySlot != Player->Inventory.SelectedInventoryItem->OccupiedInventorySlot)
                {
                    Player->Inventory.SwapItem = Check;
                }
            }
            
            // NOTE(Sleepster): Actually swapping the two 
            if(Player->Inventory.SwapItem && Player->Inventory.SelectedInventoryItem)
            {
                SwapInventoryItems(&Player->Inventory, Player->Inventory.SelectedInventoryItem, Player->Inventory.SwapItem); 
                Player->Inventory.SwapItem = {};
                Player->Inventory.SelectedInventoryItem = {};
            }
        }
    }
    
    // NOTE(Sleepster): Quickdropping HotbarItem
    if(IsGameKeyPressed(DROP_HELD, &GameState->GameInput))
    {
        item *HotbarItem = Player->Inventory.SelectedHotbarItem;
        if(HotbarItem && Player->Inventory.CurrentInventorySlot != NULLSLOT)
        {
            if(HotbarItem->CurrentStack != 0)
            {
                if(IsKeyDown(KEY_CONTROL, &GameState->GameInput))
                {
                    for(int32 ItemCount = 0;
                        ItemCount < HotbarItem->CurrentStack;
                        ItemCount++)
                    {
                        entity *DroppedEntity = CreateEntity(GameState);
                        SetupDroppedEntity(RenderData, GameState, HotbarItem, DroppedEntity);
                        HotbarItem->CurrentStack = 0;
                    }
                }
                else
                {
                    --HotbarItem->CurrentStack;
                    entity *DroppedEntity = CreateEntity(GameState);
                    SetupDroppedEntity(RenderData, GameState, HotbarItem, DroppedEntity);
                    DroppedEntity->DroppedFromInventoryItemCount = 1;
                }
            }
        }
    }
    
    // NOTE(Sleepster): UI Rendering 
    CloverUIDrawWidgets(RenderData, TransientState, &GameState->UIContext);
    CloverUIResetState(&GameState->UIContext); 
    
    // NOTE(Sleepster): Building
    {
        if(GameState->GameUIState == UI_State_Building)
        {
            const real32 IconSize = 12;
            const real32 Padding = 2;
            const real32 BoxWidth  = (IconSize * 5) + ((5 - 1) * Padding);
            
            real32 BoxHeight = 50;
            
            const real32 StartingXOffset = -IconSize * 2;
            
            int IconCount = 0;
            mat4 XForm = mat4Identity(1.0f);
            for(uint32 Element = 0;
                Element < ITEM_IDCount;
                Element++)
            {
                item *Item = &GameState->GameData.GameItems[Element];
                if(Item && Item->Flags & IS_BUILDABLE)
                {
                    real32 NewXOffset = StartingXOffset + ((IconSize + Padding) * IconCount);
                    XForm = mat4Identity(1.0f);
                    XForm = mat4Translation(XForm, vec3{NewXOffset, 10, 0});
                    
                    vec2 SpriteSize = {IconSize, IconSize};
                    if(GameState->ActiveBlueprint && Item->ItemID == GameState->ActiveBlueprint->ItemID)
                    {
                        SpriteSize = {14, 14};
                    }
                    XForm = mat4Scale(XForm, v2Expand(SpriteSize, 1));
                    CloverUISpriteElement(&GameState->UIContext, {0, 0}, {0, 0}, XForm, GetSprite(GameState, Item->Sprite), WHITE);
                    vec2 Position = XForm.Columns[3].XY;
                    
                    CloverUIPushLayer(&GameState->UIContext, 1);
                    ui_element_state Button = CloverUIButton(&GameState->UIContext, STR("Element"), Position, SpriteSize, GetSprite(GameState, SPRITE_Outline), WHITE);
                    CloverUIPushLayer(&GameState->UIContext, 0);
                    
                    IconCount++;
                    if((Button.IsPressed && !GameState->ActiveBlueprint) || (Button.IsPressed && Item->ItemID != GameState->ActiveBlueprint->ItemID))
                    {
                        GameState->ActiveBlueprint = Item;
                    }
                    else if(Button.IsPressed && GameState->ActiveBlueprint)
                    {
                        GameState->ActiveBlueprint = {};
                    }
                }
            }
            
            vec2 BoxPosition = {0, 0};
            if(GameState->ActiveBlueprint)
            {
                item *Item = GameState->ActiveBlueprint;
                BoxHeight = 115;
                BoxPosition = {0, -20};
                
                XForm = mat4Identity(1.0f);
                XForm = mat4Multiply(XForm, mat4Translate(v2Expand({BoxPosition.X, BoxPosition.Y + 20}, 0)));
                XForm = mat4Multiply(XForm, mat4MakeScale(vec3{BoxWidth, 2, 1}));
                
                CloverUIPushLayer(&GameState->UIContext, 1);
                DrawUISpriteXForm(TransientState, XForm, GetSprite(GameState, SPRITE_Nil), 0, vec4{0.0, 0.0, 0.0, 0.8f});
                CloverUIPushLayer(&GameState->UIContext, 0);
                
                int InventoryCount[MAX_CRAFTING_ELEMENTS] = {};
                for(uint32 InventorySlotIndex = 0;
                    InventorySlotIndex < TOTAL_INVENTORY_SIZE;
                    InventorySlotIndex++)
                {
                    item *InventoryItem = &Player->Inventory.Items[InventorySlotIndex];
                    for(int32 FormulaIndex = 0;
                        FormulaIndex < GameState->ActiveBlueprint->UniqueMaterialCount;
                        ++FormulaIndex)
                    {
                        crafting_material *FormulaItem = &GameState->ActiveBlueprint->CraftingFormula[FormulaIndex];
                        if(InventoryItem->ItemID == FormulaItem->CraftingMaterial)
                        {
                            InventoryCount[FormulaIndex] = InventoryItem->CurrentStack;
                        }
                    }
                }
                const real32 InitialYOffset = -20;
                for(int32 MaterialIndex = 0;
                    MaterialIndex < Item->UniqueMaterialCount;
                    MaterialIndex++)
                {
                    crafting_material *Material = &Item->CraftingFormula[MaterialIndex];
                    real32 NewYOffset = InitialYOffset - ((14 + Padding) * MaterialIndex);
                    
                    XForm = mat4Identity(1.0f);
                    XForm = mat4Translation(XForm, vec3{0, NewYOffset + 10, 0});
                    XForm = mat4Scale(XForm, vec3{30, 12, 1});
                    CloverUISpriteElement(&GameState->UIContext, {0, 0}, {0, 0}, XForm, GetSprite(GameState, SPRITE_Nil), {0.4, 0.4, 0.4, 0.3});
                    
                    XForm = mat4Identity(1.0f);
                    XForm = mat4Translation(XForm, vec3{-10, NewYOffset + 10, 0});
                    XForm = mat4Scale(XForm, vec3{8, 8, 1});
                    
                    static_sprite_data Sprite = GetSprite(GameState, GetSpriteFromPair(GameState, Material->CraftingMaterial));
                    
                    CloverUIPushLayer(&GameState->UIContext, 2);
                    CloverUISpriteElement(&GameState->UIContext, {0, 0}, {0, 0}, XForm, Sprite, WHITE);

                    CloverUIMakeTextElement(&GameState->UIContext, sprints(&TransientState->Garbage, STR("%d/%d"), InventoryCount[MaterialIndex], Material->RequiredCount), {10, NewYOffset + 5}, 10, TEXT_ALIGNMENT_Center, BLACK);
                    
                    CloverUIPushLayer(&GameState->UIContext, 0);
                }
                
                vec4 ButtonColor = {0.2, 0.2, 0.2, 0.4};
                CloverUIPushLayer(&GameState->UIContext, 4);
                ui_element_state Button = CloverUIButton(&GameState->UIContext, STR("Element"), {0, -65}, {40, 12}, GetSprite(GameState, SPRITE_Nil), ButtonColor);
                CloverUIMakeTextElement(&GameState->UIContext, STR("BUILD!"), {5, -70}, 10, TEXT_ALIGNMENT_Center, WHITE);
                CloverUIPushLayer(&GameState->UIContext, 0);
                
                ui_element *ButtonId = &GameState->UIContext.UIElements[Button.UIID.ID];
                if(Button.IsHot)
                {
                    ButtonId->DrawColor = {0.6, 0.2, 0.2, 0.4};
                    if(IsGameKeyDown(ATTACK, &GameState->GameInput))
                    {
                        ButtonId->Size = ButtonId->Size * 1.1;
                    }
                }
                
                if(Button.IsPressed)
                {
                    if(IsItemCraftable(InventoryCount, GameState->ActiveBlueprint))
                    {
                        for(uint32 InventorySlotIndex = 0;
                            InventorySlotIndex < TOTAL_INVENTORY_SIZE;
                            InventorySlotIndex++)
                        {
                            item *InventoryItem = &Player->Inventory.Items[InventorySlotIndex];
                            for(int32 FormulaIndex = 0;
                                FormulaIndex < GameState->ActiveBlueprint->UniqueMaterialCount;
                                ++FormulaIndex)
                            {
                                crafting_material *FormulaItem = &GameState->ActiveBlueprint->CraftingFormula[FormulaIndex];
                                if(InventoryItem->ItemID == FormulaItem->CraftingMaterial)
                                {
                                    InventoryItem->CurrentStack -= FormulaItem->RequiredCount;
                                    if(InventoryItem->CurrentStack <= 0)
                                    {
                                        ResetItemSlotState(InventoryItem);
                                    }
                                }
                            }
                        }
                        
                        // NOTE(Sleepster): Simply Drop the item in the player's inventory
                        switch(GameState->ActiveBlueprint->Archetype)
                        {
                            case ARCH_Workbench:
                            {
                                entity *Workbench = CreateEntity(GameState);
                                SetupItemWorkbench(GameState, Workbench);
                                Workbench->Position = Player->Position;
                                Workbench->Target   = Player->Position;
                            }break;
                            case ARCH_Furnace:
                            {
                                entity *Furnace = CreateEntity(GameState);
                                SetupItemFurnace(GameState, Furnace);
                                Furnace->Position = Player->Position;
                                Furnace->Target   = Player->Position;
                            }break;
                        }
                    }
                }
            }
            else
            {
                BoxPosition = {0, 12.5};
            }
            XForm = mat4Identity(1.0f);
            XForm = mat4Multiply(XForm, mat4Translate(v2Expand(BoxPosition, 0)));
            XForm = mat4Multiply(XForm, mat4MakeScale(vec3{BoxWidth, BoxHeight, 1}));
            
            DrawUISpriteXForm(TransientState, XForm, GetSprite(GameState, SPRITE_Nil), 0, vec4{0.0, 0.0, 0.0, 0.8f});
            DrawUIText(TransientState, RenderData, STR("Building..."), {-IconSize, 20}, 15, GF_UbuntuMono, WHITE);
        }
        
        // NOTE(Sleepster): Building From Inventory/Hotbar
        if((Player->Inventory.SelectedHotbarItem || Player->Inventory.SelectedInventoryItem) && (Player->Inventory.CurrentInventorySlot != NULLSLOT))
        {
            item *InventoryItem = {};
            item *HotbarItem = {};
            item *Item = {};
            for(uint32 InventoryIndex = 0;
                InventoryIndex < TOTAL_INVENTORY_SIZE;
                InventoryIndex++)
            {
                Item = &Player->Inventory.Items[InventoryIndex];
                if(Item == Player->Inventory.SelectedHotbarItem) 
                {
                    HotbarItem = Item;
                    break;
                }
                if(Item == Player->Inventory.SelectedInventoryItem) 
                {
                    InventoryItem = Item;
                    break;
                };
            }
            
            // NOTE(Sleepster): PLACING 
            vec2 MousePos = TransformMouseCoords(RenderData->GameCamera.ViewMatrix, 
                                              RenderData->GameCamera.ProjectionMatrix, 
                                              GameState->GameInput.Keyboard.CurrentMouse, 
                                              SizeData);
            real32 PlayerDist = v2Distance(Player->Position, MousePos);
            if((HotbarItem && PlayerDist < (ItemPickupDist * 2) && 
               (HotbarItem->Flags & IS_BUILDABLE)) || (InventoryItem && (InventoryItem->Flags & IS_BUILDABLE)))
            {
                vec2 MousePosition = RoundToTile(vec2{MouseToWorld.X + (TILE_SIZE * 0.5f), MouseToWorld.Y});
                static_sprite_data HotbarSprite = GetSprite(GameState, HotbarItem->Sprite);
                
                mat4 XForm = mat4Identity(1.0f);
                XForm = mat4Multiply(XForm, mat4Translate(vec3{MousePosition.X, MousePosition.Y  + (TILE_SIZE * 0.5f), 0}));
                XForm = mat4Multiply(XForm, mat4MakeScale(v2Expand(v2Cast(HotbarSprite.SpriteSize), 1)));
                DrawSpriteXForm(TransientState, XForm, HotbarSprite, 0, vec4{0.2, 0.2, 0.2, 0.3f});
                
                if(IsGameKeyPressed(INTERACT, &GameState->GameInput))
                {
                    bool32 Overlap = {};
                    for(uint32 EntityCounter = 0;
                        EntityCounter <= GameState->World.EntityCounter;
                        EntityCounter++)
                    {
                        entity *TestBuildingBounds = &GameState->World.Entities[EntityCounter];
                        Overlap = IsRangeWithinBounds(MouseToWorld, TestBuildingBounds->BoxCollider);
                        if(Overlap)
                        {
                            break;
                        }
                    }
                    if(!Overlap)
                    {
                        entity *Building = CreateEntity(GameState);
                        switch(Item->ItemID)
                        {
                            case ITEM_Workbench:
                            {
                                SetupBuildingWorkbench(GameState, Building);
                            }break;
                            case ITEM_Furnace:
                            {
                                SetupBuildingFurnace(GameState, Building);
                            }break;
                        }
                        Building->Position = MousePosition;
                        Building->BoxCollider = CreateRange(vec2{Building->Position.X - (TILE_SIZE * 0.5f), Building->Position.Y}, 
                                                            vec2{Building->Position.X - (TILE_SIZE * 0.5f), Building->Position.Y} + Building->Size);
                        
                        if(InventoryItem) ResetItemSlotState(InventoryItem);
                        if(HotbarItem) ResetItemSlotState(HotbarItem);
                    }
                }
            }
        }
    }
    
    // NOTE(Sleepster): Crafting
    {
        // NOTE(Sleepster): Can the crafting dialogue be displayed?
        if(IsGameKeyPressed(CRAFTING, &GameState->GameInput))
        {
            if(!GameState->ActiveCraftingStation && GameState->GameUIState != UI_State_Crafting)
            {
                for(uint32 EntityIndex = 0;
                    EntityIndex <= GameState->World.EntityCounter;
                    EntityIndex++)
                {
                    entity *Temp = &GameState->World.Entities[EntityIndex];
                    if(Temp->Flags & IS_PLACED)
                    {
                        real32 Distance = v2Distance(Player->Position, Temp->Position);
                        if(Distance <= ItemPickupDist && TransientState->SelectedEntityThisFrame) 
                        {
                            GameState->GameUIState = UI_State_Crafting;
                            GameState->ActiveCraftingStation = Temp;
                            break;
                        }
                    }
                }
            }
            else
            {
                GameState->GameUIState = UI_State_Nil;
                GameState->ActiveCraftingStation = {};
            }
        }
        
        // NOTE(Sleepster): If it can, display it
        if(GameState->GameUIState == UI_State_Crafting)
        {
            const real32 IconSize = 12;
            const real32 Padding = 2;
            const real32 BoxWidth  = (IconSize * 5) + ((5 - 1) * Padding);
            const real32 BoxHeight = 120;
            
            const vec2 BoxPosition = {-45, 0};
            const vec4 BoxColor = {0.0, 0.0, 0.0, 0.6};
            
            const real32 StartingXOffset = -BoxWidth;
            const real32 StartingYOffset = 32;
            
            // NOTE(Sleepster): This can be up to 4;
            int IconCount = 0;
            int RowCount = 0;
            
            mat4 XForm = mat4Identity(1.0f);
            XForm = mat4Translation(XForm, v2Expand(BoxPosition, 0));
            XForm = mat4Scale(XForm, vec3{BoxWidth, BoxHeight, 1});
            CloverUISpriteElement(&GameState->UIContext, {0, 0}, {0, 0}, XForm, GetSprite(GameState, SPRITE_Nil), BoxColor);
            
            XForm = mat4Identity(1.0f);
            XForm = mat4Translation(XForm, v2Expand(BoxPosition * -1, 0));
            XForm = mat4Scale(XForm, vec3{BoxWidth, BoxHeight, 1});
            CloverUISpriteElement(&GameState->UIContext, {0, 0}, {0, 0}, XForm, GetSprite(GameState, SPRITE_Nil), BoxColor);
            
            DrawUIText(TransientState, RenderData, STR("Crafting"), {-55, 40}, 15, GF_UbuntuMono, WHITE);
            for(uint32 Element = 0;
                Element < ITEM_IDCount;
                Element++)
            {
                item *Item = &GameState->GameData.GameItems[Element];
                if((Item && Item->Craftable) && Item && !(Item->Flags & IS_BUILDABLE))
                {
                    real32 NewXOffset = StartingXOffset + ((IconSize + Padding) * IconCount);
                    real32 NewYOffset = StartingYOffset - ((IconSize + Padding) * RowCount);
                    
                    XForm = mat4Identity(1.0f);
                    XForm = mat4Translation(XForm, vec3{NewXOffset, NewYOffset, 0});
                    
                    vec2 SpriteSize = {IconSize, IconSize};
                    if(Item == GameState->ActiveRecipe)
                    {
                        SpriteSize = {14, 14};
                    }
                    XForm = mat4Scale(XForm, v2Expand(SpriteSize, 1));
                    CloverUISpriteElement(&GameState->UIContext, {0, 0}, {0, 0}, XForm, GetSprite(GameState, Item->Sprite), WHITE);
                    vec2 Position = XForm.Columns[3].XY;
                    CloverUIPushLayer(&GameState->UIContext, 1);
                    ui_element_state Button = CloverUIButton(&GameState->UIContext, STR("Element"), Position, SpriteSize, GetSprite(GameState, SPRITE_Outline), WHITE);
                    CloverUIPushLayer(&GameState->UIContext, 0);
                    
                    IconCount++;
                    if(IconCount >= 4)
                    {
                        IconCount = 0;
                        RowCount = 1;
                    }
                    
                    if((Button.IsPressed && !GameState->ActiveRecipe) || (Button.IsPressed && Item->ItemID != GameState->ActiveRecipe->ItemID))
                    {
                        GameState->ActiveRecipe = Item;
                    }
                    else if(Button.IsPressed && GameState->ActiveRecipe)
                    {
                        GameState->ActiveRecipe = {};
                    }
                }
            }
            
            // NOTE(Sleepster): If we have an active recipe, display the recipe
            if(GameState->ActiveRecipe)
            {
                item *Item = GameState->ActiveRecipe;
                CloverUIMakeTextElement(&GameState->UIContext, Item->ItemName, {50, 40}, 15, TEXT_ALIGNMENT_Center, WHITE);
                
                XForm = mat4Identity(1.0f);
                XForm = mat4Translation(XForm, vec3{45, 30, 0});
                XForm = mat4Scale(XForm, vec3{IconSize, IconSize, 1});
                CloverUISpriteElement(&GameState->UIContext, {0, 0}, {0, 0}, XForm, GetSprite(GameState, Item->Sprite), WHITE);
                
                int InventoryCount[MAX_CRAFTING_ELEMENTS] = {};
                for(uint32 InventorySlotIndex = 0;
                    InventorySlotIndex < TOTAL_INVENTORY_SIZE;
                    InventorySlotIndex++)
                {
                    item *InventoryItem = &Player->Inventory.Items[InventorySlotIndex];
                    for(int32 FormulaIndex = 0;
                        FormulaIndex < GameState->ActiveRecipe->UniqueMaterialCount;
                        ++FormulaIndex)
                    {
                        crafting_material *FormulaItem = &GameState->ActiveRecipe->CraftingFormula[FormulaIndex];
                        if(InventoryItem->ItemID == FormulaItem->CraftingMaterial)
                        {
                            InventoryCount[FormulaIndex] = InventoryItem->CurrentStack;
                        }
                    }
                }
                
                const real32 InitialYOffset = 10;
                for(int32 MaterialIndex = 0;
                    MaterialIndex < Item->UniqueMaterialCount;
                    MaterialIndex++)
                {
                    crafting_material *Material = &Item->CraftingFormula[MaterialIndex];
                    real32 NewYOffset = InitialYOffset - ((14 + Padding) * MaterialIndex);
                    
                    XForm = mat4Identity(1.0f);
                    XForm = mat4Translation(XForm, vec3{45, NewYOffset, 0});
                    XForm = mat4Scale(XForm, vec3{30, 12, 1});
                    CloverUISpriteElement(&GameState->UIContext, {0, 0}, {0, 0}, XForm, GetSprite(GameState, SPRITE_Nil), {0.4, 0.4, 0.4, 0.3});
                    
                    XForm = mat4Identity(1.0f);
                    XForm = mat4Translation(XForm, vec3{40, NewYOffset, 0});
                    XForm = mat4Scale(XForm, vec3{8, 8, 1});
                    
                    static_sprite_data Sprite = GetSprite(GameState, GetSpriteFromPair(GameState, Material->CraftingMaterial));
                    
                    CloverUIPushLayer(&GameState->UIContext, 2);
                    CloverUISpriteElement(&GameState->UIContext, {0, 0}, {0, 0}, XForm, Sprite, WHITE);

                    CloverUIMakeTextElement(&GameState->UIContext, sprints(&TransientState->Garbage, STR("%d/%d"), InventoryCount[MaterialIndex], Material->RequiredCount), {55, NewYOffset - 5}, 10, TEXT_ALIGNMENT_Center, BLACK);

                    CloverUIPushLayer(&GameState->UIContext, 0);
                }
                
                vec4 ButtonColor = {0.2, 0.2, 0.2, 0.4};
                ui_element_state Button = CloverUIButton(&GameState->UIContext, STR("Element"), {45, -50}, {40, 12}, GetSprite(GameState, SPRITE_Nil), ButtonColor);
                ui_element *ButtonId = &GameState->UIContext.UIElements[Button.UIID.ID];
                if(Button.IsHot)
                {
                    ButtonId->DrawColor = {0.6, 0.2, 0.2, 0.4};
                    if(IsGameKeyDown(ATTACK, &GameState->GameInput))
                    {
                        ButtonId->Size = ButtonId->Size * 1.1;
                    }
                }
                
                if(Button.IsPressed)
                {
                    if(IsItemCraftable(&InventoryCount[0], GameState->ActiveRecipe))
                    {
                        for(int32 Index = 0;
                            Index < GameState->ActiveRecipe->FormulaResultCount;
                            Index++)
                        {
                            entity *CraftedItem = CreateEntity(GameState);
                            SetupDroppedEntity(RenderData, GameState, GameState->ActiveRecipe, CraftedItem);
                            CraftedItem->Position = GameState->ActiveCraftingStation->Position;
                            CraftedItem->Target = GameState->ActiveCraftingStation->Position;
                        }
                        
                        for(uint32 InventorySlotIndex = 0;
                            InventorySlotIndex < TOTAL_INVENTORY_SIZE;
                            InventorySlotIndex++)
                        {
                            item *InventoryItem = &Player->Inventory.Items[InventorySlotIndex];
                            for(int32 FormulaIndex = 0;
                                FormulaIndex < GameState->ActiveRecipe->UniqueMaterialCount;
                                ++FormulaIndex)
                            {
                                crafting_material *FormulaItem = &GameState->ActiveRecipe->CraftingFormula[FormulaIndex];
                                if(InventoryItem->ItemID == FormulaItem->CraftingMaterial)
                                {
                                    InventoryItem->CurrentStack -= FormulaItem->RequiredCount;
                                    if(InventoryItem->CurrentStack <= 0)
                                    {
                                        ResetItemSlotState(InventoryItem);
                                    }
                                }
                            }
                        }
                    }
                }
                CloverUIMakeTextElement(&GameState->UIContext, STR("Craft"), {48, -55}, 10, TEXT_ALIGNMENT_Center, BLACK);
            }
            else
            {
                CloverUIMakeTextElement(&GameState->UIContext, STR("Please Select an Item"), {50, 0}, 10, TEXT_ALIGNMENT_Center, WHITE);
            }
            
            // NOTE(Sleepster): If the player gets to far, stop displaying it
            real32 Distance = v2Distance(Player->Position, GameState->ActiveCraftingStation->Position);
            if(Distance > ItemPickupDist)
            {
                GameState->GameUIState = UI_State_Nil;
                GameState->ActiveCraftingStation = {};
            }
        }
    }
    
    
    // TRANSPARENCY TEST
    {
        mat4 Identity  = mat4Identity(1.0f);
        mat4 Translate = mat4Multiply(Identity, mat4Translate(vec3{10.0f, 10.0f, 0.0f}));
        mat4 Scale     = mat4Multiply(Identity, mat4MakeScale(vec3{100.0f, 100.0f, 1.0f}));
        
        mat4 Total = Translate * Scale;
        DrawRectXForm(TransientState, Total, {16, 16}, 0, vec4{1.0f, 0.0f, 1.0f, 0.3f});
    }
    
    
    // NOTE(Sleepster): Sorting, off for now. Breaks too much 
    {
        //qsort(GameState->World.Entities, GameState->World.EntityCounter, sizeof(struct entity), CompareEntityYAxis); 
    }
    
    for(uint32 EntityIndex = 0;
        EntityIndex <= GameState->World.EntityCounter;
        ++EntityIndex)
    {
        entity *Temp = &GameState->World.Entities[EntityIndex];
        if((Temp->Flags & IS_VALID))
        {
            real32 Distance = fabsf(v2Distance(Temp->Position, MouseToWorld));
            real32 PlayerToObjectDistance = fabsf(v2Distance(Temp->Position, Player->Position));
            if(Distance <= SelectionDistance && PlayerToObjectDistance <= MaxHitRange)
            {
                if(!TransientState->SelectedEntityThisFrame || (Distance < MinimumDistance))
                {
                    TransientState->SelectedEntityThisFrame = Temp;
                    MinimumDistance = Distance;
                }
            }
        }
    }
    
    vec2 SelectionBoxDrawSize = {16, 16};
    // NOTE(Sleepster): DRAW ENTITIES
    for(uint32 EntityIndex = 0;
        EntityIndex <= GameState->World.EntityCounter;
        ++EntityIndex)
    {
        entity *Temp = &GameState->World.Entities[EntityIndex];
        SelectionBoxDrawSize.X = SinBreatheNormalized(Time.CurrentTimeInSeconds, 0.5f, 13.0f, 15.0f);
        SelectionBoxDrawSize.Y = SinBreatheNormalized(Time.CurrentTimeInSeconds, 0.5f, 13.0f, 15.0f);
        
        if(Temp->Flags & IS_VALID)
        {
            switch(Temp->Archetype)
            {
                case ARCH_Player:
                {
                    Player = Temp;
                    HandleInput(GameState, Temp, Time);
                    RenderData->GameCamera.Target = Temp->Position;
                    
                    v2Approach(&RenderData->GameCamera.Position, RenderData->GameCamera.Target, 5.0f, Time.Delta);
                    DrawEntity(TransientState, GameState, Temp, Temp->Position, WHITE);
                }break;
                default:
                {
                    if(Temp->Flags & IS_ITEM)
                    {
                        v2Approach(&Temp->Position, Temp->Target, 5.0f, Time.Delta);
                        Temp->Position.Y += 0.01f * SinBreathe(Time.CurrentTimeInSeconds, 1.25f);
                        if(v2Distance(Temp->Target, Temp->Position) <= PickupEpsilon)
                        {
                            Temp->Flags += CAN_BE_PICKED_UP;
                        }
                    }
                    
                    if(TransientState->SelectedEntityThisFrame == Temp && !(Temp->Flags & IS_ITEM))
                    {
                        TransientState->SelectedEntityThisFrame = Temp;
                        static_sprite_data SelectionBoxSprite = GetSprite(GameState, SPRITE_SelectionBox);
                        static_sprite_data EntitySprite = GetSprite(GameState, Temp->Sprite);
                        
                        DrawSprite(TransientState, 
                                   SelectionBoxSprite, 
                                   Temp->Position 
                                   - vec2{0, real32(EntitySprite.SpriteSize.Y * 0.25f)},
                                   SelectionBoxDrawSize, 
                                   WHITE, 
                                   0, 
                                   0);
                    }
                    DrawEntity(TransientState, GameState, Temp, Temp->Position, WHITE);
                }break;
            }
        }
    }
    
    // NOTE(Sleepster): DRAW FLOOR TILES
    ivec2  PlayerOffset = WorldToTilePos(Player->Position);
    ivec2  TileRadius   = {14, 16};
    
    for(int32 TileX = PlayerOffset.X - TileRadius.X;
        TileX < PlayerOffset.X + TileRadius.Y;
        ++TileX)
    {
        for(int32 TileY = PlayerOffset.Y - TileRadius.Y;
            TileY < PlayerOffset.Y + TileRadius.Y;
            ++TileY)
        {
            if((TileX + (TileY % 2 == 0)) % 2 == 0)
            {
                real32 X = TileX * TILE_SIZE;
                real32 Y = TileY * TILE_SIZE;
                DrawQuadTextured(TransientState, {X, Y - (TILE_SIZE)}, vec2{16, 16}, ivec2{0, 0}, ivec2{16, 16}, 0, DARK_GRAY, 0, 0);
            }
            else
            {
                real32 X = TileX * TILE_SIZE;
                real32 Y = TileY * TILE_SIZE;
                DrawQuadTextured(TransientState, {X, Y - (TILE_SIZE)}, vec2{16, 16}, ivec2{0, 0}, ivec2{16, 16}, 0, DARKER_GRAY, 0, 0);
            }
        }
    }
    
    attenuation_data TestLightData = {.Constant = 0.05, .Linear = 0.0009, .Quadratic = 0.001};
    CreatePointLight(TransientState, vec2{ 80, 80}, 2.0, 100, &TestLightData, RED);
    CreatePointLight(TransientState, vec2{-80, 80}, 2.0, 100, &TestLightData, WHITE);
    
    if(IsGamepadButtonDown(A_BUTTON, &GameState->GameInput))
    {
        GameState->GameInput.Controller.LeftRumble = 1000;
        GameState->GameInput.Controller.RightRumble = 1000;
    }
}

extern
GAME_GET_SOUND_SAMPLES(GameGetSoundSamples)
{
    real32 *MixerBuffer00 = PushArray(&TransientState->Garbage, real32, SoundBuffer->SampleOutputCount);
    real32 *MixerBuffer01 = PushArray(&TransientState->Garbage, real32, SoundBuffer->SampleOutputCount);

    real32 *Dest00 = MixerBuffer00;
    real32 *Dest01 = MixerBuffer01;

    for(int32 SampleIndex = 0;
            SampleIndex < SoundBuffer->SampleOutputCount;
            ++SampleIndex)
    {
        *Dest00++ = 0;
        *Dest01++ = 0;
    }

    real32 MasterVolume  = 1.0f;
    real32 FadeOutMultiplier = 1.0f;
    for(playing_sound **PlayingSoundptr = &GameState->FirstPlayingSound;
        *PlayingSoundptr;
       )
    {
        playing_sound *PlayingSound = *PlayingSoundptr;
        uint32 TotalSamplesToMix = SoundBuffer->SampleOutputCount;
        bool IsFinished = false;
        Dest00 = MixerBuffer00;
        Dest01 = MixerBuffer01;

        loaded_sound *CurrentSound = GetSoundFromID(GameMemory, (soundfx_id)PlayingSound->ID, PlayingSound->StreamedFromFile,
                                                    PlayingSound->SamplesCursor, SoundBuffer->SampleOutputCount);
        if(CurrentSound->SampleCount != 0)
        {
            GetSoundFromID(GameMemory, (soundfx_id)PlayingSound->NextIDToPlay);

            uint32 SamplesToMix = TotalSamplesToMix;
            uint32 SamplesRemaining = (CurrentSound->SampleCount - PlayingSound->SamplesCursor);
            if(SamplesToMix > SamplesRemaining)
            {
                SamplesToMix = SamplesRemaining;
            }

            for(uint32 SampleIndex = 0;
                SampleIndex < SamplesToMix;
                ++SampleIndex)
            {
                uint32 FadeParameter = CurrentSound->SampleCount - PlayingSound->SamplesCursor; 
                if(FadeParameter < 256)
                {
                    FadeOutMultiplier = (real32)FadeParameter / 256.0f;
                }
                int32  SampleOffset           = (PlayingSound->SamplesCursor + SampleIndex) % CurrentSound->SampleCount;
                real32 LeftSampleValue        = CurrentSound->Samples[SampleOffset * 2] * FadeOutMultiplier;
                real32 RightSampleValue       = CurrentSound->Samples[(SampleOffset * 2) + 1] * FadeOutMultiplier;

                *Dest00++ += LeftSampleValue  * PlayingSound->Volume[0];
                *Dest01++ += RightSampleValue * PlayingSound->Volume[1];            
            }

            PlayingSound->SamplesCursor += SamplesToMix;
            TotalSamplesToMix -= SamplesToMix;
            if(PlayingSound->SamplesCursor >= CurrentSound->SampleCount)
            {
                if(IsValid(PlayingSound->NextIDToPlay))
                {
                    PlayingSound->ID = PlayingSound->NextIDToPlay;
                    PlayingSound->SamplesCursor = 0;
                }
                else
                {
                    IsFinished = true;
                }
            }
        }

        if(IsFinished)
        {
            *PlayingSoundptr = PlayingSound->Next;
             PlayingSound->Next = GameState->FirstFreePlayingSound;
             GameState->FirstFreePlayingSound = PlayingSound;
        }
        else
        {
            PlayingSoundptr = &PlayingSound->Next;
        }
    }

    Dest00 = MixerBuffer00;
    Dest01 = MixerBuffer01;
    int16 *SampleOut = SoundBuffer->SampleBuffer;
    for(int32 SampleIndex = 0;
        SampleIndex < SoundBuffer->SampleOutputCount;
        ++SampleIndex)
    {
        *SampleOut++ = int16((*Dest00++ * MasterVolume) + 0.5f);
        *SampleOut++ = int16((*Dest01++ * MasterVolume) + 0.5f);
    }
}
