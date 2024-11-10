/* date = August 27 2024 05:59 am*/

#ifndef CLOVER_INPUT_H
#define CLOVER_INPUT_H

#if 0
#include "../data/deps/SDL3/include/SDL3/SDL.h"
#include "../data/deps/SDL3/include/SDL3/SDL_gamepad.h"
#include "../data/deps/SDL3/include/SDL3/SDL_joystick.h"
#endif

#include "Intrinsics.h"
#include "util/Math.h"

enum keycodeID 
{ 
    KEY_NONE,
    KEY_LEFT_MOUSE,
    KEY_MIDDLE_MOUSE,
    KEY_RIGHT_MOUSE,
    
    KEY_A, KEY_B, KEY_C, KEY_D, KEY_E, KEY_F, KEY_G, KEY_H, KEY_I, KEY_J,
    KEY_K, KEY_L, KEY_M, KEY_N, KEY_O, KEY_P, KEY_Q, KEY_R, KEY_S, KEY_T,
    KEY_U, KEY_V, KEY_W, KEY_X, KEY_Y, KEY_Z,
    
    KEY_0, KEY_1, KEY_2, KEY_3, KEY_4, KEY_5, KEY_6, KEY_7, KEY_8, KEY_9,
    
    KEY_SPACE,
    KEY_TICK,
    KEY_MINUS,
    KEY_EQUAL,
    KEY_LEFT_BRACKET,
    KEY_RIGHT_BRACKET,
    KEY_SEMICOLON,
    KEY_QUOTE,
    KEY_COMMA,
    KEY_PERIOD,
    KEY_FORWARD_SLASH,
    KEY_BACKWARD_SLASH,
    KEY_TAB,
    KEY_ESCAPE,
    KEY_PAUSE,
    KEY_UP,
    KEY_DOWN,
    KEY_LEFT,
    KEY_RIGHT,
    KEY_BACKSPACE,
    KEY_RETURN,
    KEY_DELETE,
    KEY_INSERT,
    KEY_HOME,
    KEY_END,
    KEY_PAGE_UP,
    KEY_PAGE_DOWN,
    KEY_CAPS_LOCK,
    KEY_NUM_LOCK,
    KEY_SCROLL_LOCK,
    KEY_MENU,
    KEY_SHIFT,
    KEY_CONTROL,
    KEY_ALT,
    KEY_COMMAND,
    
    KEY_F1, KEY_F2, KEY_F3, KEY_F4, KEY_F5, KEY_F6, 
    KEY_F7, KEY_F8, KEY_F9, KEY_F10, KEY_F11, KEY_F12,
    
    KEY_NUMPAD_0,
    KEY_NUMPAD_1,
    KEY_NUMPAD_2,
    KEY_NUMPAD_3,
    KEY_NUMPAD_4,
    KEY_NUMPAD_5,
    KEY_NUMPAD_6,
    KEY_NUMPAD_7,
    KEY_NUMPAD_8,
    KEY_NUMPAD_9,
    
    KEY_NUMPAD_STAR,
    KEY_NUMPAD_PLUS,
    KEY_NUMPAD_MINUS,
    KEY_NUMPAD_DOT,
    KEY_NUMPAD_SLASH,
    
    KEY_COUNT = 256,
};

enum KeyBindings 
{
    BINDING_NONE,
    MOVE_UP,
    MOVE_DOWN,
    MOVE_LEFT,
    MOVE_RIGHT,
    ATTACK,
    INTERACT,
    INVENTORY,
    SHOW_HOTBAR,
    HOTBAR_01,
    HOTBAR_02,
    HOTBAR_03,
    HOTBAR_04,
    HOTBAR_05,
    HOTBAR_06,
    HOTBAR_07,
    DROP_ITEM,
    DROP_HELD,
    CRAFTING,
    BUILD_MENU,
    BINDING_COUNT,
};

// NOTE(Sleepster): This is based off the Xbox One (2015) controler
enum controller_buttonID
{
    DPAD_UP,
    DPAD_DOWN,
    DPAD_LEFT,
    DPAD_RIGHT,

    START_BUTTON,
    BACK_BUTTON,

    LEFT_THUMBSTICK_DOWN,
    RIGHT_THUMBSTICK_DOWN,

    LEFT_SHOULDER_BUTTON,
    RIGHT_SHOULDER_BUTTON,

    A_BUTTON,
    B_BUTTON,
    X_BUTTON,
    Y_BUTTON,

    GAMEPAD_BUTTON_COUNT = 14,
    NULL_BUTTON,
};

#if 0

struct action_button
{
    bool32 JustPressed;
    bool32 JustReleased;
    bool32 IsDown;
    uint8 HalfTransitionCount;
};

struct key_mapping 
{
    keycodeID MainKey;
    keycodeID AltKey;

    controller_buttonID PrimaryButton;
    controller_buttonID SecondaryButton;
};

struct keyboard_input 
{
    ivec2 LastMouse;
    ivec2 CurrentMouse;
    ivec2 DeltaMouse;
    
    action_button Keys[KEY_COUNT];
    key_mapping Bindings[BINDING_COUNT];
};

struct controller_input
{
    action_button GamepadButtons[GAMEPAD_BUTTON_COUNT];

    int16 LeftStickX;
    int16 LeftStickY;

    int16 RightStickX;
    int16 RightStickY;

    uint8 LeftTriggerValue;
    uint8 RightTriggerValue;

    int32 Rumble;
};

struct player_controller
{
    bool IsController;

    controller_input Controller;
    keyboard_input   Keyboard;
};

#endif

struct action_button
{
    bool32 JustPressed;
    bool32 JustReleased;
    bool32 IsDown;
    uint8 HalfTransitionCount;
};

struct gamepad_input
{
    action_button GamepadButtons[GAMEPAD_BUTTON_COUNT];

    ivec2 LeftStick;
    ivec2 RightStick;

    uint8 LeftTriggerValue;
    uint8 RightTriggerValue;

    uint16 LeftRumble;
    uint16 RightRumble;

    int32 ControllerIndex;
};

struct keyboard_key 
{
    bool32 JustPressed;
    bool32 JustReleased;
    bool32 IsDown;
    uint8  HalfTransitionCount;
};

struct game_mapping 
{
    keycodeID MainKey;
    keycodeID AltKey;

    controller_buttonID PrimaryButton;
    controller_buttonID SecondaryButton;
};

struct keyboard_input 
{
    ivec2 LastMouse;
    ivec2 CurrentMouse;
    ivec2 DeltaMouse;
    
    keyboard_key Keys[KEY_COUNT];
};

struct input
{
    keycodeID KeyCodeLookup[KEY_COUNT];
    keyboard_input Keyboard;

    uint16 ButtonLookup[GAMEPAD_BUTTON_COUNT];
    gamepad_input  Controller;

    game_mapping   Mappings[BINDING_COUNT];
};

#endif // _CLOVER_INPUT_H

