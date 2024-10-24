/* date = August 30 2024 05:13 am*/

#ifndef CLOVER_GLOBALS_H
#define CLOVER_GLOBALS_H

#include "Intrinsics.h"

#include "util/Math.h"
#include "util/Array.h"
#include "util/FileIO.h"
#include "util/CustomStrings.h"

// PLATFORM DATA
global_variable ivec4 SizeData = {0, 0, 1920, 1080};
global_variable bool  Running  = 0;

// TIMING DATA
global_variable int64 PerfCountFrequency;
global_variable real64 DeltaCounter;


// COLORS
constexpr vec4 WHITE     = {1.0f, 1.0f, 1.0f, 1.0f};
constexpr vec4 GRAY      = {0.1f, 0.1f, 0.1f, 1.0f};
constexpr vec4 BLACK     = {0.0f, 0.0f, 0.0f, 1.0f};
constexpr vec4 RED       = {1.0f, 0.0f, 0.0f, 1.0f};
constexpr vec4 YELLOW    = {1.0f, 1.0f, 0.0f, 1.0f};
constexpr vec4 ORANGE    = {1.0f, 0.2f, 0.0f, 1.0f};
constexpr vec4 GREEN     = {0.0f, 1.0f, 0.0f, 1.0f};
constexpr vec4 BLUE      = {0.0f, 0.0f, 1.0f, 1.0f};
constexpr vec4 TEAL      = {0.1f, 0.6f, 1.0f, 1.0f};
constexpr vec4 PURPLE    = {0.4f, 0.2f, 1.0f, 1.0f};
constexpr vec4 DARK_GRAY = {0.05f, 0.05f, 0.05f, 1.0f};


// RENDERER STUFF
constexpr real32 SIMRATE = (1.0f/90.0f);
constexpr uint32 MAX_QUADS    = 10000;
constexpr uint32 MAX_VERTICES = MAX_QUADS * 4;
constexpr uint32 MAX_INDICES  = MAX_QUADS * 6;
constexpr uint32 TRUE_MAX_VERTICES = MAX_VERTICES * 2;

constexpr uint32 MAX_POINT_LIGHTS = 1000;
constexpr uint32 MAX_SPOT_LIGHTS  = 1000;


// POST PROCESSING VALUES
global_variable real32 RenderBrightness = 1.0f;


// GAME GLOBALS
constexpr uint32 MAX_SOUNDS      = 128;
constexpr uint32 MAX_TRACKS      = 12;
constexpr uint32 MAX_ENTITIES    = 10000;
constexpr uint32 MAX_UI_ELEMENTS = 1000;

constexpr real32 WORLD_SIZE   = 100;
constexpr real32 TILE_SIZE    = 16;

constexpr int32 PlayerLifeCount = 3;
constexpr int32 PlayerHealth    = PlayerLifeCount * 2;

constexpr int32 RockHealth      = 2;
constexpr int32 TreeHealth      = 2;
constexpr int32 NodeHealth      = 4;

constexpr real32 ItemPickupDist  = 30.0f;
constexpr real32 MaxHitRange     = 60.0f;
constexpr real32 MaxDropDistance = 60.0f;

// NOTE(Sleepster): not a constexpr because it may change at 
constexpr uint32 PLAYER_HOTBAR_COUNT = 7;
constexpr uint32 PLAYER_INVENTORY_SIZE = 15;
constexpr uint32 MAX_CRAFTING_ELEMENTS = 9;
constexpr uint32 MAX_ENTITY_DROPS      = 4;
constexpr uint32 TOTAL_INVENTORY_SIZE = PLAYER_HOTBAR_COUNT + PLAYER_INVENTORY_SIZE;

constexpr real32 PickupEpsilon = 5.0f;

#define NULLSLOT 100

#endif // _CLOVER_GLOBALS_H

