/* ========================================================================
   $File: Clover_Entity.cpp $
   $Date: Sat, 23 Nov 24: 09:32AM $
   $Revision: $
   $Creator: Justin Lewis $
   ======================================================================== */

internal entity*
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

    Entity->WhenStruck  =
    {
        .IsActive = false,
        .IDToPlay = GSFX_Bap,
    };
    
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

    Entity->WhenStruck  =
    {
        .IsActive = false,
        .IDToPlay = GSFX_Bap,
    };
    
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

    Entity->WhenStruck  =
    {
        .IsActive = false,
        .IDToPlay = GSFX_Bap,
    };
    
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

    Entity->WhenStruck  =
    {
        .IsActive = false,
        .IDToPlay = GSFX_Bap,
    };
    
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

    Entity->WhenStruck  =
    {
        .IsActive = false,
        .IDToPlay = GSFX_Bap,
    };

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
