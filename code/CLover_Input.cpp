#include "Clover_Input.h"
#include "Clover.h"
#include "Intrinsics.h"

internal void 
Win32LoadKeyData(game_state *State) 
{ 
    State->KeyCodeLookup[VK_LBUTTON] = KEY_LEFT_MOUSE;
    State->KeyCodeLookup[VK_MBUTTON] = KEY_MIDDLE_MOUSE;
    State->KeyCodeLookup[VK_RBUTTON] = KEY_RIGHT_MOUSE;
    
    State->KeyCodeLookup['A'] = KEY_A;
    State->KeyCodeLookup['B'] = KEY_B;
    State->KeyCodeLookup['C'] = KEY_C;
    State->KeyCodeLookup['D'] = KEY_D;
    State->KeyCodeLookup['E'] = KEY_E;
    State->KeyCodeLookup['F'] = KEY_F;
    State->KeyCodeLookup['G'] = KEY_G;
    State->KeyCodeLookup['H'] = KEY_H;
    State->KeyCodeLookup['I'] = KEY_I;
    State->KeyCodeLookup['J'] = KEY_J;
    State->KeyCodeLookup['K'] = KEY_K;
    State->KeyCodeLookup['L'] = KEY_L;
    State->KeyCodeLookup['M'] = KEY_M;
    State->KeyCodeLookup['N'] = KEY_N;
    State->KeyCodeLookup['O'] = KEY_O;
    State->KeyCodeLookup['P'] = KEY_P;
    State->KeyCodeLookup['Q'] = KEY_Q;
    State->KeyCodeLookup['R'] = KEY_R;
    State->KeyCodeLookup['S'] = KEY_S;
    State->KeyCodeLookup['T'] = KEY_T;
    State->KeyCodeLookup['U'] = KEY_U;
    State->KeyCodeLookup['V'] = KEY_V;
    State->KeyCodeLookup['W'] = KEY_W;
    State->KeyCodeLookup['X'] = KEY_X;
    State->KeyCodeLookup['Y'] = KEY_Y;
    State->KeyCodeLookup['Z'] = KEY_Z;
    State->KeyCodeLookup['0'] = KEY_0;
    State->KeyCodeLookup['1'] = KEY_1;
    State->KeyCodeLookup['2'] = KEY_2;
    State->KeyCodeLookup['3'] = KEY_3;
    State->KeyCodeLookup['4'] = KEY_4;
    State->KeyCodeLookup['5'] = KEY_5;
    State->KeyCodeLookup['6'] = KEY_6;
    State->KeyCodeLookup['7'] = KEY_7;
    State->KeyCodeLookup['8'] = KEY_8;
    State->KeyCodeLookup['9'] = KEY_9;
    
    State->KeyCodeLookup[VK_SPACE]      = KEY_SPACE,
    State->KeyCodeLookup[VK_OEM_3]      = KEY_TICK,
    State->KeyCodeLookup[VK_OEM_MINUS]  = KEY_MINUS,
    
    State->KeyCodeLookup[VK_OEM_PLUS]   = KEY_EQUAL,
    State->KeyCodeLookup[VK_OEM_4]      = KEY_LEFT_BRACKET,
    State->KeyCodeLookup[VK_OEM_6]      = KEY_RIGHT_BRACKET,
    State->KeyCodeLookup[VK_OEM_1]      = KEY_SEMICOLON,
    State->KeyCodeLookup[VK_OEM_7]      = KEY_QUOTE,
    State->KeyCodeLookup[VK_OEM_COMMA]  = KEY_COMMA,
    State->KeyCodeLookup[VK_OEM_PERIOD] = KEY_PERIOD,
    State->KeyCodeLookup[VK_OEM_2]      = KEY_FORWARD_SLASH,
    State->KeyCodeLookup[VK_OEM_5]      = KEY_BACKWARD_SLASH,
    State->KeyCodeLookup[VK_TAB]        = KEY_TAB,
    State->KeyCodeLookup[VK_ESCAPE]     = KEY_ESCAPE,
    State->KeyCodeLookup[VK_PAUSE]      = KEY_PAUSE,
    State->KeyCodeLookup[VK_UP]         = KEY_UP,
    State->KeyCodeLookup[VK_DOWN]       = KEY_DOWN,
    State->KeyCodeLookup[VK_LEFT]       = KEY_LEFT,
    State->KeyCodeLookup[VK_RIGHT]      = KEY_RIGHT,
    State->KeyCodeLookup[VK_BACK]       = KEY_BACKSPACE,
    State->KeyCodeLookup[VK_RETURN]     = KEY_RETURN,
    State->KeyCodeLookup[VK_DELETE]     = KEY_DELETE,
    State->KeyCodeLookup[VK_INSERT]     = KEY_INSERT,
    State->KeyCodeLookup[VK_HOME]       = KEY_HOME,
    State->KeyCodeLookup[VK_END]        = KEY_END,
    State->KeyCodeLookup[VK_PRIOR]      = KEY_PAGE_UP,
    State->KeyCodeLookup[VK_NEXT]       = KEY_PAGE_DOWN,
    State->KeyCodeLookup[VK_CAPITAL]    = KEY_CAPS_LOCK,
    State->KeyCodeLookup[VK_NUMLOCK]    = KEY_NUM_LOCK,
    State->KeyCodeLookup[VK_SCROLL]     = KEY_SCROLL_LOCK,
    State->KeyCodeLookup[VK_APPS]       = KEY_MENU,
    
    State->KeyCodeLookup[VK_SHIFT]      = KEY_SHIFT,
    State->KeyCodeLookup[VK_LSHIFT]     = KEY_SHIFT,
    State->KeyCodeLookup[VK_RSHIFT]     = KEY_SHIFT,
    
    State->KeyCodeLookup[VK_CONTROL]    = KEY_CONTROL,
    State->KeyCodeLookup[VK_LCONTROL]   = KEY_CONTROL,
    State->KeyCodeLookup[VK_RCONTROL]   = KEY_CONTROL,
    
    State->KeyCodeLookup[VK_MENU]       = KEY_ALT,
    State->KeyCodeLookup[VK_LMENU]      = KEY_ALT,
    State->KeyCodeLookup[VK_RMENU]      = KEY_ALT,
    
    State->KeyCodeLookup[VK_F1]  = KEY_F1;
    State->KeyCodeLookup[VK_F2]  = KEY_F2;
    State->KeyCodeLookup[VK_F3]  = KEY_F3;
    State->KeyCodeLookup[VK_F4]  = KEY_F4;
    State->KeyCodeLookup[VK_F5]  = KEY_F5;
    State->KeyCodeLookup[VK_F6]  = KEY_F6;
    State->KeyCodeLookup[VK_F7]  = KEY_F7;
    State->KeyCodeLookup[VK_F8]  = KEY_F8;
    State->KeyCodeLookup[VK_F9]  = KEY_F9;
    State->KeyCodeLookup[VK_F10] = KEY_F10;
    State->KeyCodeLookup[VK_F11] = KEY_F11;
    State->KeyCodeLookup[VK_F12] = KEY_F12;
    
    State->KeyCodeLookup[VK_NUMPAD0] = KEY_NUMPAD_0;
    State->KeyCodeLookup[VK_NUMPAD1] = KEY_NUMPAD_1;
    State->KeyCodeLookup[VK_NUMPAD2] = KEY_NUMPAD_2;
    State->KeyCodeLookup[VK_NUMPAD3] = KEY_NUMPAD_3;
    State->KeyCodeLookup[VK_NUMPAD4] = KEY_NUMPAD_4;
    State->KeyCodeLookup[VK_NUMPAD5] = KEY_NUMPAD_5;
    State->KeyCodeLookup[VK_NUMPAD6] = KEY_NUMPAD_6;
    State->KeyCodeLookup[VK_NUMPAD7] = KEY_NUMPAD_7;
    State->KeyCodeLookup[VK_NUMPAD8] = KEY_NUMPAD_8;
    State->KeyCodeLookup[VK_NUMPAD9] = KEY_NUMPAD_9;
}

internal inline Keymapping
AddKeyBinding(KeyCodeID MainKey, KeyCodeID AltKey)
{
    Keymapping Result = {};
    Result.MainKey = MainKey;
    Result.AltKey = AltKey;
    return(Result);
}

internal void
Win32LoadDefaultBindings(Input *GameInput)
{
    GameInput->Keyboard.Bindings[MOVE_UP]         = AddKeyBinding(KEY_W, KEY_UP);
    GameInput->Keyboard.Bindings[MOVE_DOWN]       = AddKeyBinding(KEY_S, KEY_DOWN);
    GameInput->Keyboard.Bindings[MOVE_LEFT]       = AddKeyBinding(KEY_A, KEY_LEFT);
    GameInput->Keyboard.Bindings[MOVE_RIGHT]      = AddKeyBinding(KEY_D, KEY_RIGHT);
    GameInput->Keyboard.Bindings[ATTACK]          = AddKeyBinding(KEY_LEFT_MOUSE, KEY_SPACE);
    GameInput->Keyboard.Bindings[INTERACT]        = AddKeyBinding(KEY_F, KEY_F);
    GameInput->Keyboard.Bindings[INVENTORY]       = AddKeyBinding(KEY_ESCAPE, KEY_ESCAPE);
    GameInput->Keyboard.Bindings[SHOW_HOTBAR]     = AddKeyBinding(KEY_TAB, KEY_TAB);
    GameInput->Keyboard.Bindings[HOTBAR_01]       = AddKeyBinding(KEY_1, KEY_NUMPAD_1);
    GameInput->Keyboard.Bindings[HOTBAR_02]       = AddKeyBinding(KEY_2, KEY_NUMPAD_2);
    GameInput->Keyboard.Bindings[HOTBAR_03]       = AddKeyBinding(KEY_3, KEY_NUMPAD_3);
    GameInput->Keyboard.Bindings[HOTBAR_04]       = AddKeyBinding(KEY_4, KEY_NUMPAD_4);
    GameInput->Keyboard.Bindings[HOTBAR_05]       = AddKeyBinding(KEY_5, KEY_NUMPAD_5);
    GameInput->Keyboard.Bindings[HOTBAR_06]       = AddKeyBinding(KEY_6, KEY_NUMPAD_6);
    GameInput->Keyboard.Bindings[HOTBAR_07]       = AddKeyBinding(KEY_7, KEY_NUMPAD_7);
    GameInput->Keyboard.Bindings[DROP_HELD]       = AddKeyBinding(KEY_Q, KEY_Q);
    GameInput->Keyboard.Bindings[DROP_ITEM]       = AddKeyBinding(KEY_RIGHT_MOUSE, KEY_RIGHT_MOUSE);
    GameInput->Keyboard.Bindings[CRAFTING_MENU]   = AddKeyBinding(KEY_E, KEY_E);
}
