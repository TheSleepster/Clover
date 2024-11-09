#include "Clover_Input.h"
#include "Clover.h"

#include "Intrinsics.h"

// UTILS
#include "util/Math.h"
#include "util/Array.h"
#include "util/FileIO.h"
#include "util/Arena.h"
#include "util/String.h"

internal inline game_mapping
AddGameMapping(keycodeID MainKey, keycodeID AltKey, 
               controller_buttonID PrimaryButton,
               controller_buttonID SecondaryButton) 
{
    game_mapping Result = {};
    Result.MainKey = MainKey;
    Result.AltKey = AltKey;

    Result.PrimaryButton = PrimaryButton;
    Result.SecondaryButton = SecondaryButton;

    return(Result);
}

internal inline bool
IsKeyPressed(keycodeID Keycode, input *GameInput)
{
    keyboard_key inputKey = GameInput->Keyboard.Keys[Keycode];
    return(inputKey.IsDown && inputKey.HalfTransitionCount >= 1);
}

internal inline bool
IsKeyReleased(keycodeID Keycode, input *GameInput)
{
    keyboard_key inputKey = GameInput->Keyboard.Keys[Keycode];
    return(!inputKey.IsDown && inputKey.HalfTransitionCount >= 1);
}

internal inline bool 
IsKeyDown(keycodeID Keycode, input *GameInput)
{
    return(GameInput->Keyboard.Keys[Keycode].IsDown);
}

internal inline void
ConsumeKeyinput(keycodeID KeyCode, input *GameInput)
{
    GameInput->Keyboard.Keys[KeyCode].HalfTransitionCount = 0;
}

internal inline bool 
IsGameKeyDown(KeyBindings InputType, input *GameInput)
{
    keyboard_key InputKey     = GameInput->Keyboard.Keys[GameInput->Mappings[InputType].MainKey];
    keyboard_key AltInputKey  = GameInput->Keyboard.Keys[GameInput->Mappings[InputType].AltKey];

    action_button InputButton = GameInput->Controller.GamepadButtons[GameInput->Mappings[InputType].PrimaryButton];
    action_button AltButton = GameInput->Controller.GamepadButtons[GameInput->Mappings[InputType].SecondaryButton];

    return(InputKey.IsDown || AltInputKey.IsDown || InputButton.IsDown || AltButton.IsDown);
}

internal inline bool
IsGameKeyPressed(KeyBindings InputType, input *GameInput)
{
    keyboard_key InputKey    = GameInput->Keyboard.Keys[GameInput->Mappings[InputType].MainKey];
    keyboard_key AltInputKey = GameInput->Keyboard.Keys[GameInput->Mappings[InputType].AltKey];

    action_button InputButton = GameInput->Controller.GamepadButtons[GameInput->Mappings[InputType].PrimaryButton];
    action_button AltButton = GameInput->Controller.GamepadButtons[GameInput->Mappings[InputType].SecondaryButton];

    return((InputKey.IsDown && InputKey.HalfTransitionCount >= 1) || 
           (AltInputKey.IsDown && AltInputKey.HalfTransitionCount >= 1) ||
           (InputButton.IsDown && InputButton.HalfTransitionCount >= 1) ||
           (AltButton.IsDown && AltButton.HalfTransitionCount >= 1));
}


internal inline bool
IsGameKeyReleased(KeyBindings InputType, input *GameInput)
{
    keyboard_key InputKey    = GameInput->Keyboard.Keys[GameInput->Mappings[InputType].MainKey];
    keyboard_key AltInputKey = GameInput->Keyboard.Keys[GameInput->Mappings[InputType].AltKey];

    action_button InputButton = GameInput->Controller.GamepadButtons[GameInput->Mappings[InputType].PrimaryButton];
    action_button AltButton = GameInput->Controller.GamepadButtons[GameInput->Mappings[InputType].SecondaryButton];

    return((!InputKey.IsDown && InputKey.HalfTransitionCount >= 1) || 
           (!AltInputKey.IsDown && AltInputKey.HalfTransitionCount >= 1) ||
           (!InputButton.IsDown && InputButton.HalfTransitionCount >= 1) ||
           (!AltButton.IsDown && AltButton.HalfTransitionCount >= 1));
}

internal inline void
ConsumeGameKeyinput(KeyBindings InputType, input *GameInput)
{
    keyboard_key InputKey    = GameInput->Keyboard.Keys[GameInput->Mappings[InputType].MainKey];
    keyboard_key AltInputKey = GameInput->Keyboard.Keys[GameInput->Mappings[InputType].AltKey];
    
    InputKey.HalfTransitionCount = 0;
    InputKey.IsDown = 1;
    InputKey.JustPressed = 1;
    InputKey.JustReleased = 0;
    
    AltInputKey.HalfTransitionCount = 0;
    AltInputKey.IsDown = 1;
    AltInputKey.JustPressed = 1;
    AltInputKey.JustReleased = 0;
}
