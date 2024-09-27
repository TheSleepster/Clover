#if !defined(CLOVER_UI_H)
/* ========================================================================
   $File: Clover_UI.h $
   $Date: September 26 2024 10:14 am $
   $Revision: $
   $Creator: Justin Lewis $
   ======================================================================== */

#define CLOVER_UI_H

#include "Intrinsics.h"

// UTILS
#include "util/Math.h"
#include "util/Array.h"
#include "util/FileIO.h"
#include "util/CustomStrings.h"

// CLOVER HEADERS
#include "Clover.h"
#include "Clover_Globals.h"
#include "Clover_Renderer.h"
#include "Clover_Input.h"

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

enum ui_type : int8
{
    UI_Nil,  
    UI_Button,  
    UI_Text,
    UI_TextBox,
    UI_ItemSprite,
};

enum ui_text_alignment : int8
{
    TEXT_ALIGNMENT_Left,
    TEXT_ALIGNMENT_Right,
    TEXT_ALIGNMENT_Center,
};

struct ui_id
{
    int32    ID;
    ui_layer LayerIdx;
};

struct ui_element
{
    ui_type Type;
    ui_id UIID;  

    string ElementName;
    string ElementText;
    ui_text_alignment TextAlignment;
    real32 FontScale;
    vec4 TextDrawColor;

    static_sprite_data Sprite;

    vec2 Position;
    vec2 TextOrigin;
    vec2 Size;
    vec4 DrawColor;

    range_v2 OccupiedRange;

    bool IsValid;
    bool IsDisplayed;
    bool IsHot;
    bool IsActive;
    bool HasText;
    
    mat4 XForm;
};

struct ui_element_state
{
    bool IsHot;
    bool IsPressed;
    ui_id UIID;
};

struct clover_ui_context
{
    ui_id HotLastFrame;
    ui_id ActiveLastFrame;

    ui_layer ActiveLayer;

    int32 CreatedElementCounter;
    ui_element UIElements[MAX_UI_ELEMENTS];

    mat4 UICameraViewMatrix;
    mat4 UICameraProjectionMatrix;

    Input     *GameInput;
    font_data *ActiveFont;
    font_index ActiveFontIndex;
    real32     LastActiveFontSize;
};

#endif // CLOVER_UI_H

