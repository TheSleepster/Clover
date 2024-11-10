/* ========================================================================
   $File: Win32_Clover.cpp $
   $Date: August 27 2024 10:26 am $
   $Revision: $
   $Creator: Justin Lewis $
   ======================================================================== */

// NOTE(Sleepster): Freetype must come first due to the #define internal static inside of the intrinsics header
#include "../data/deps/Freetype/include/ft2build.h"
#include FT_FREETYPE_H

#if 0
#include "../data/deps/SDL3/include/SDL3/SDL.h"
#include "../data/deps/SDL3/include/SDL3/SDL_gamepad.h"
#include "../data/deps/SDL3/include/SDL3/SDL_joystick.h"
#endif

// INTRINSICS
#include "Intrinsics.h"

// UTILS
#include "util/Math.h"
#include "util/Array.h"
#include "util/FileIO.h"
#include "util/Arena.h"
#include "util/String.h"

// GLAD
#define GLAD_OPENGL_IMPL
#include "../data/deps/OpenGL/glad/include/glad/glad.h"

// IMGUI IMPl
#include "../data/deps/ImGui/imgui.h"
#include "../data/deps/ImGui/imgui_impl_win32.h"
#include "../data/deps/ImGUI/imgui_impl_opengl3.h"

// STB IMAGE TEXTURE LOADING
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../data/deps/stb/stb_image.h"

// WINDOWS
#pragma warning(disable:4005)
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <dbt.h>

// DSOUND
#include <mmreg.h>
#include <dsound.h>

// XINPUT

#include <xinput.h>
#pragma warning(default:4005)

// OPENGL HEADERS
#include "../data/deps/OpenGL/glext.h"
#include "../data/deps/OpenGL/wglext.h"
#include "../data/deps/OpenGL/glcorearb.h"

// CLOVER HEADERS
#include "Clover_Globals.h"
#include "Clover.h"
#include "Clover_Renderer.h"
#include "Clover_Input.h"
#include "Win32_Clover.h"
#include "Clover_Audio.h"

// FILES FOR UNITY BUILD
#include "Clover_Renderer.cpp"
#include "Clover_Input.cpp"
#include "Clover_Audio.cpp"


// NOTE(Sleepster): ImGui WNDPROC. It uses this for input
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);


internal inline real32
GetFPSTime(void)
{
    return((1000 *(real32)DeltaCounter) / real32(PerfCountFrequency));
}

internal inline real64
GetLastTime(void)
{
    return(real64(DeltaCounter) / real64(PerfCountFrequency));
}

internal inline real64
GetCurrentTimeInSeconds(void)
{
    LARGE_INTEGER Counter;
    QueryPerformanceCounter(&Counter);
    
    return((real64)Counter.QuadPart / PerfCountFrequency);
}

internal inline FILETIME
Win32MaxFiletime(FILETIME A, FILETIME B)
{
    if(CompareFileTime(&A, &B) != 0)
    {
        return(A);
    }
    return(B);
}

internal FILETIME
Win32GetLastWriteTime(string Filename)
{
    FILETIME LastWriteTime = {};
    
    WIN32_FIND_DATA FindData;
    HANDLE FindHandle = FindFirstFileA((const char *)Filename.Data, &FindData);
    if(FindHandle != INVALID_HANDLE_VALUE)
    {
        LastWriteTime = FindData.ftLastWriteTime;
        FindClose(FindHandle);
    }
    
    return(LastWriteTime);
}

internal void 
Win32LoadKeyData(game_state *State) 
{ 
    State->GameInput.KeyCodeLookup[VK_LBUTTON] = KEY_LEFT_MOUSE;
    State->GameInput.KeyCodeLookup[VK_MBUTTON] = KEY_MIDDLE_MOUSE;
    State->GameInput.KeyCodeLookup[VK_RBUTTON] = KEY_RIGHT_MOUSE;
    
    State->GameInput.KeyCodeLookup['A'] = KEY_A;
    State->GameInput.KeyCodeLookup['B'] = KEY_B;
    State->GameInput.KeyCodeLookup['C'] = KEY_C;
    State->GameInput.KeyCodeLookup['D'] = KEY_D;
    State->GameInput.KeyCodeLookup['E'] = KEY_E;
    State->GameInput.KeyCodeLookup['F'] = KEY_F;
    State->GameInput.KeyCodeLookup['G'] = KEY_G;
    State->GameInput.KeyCodeLookup['H'] = KEY_H;
    State->GameInput.KeyCodeLookup['I'] = KEY_I;
    State->GameInput.KeyCodeLookup['J'] = KEY_J;
    State->GameInput.KeyCodeLookup['K'] = KEY_K;
    State->GameInput.KeyCodeLookup['L'] = KEY_L;
    State->GameInput.KeyCodeLookup['M'] = KEY_M;
    State->GameInput.KeyCodeLookup['N'] = KEY_N;
    State->GameInput.KeyCodeLookup['O'] = KEY_O;
    State->GameInput.KeyCodeLookup['P'] = KEY_P;
    State->GameInput.KeyCodeLookup['Q'] = KEY_Q;
    State->GameInput.KeyCodeLookup['R'] = KEY_R;
    State->GameInput.KeyCodeLookup['S'] = KEY_S;
    State->GameInput.KeyCodeLookup['T'] = KEY_T;
    State->GameInput.KeyCodeLookup['U'] = KEY_U;
    State->GameInput.KeyCodeLookup['V'] = KEY_V;
    State->GameInput.KeyCodeLookup['W'] = KEY_W;
    State->GameInput.KeyCodeLookup['X'] = KEY_X;
    State->GameInput.KeyCodeLookup['Y'] = KEY_Y;
    State->GameInput.KeyCodeLookup['Z'] = KEY_Z;
    State->GameInput.KeyCodeLookup['0'] = KEY_0;
    State->GameInput.KeyCodeLookup['1'] = KEY_1;
    State->GameInput.KeyCodeLookup['2'] = KEY_2;
    State->GameInput.KeyCodeLookup['3'] = KEY_3;
    State->GameInput.KeyCodeLookup['4'] = KEY_4;
    State->GameInput.KeyCodeLookup['5'] = KEY_5;
    State->GameInput.KeyCodeLookup['6'] = KEY_6;
    State->GameInput.KeyCodeLookup['7'] = KEY_7;
    State->GameInput.KeyCodeLookup['8'] = KEY_8;
    State->GameInput.KeyCodeLookup['9'] = KEY_9;
    
    State->GameInput.KeyCodeLookup[VK_SPACE]      = KEY_SPACE,
    State->GameInput.KeyCodeLookup[VK_OEM_3]      = KEY_TICK,
    State->GameInput.KeyCodeLookup[VK_OEM_MINUS]  = KEY_MINUS,
    
    State->GameInput.KeyCodeLookup[VK_OEM_PLUS]   = KEY_EQUAL,
    State->GameInput.KeyCodeLookup[VK_OEM_4]      = KEY_LEFT_BRACKET,
    State->GameInput.KeyCodeLookup[VK_OEM_6]      = KEY_RIGHT_BRACKET,
    State->GameInput.KeyCodeLookup[VK_OEM_1]      = KEY_SEMICOLON,
    State->GameInput.KeyCodeLookup[VK_OEM_7]      = KEY_QUOTE,
    State->GameInput.KeyCodeLookup[VK_OEM_COMMA]  = KEY_COMMA,
    State->GameInput.KeyCodeLookup[VK_OEM_PERIOD] = KEY_PERIOD,
    State->GameInput.KeyCodeLookup[VK_OEM_2]      = KEY_FORWARD_SLASH,
    State->GameInput.KeyCodeLookup[VK_OEM_5]      = KEY_BACKWARD_SLASH,
    State->GameInput.KeyCodeLookup[VK_TAB]        = KEY_TAB,
    State->GameInput.KeyCodeLookup[VK_ESCAPE]     = KEY_ESCAPE,
    State->GameInput.KeyCodeLookup[VK_PAUSE]      = KEY_PAUSE,
    State->GameInput.KeyCodeLookup[VK_UP]         = KEY_UP,
    State->GameInput.KeyCodeLookup[VK_DOWN]       = KEY_DOWN,
    State->GameInput.KeyCodeLookup[VK_LEFT]       = KEY_LEFT,
    State->GameInput.KeyCodeLookup[VK_RIGHT]      = KEY_RIGHT,
    State->GameInput.KeyCodeLookup[VK_BACK]       = KEY_BACKSPACE,
    State->GameInput.KeyCodeLookup[VK_RETURN]     = KEY_RETURN,
    State->GameInput.KeyCodeLookup[VK_DELETE]     = KEY_DELETE,
    State->GameInput.KeyCodeLookup[VK_INSERT]     = KEY_INSERT,
    State->GameInput.KeyCodeLookup[VK_HOME]       = KEY_HOME,
    State->GameInput.KeyCodeLookup[VK_END]        = KEY_END,
    State->GameInput.KeyCodeLookup[VK_PRIOR]      = KEY_PAGE_UP,
    State->GameInput.KeyCodeLookup[VK_NEXT]       = KEY_PAGE_DOWN,
    State->GameInput.KeyCodeLookup[VK_CAPITAL]    = KEY_CAPS_LOCK,
    State->GameInput.KeyCodeLookup[VK_NUMLOCK]    = KEY_NUM_LOCK,
    State->GameInput.KeyCodeLookup[VK_SCROLL]     = KEY_SCROLL_LOCK,
    State->GameInput.KeyCodeLookup[VK_APPS]       = KEY_MENU,
    
    State->GameInput.KeyCodeLookup[VK_SHIFT]      = KEY_SHIFT,
    State->GameInput.KeyCodeLookup[VK_LSHIFT]     = KEY_SHIFT,
    State->GameInput.KeyCodeLookup[VK_RSHIFT]     = KEY_SHIFT,
    
    State->GameInput.KeyCodeLookup[VK_CONTROL]    = KEY_CONTROL,
    State->GameInput.KeyCodeLookup[VK_LCONTROL]   = KEY_CONTROL,
    State->GameInput.KeyCodeLookup[VK_RCONTROL]   = KEY_CONTROL,
    
    State->GameInput.KeyCodeLookup[VK_MENU]       = KEY_ALT,
    State->GameInput.KeyCodeLookup[VK_LMENU]      = KEY_ALT,
    State->GameInput.KeyCodeLookup[VK_RMENU]      = KEY_ALT,
    
    State->GameInput.KeyCodeLookup[VK_F1]  = KEY_F1;
    State->GameInput.KeyCodeLookup[VK_F2]  = KEY_F2;
    State->GameInput.KeyCodeLookup[VK_F3]  = KEY_F3;
    State->GameInput.KeyCodeLookup[VK_F4]  = KEY_F4;
    State->GameInput.KeyCodeLookup[VK_F5]  = KEY_F5;
    State->GameInput.KeyCodeLookup[VK_F6]  = KEY_F6;
    State->GameInput.KeyCodeLookup[VK_F7]  = KEY_F7;
    State->GameInput.KeyCodeLookup[VK_F8]  = KEY_F8;
    State->GameInput.KeyCodeLookup[VK_F9]  = KEY_F9;
    State->GameInput.KeyCodeLookup[VK_F10] = KEY_F10;
    State->GameInput.KeyCodeLookup[VK_F11] = KEY_F11;
    State->GameInput.KeyCodeLookup[VK_F12] = KEY_F12;
    
    State->GameInput.KeyCodeLookup[VK_NUMPAD0] = KEY_NUMPAD_0;
    State->GameInput.KeyCodeLookup[VK_NUMPAD1] = KEY_NUMPAD_1;
    State->GameInput.KeyCodeLookup[VK_NUMPAD2] = KEY_NUMPAD_2;
    State->GameInput.KeyCodeLookup[VK_NUMPAD3] = KEY_NUMPAD_3;
    State->GameInput.KeyCodeLookup[VK_NUMPAD4] = KEY_NUMPAD_4;
    State->GameInput.KeyCodeLookup[VK_NUMPAD5] = KEY_NUMPAD_5;
    State->GameInput.KeyCodeLookup[VK_NUMPAD6] = KEY_NUMPAD_6;
    State->GameInput.KeyCodeLookup[VK_NUMPAD7] = KEY_NUMPAD_7;
    State->GameInput.KeyCodeLookup[VK_NUMPAD8] = KEY_NUMPAD_8;
    State->GameInput.KeyCodeLookup[VK_NUMPAD9] = KEY_NUMPAD_9;
}

internal void
Win32LoadDefaultBindings(input *GameInput)
{
    GameInput->Mappings[BINDING_NONE]    = AddGameMapping(KEY_NONE, KEY_NONE, NULL_BUTTON, NULL_BUTTON);
    GameInput->Mappings[MOVE_UP]         = AddGameMapping(KEY_W, KEY_UP, NULL_BUTTON, NULL_BUTTON);
    GameInput->Mappings[MOVE_DOWN]       = AddGameMapping(KEY_S, KEY_DOWN, NULL_BUTTON, NULL_BUTTON);
    GameInput->Mappings[MOVE_LEFT]       = AddGameMapping(KEY_A, KEY_LEFT, NULL_BUTTON, NULL_BUTTON);
    GameInput->Mappings[MOVE_RIGHT]      = AddGameMapping(KEY_D, KEY_RIGHT, NULL_BUTTON, NULL_BUTTON);
    GameInput->Mappings[ATTACK]          = AddGameMapping(KEY_LEFT_MOUSE, KEY_SPACE, B_BUTTON, NULL_BUTTON);
    GameInput->Mappings[INTERACT]        = AddGameMapping(KEY_F, KEY_F, Y_BUTTON, NULL_BUTTON);
    GameInput->Mappings[INVENTORY]       = AddGameMapping(KEY_ESCAPE, KEY_ESCAPE, NULL_BUTTON, NULL_BUTTON);
    GameInput->Mappings[SHOW_HOTBAR]     = AddGameMapping(KEY_TAB, KEY_TAB, NULL_BUTTON, NULL_BUTTON);
    GameInput->Mappings[HOTBAR_01]       = AddGameMapping(KEY_1, KEY_NUMPAD_1, NULL_BUTTON, NULL_BUTTON);
    GameInput->Mappings[HOTBAR_02]       = AddGameMapping(KEY_2, KEY_NUMPAD_2, NULL_BUTTON, NULL_BUTTON);
    GameInput->Mappings[HOTBAR_03]       = AddGameMapping(KEY_3, KEY_NUMPAD_3, NULL_BUTTON, NULL_BUTTON);
    GameInput->Mappings[HOTBAR_04]       = AddGameMapping(KEY_4, KEY_NUMPAD_4, NULL_BUTTON, NULL_BUTTON);
    GameInput->Mappings[HOTBAR_05]       = AddGameMapping(KEY_5, KEY_NUMPAD_5, NULL_BUTTON, NULL_BUTTON);
    GameInput->Mappings[HOTBAR_06]       = AddGameMapping(KEY_6, KEY_NUMPAD_6, NULL_BUTTON, NULL_BUTTON);
    GameInput->Mappings[HOTBAR_07]       = AddGameMapping(KEY_7, KEY_NUMPAD_7, NULL_BUTTON, NULL_BUTTON);
    GameInput->Mappings[DROP_HELD]       = AddGameMapping(KEY_Q, KEY_Q, X_BUTTON, NULL_BUTTON);
    GameInput->Mappings[DROP_ITEM]       = AddGameMapping(KEY_RIGHT_MOUSE, KEY_RIGHT_MOUSE, NULL_BUTTON, NULL_BUTTON);
    GameInput->Mappings[CRAFTING]        = AddGameMapping(KEY_E, KEY_E, NULL_BUTTON, NULL_BUTTON);
    GameInput->Mappings[BUILD_MENU]      = AddGameMapping(KEY_B, KEY_B, NULL_BUTTON, NULL_BUTTON);
}

LRESULT CALLBACK
Win32MainWindowCallback(HWND WindowHandle, UINT Message,
                        WPARAM wParam, LPARAM lParam)
{
    LRESULT Result = {};
    switch(Message)
    {
        case WM_SIZE:
        {
            RECT Rect = {};
            GetClientRect(WindowHandle, &Rect);
            SizeData.Width  = int32(Rect.right - Rect.left);
            SizeData.Height = int32(Rect.bottom - Rect.top);
        }break;
        
        case WM_CLOSE:
        {
            Running = false;
            DestroyWindow(WindowHandle);
            PostQuitMessage(0);
            return(0);
        };
        case WM_DESTROY:
        {
            DestroyWindow(WindowHandle);
            PostQuitMessage(0);
            return(0);
        }break;

        case WM_DEVICECHANGE:
        {
        }break;
        default:
        {
            Result = DefWindowProc(WindowHandle, Message, wParam, lParam);
        };
    }
    return(Result);
}

internal void
Win32ProcessInputMessages(MSG Message, HWND WindowHandle, game_state *State)
{
    ImGuiIO &io = ImGui::GetIO();
    
    // NOTE(Sleepster): Makes sure to clear the mouse inputs. Why? Windows is dumb
    for(int32 KeycodeIndex = 0;
        KeycodeIndex <= KEY_COUNT;
        ++KeycodeIndex)
    {
        State->GameInput.Keyboard.Keys[KeycodeIndex].HalfTransitionCount = 0;
    }

    for(int32 ButtonIndex = 0;
        ButtonIndex < GAMEPAD_BUTTON_COUNT;
        ++ButtonIndex)
    {
        State->GameInput.Controller.GamepadButtons[ButtonIndex].HalfTransitionCount = 0;
    }
    
    while(PeekMessageA(&Message, WindowHandle, 0, 0, PM_REMOVE))
    {
        // NOTE(Sleepster): ImGui Is a little goofy ahhhhh if you don't give it complete processing of inputs 
        if(!io.WantCaptureMouse && !io.WantCaptureKeyboard)
        {
            switch(Message.message)
            {
                case WM_SYSKEYDOWN:
                case WM_SYSKEYUP:
                case WM_KEYDOWN:
                case WM_KEYUP:
                {
                    uint32 VKCode = (uint32)Message.wParam;
                    bool8 IsDown  = ((Message.lParam & (1 << 31)) == 0);
                    
                    keycodeID KeyCode  = State->GameInput.KeyCodeLookup[Message.wParam];
                    keyboard_key *Key = &State->GameInput.Keyboard.Keys[KeyCode];
                    Key->JustPressed   = !Key->JustPressed && !Key->IsDown && IsDown;
                    Key->JustReleased  = !Key->JustReleased && Key->IsDown && !IsDown;
                    Key->IsDown        = IsDown;
                    Key->HalfTransitionCount++;
                    
                    bool8 AltKeyIsDown = ((Message.lParam & (1 << 29)) != 0);
                    if(VKCode == VK_F4 && AltKeyIsDown)
                    {
                        Running = false;
                    }
                }break;
                
                case WM_LBUTTONUP:
                case WM_RBUTTONUP:
                case WM_MBUTTONUP:
                case WM_XBUTTONUP:
                case WM_LBUTTONDOWN:
                case WM_RBUTTONDOWN:
                case WM_MBUTTONDOWN:
                case WM_XBUTTONDOWN:
                {
                    // NOTE(Sleepster): This is the longest fucking "if" check ever holy moly. Unfortunately Windows is dumb, 
                    //                  so checking the Message's wParam doesn't always work right. Hence this crime of ternary
                    bool8 IsDown = 
                    (Message.message == WM_LBUTTONDOWN) || (Message.message == WM_RBUTTONDOWN)|| 
                    (Message.message == WM_MBUTTONDOWN) || (Message.message == WM_XBUTTONDOWN);
                    
                    int32 MouseCode = 
                    (Message.message == WM_LBUTTONDOWN || Message.message == WM_LBUTTONUP) ? VK_LBUTTON:
                    (Message.message == WM_RBUTTONDOWN || Message.message == WM_RBUTTONUP) ? VK_RBUTTON:
                    (Message.message == WM_MBUTTONDOWN || Message.message == WM_MBUTTONUP) ? VK_MBUTTON:
                    (Message.message == WM_XBUTTONDOWN || Message.message == WM_XBUTTONUP) ? 
                    (GET_XBUTTON_WPARAM(Message.wParam) == XBUTTON1 ? VK_XBUTTON1 : VK_XBUTTON2) : 0;
                    
                    keycodeID KeyCode  = State->GameInput.KeyCodeLookup[MouseCode];
                    keyboard_key *Key = &State->GameInput.Keyboard.Keys[KeyCode];
                    Key->JustPressed   = !Key->JustPressed && !Key->IsDown && IsDown;
                    Key->JustReleased  = !Key->JustReleased && Key->IsDown && !IsDown;
                    Key->IsDown        = IsDown;
                    Key->HalfTransitionCount++;
                }break;

                case WM_MOUSEMOVE:
                {
                    POINT MousePoint;
                    GetCursorPos(&MousePoint);
                    ScreenToClient(WindowHandle, &MousePoint);
                    
                    State->GameInput.Keyboard.LastMouse    = State->GameInput.Keyboard.CurrentMouse;
                    State->GameInput.Keyboard.CurrentMouse = ivec2{MousePoint.x, MousePoint.y};
                    State->GameInput.Keyboard.DeltaMouse   = State->GameInput.Keyboard.CurrentMouse - State->GameInput.Keyboard.LastMouse;
                }break;

                default:
                {
                    TranslateMessage(&Message);
                    DispatchMessage(&Message);
                }break;
            }
        }
        
        if(ImGui_ImplWin32_WndProcHandler(WindowHandle, Message.message, Message.wParam, Message.lParam) != 0) return;
        
        TranslateMessage(&Message);
        DispatchMessage(&Message);
    }
}

internal void
Win32LoadWGLFunctions(WNDCLASS Window, HINSTANCE hInstance, wgl_function_pointers *WGLFunctions)
{
    HWND DummyWindow = 
        CreateWindow(Window.lpszClassName,
                     "OpenGLFunctionGetter",
                     WS_OVERLAPPEDWINDOW,
                     CW_USEDEFAULT,
                     CW_USEDEFAULT,
                     CW_USEDEFAULT,
                     CW_USEDEFAULT,
                     0,
                     0,
                     hInstance,
                     0);
    HDC DummyContext = GetDC(DummyWindow);
    
    PIXELFORMATDESCRIPTOR DFormat = {};
    DFormat.nSize = sizeof(DFormat);
    DFormat.nVersion = 1;
    DFormat.dwFlags = PFD_DRAW_TO_WINDOW|PFD_SUPPORT_OPENGL|PFD_DOUBLEBUFFER;
    DFormat.iPixelType = PFD_TYPE_RGBA;
    DFormat.cColorBits = 32;
    DFormat.cAlphaBits = 8;
    DFormat.cDepthBits = 24;
    
    int32 dPixelFormat = ChoosePixelFormat(DummyContext, &DFormat);
    PIXELFORMATDESCRIPTOR DSuggestedFormat;
    DescribePixelFormat(DummyContext, dPixelFormat, sizeof(DSuggestedFormat), &DSuggestedFormat);
    SetPixelFormat(DummyContext, dPixelFormat,  &DSuggestedFormat);
    
    HGLRC TempRC = wglCreateContext(DummyContext);
    wglMakeCurrent(DummyContext, TempRC);
    
    WGLFunctions->wglChoosePixelFormatARB = (PFNWGLCHOOSEPIXELFORMATARBPROC)
        wglGetProcAddress("wglChoosePixelFormatARB");
    WGLFunctions->wglCreateContextAttribsARB = (PFNWGLCREATECONTEXTATTRIBSARBPROC)
        wglGetProcAddress("wglCreateContextAttribsARB");
    WGLFunctions->wglSwapIntervalEXT = (PFNWGLSWAPINTERVALEXTPROC)
        wglGetProcAddress("wglSwapIntervalEXT");
    if(!WGLFunctions->wglChoosePixelFormatARB||
       !WGLFunctions->wglCreateContextAttribsARB||
       !WGLFunctions->wglSwapIntervalEXT)
    {
        Check(false, "Failed to extract OpenGL function Pointers!\n");
    }
    
    wglMakeCurrent(DummyContext, 0);
    wglDeleteContext(TempRC);
    ReleaseDC(DummyWindow, DummyContext);
    DestroyWindow(DummyWindow);
}

internal game_functions
Win32LoadGameCode(string SourceDLLName)
{
    game_functions Result = {};
    string TempDLLName = STR("GameTemp.dll");
    Result.LastWriteTime = Win32GetLastWriteTime(SourceDLLName);
    Result.IsLoaded      = 0;
    while(!Result.IsLoaded)
    {
        CopyFile((char *)SourceDLLName.Data, (char *)TempDLLName.Data, FALSE);
        Result.IsLoaded = 1;
    }
    
    Result.GameCodeDLL = LoadLibraryA((const char *)TempDLLName.Data);
    if(Result.GameCodeDLL)
    {
        Result.OnAwake         = (game_on_awake *)          GetProcAddress(Result.GameCodeDLL, "GameOnAwake");
        Result.FixedUpdate     = (game_fixed_update *)      GetProcAddress(Result.GameCodeDLL, "GameFixedUpdate");
        Result.UpdateAndDraw   = (game_update_and_draw *)   GetProcAddress(Result.GameCodeDLL, "GameUpdateAndDraw");
        Result.GetSoundSamples = (game_get_sound_samples *) GetProcAddress(Result.GameCodeDLL, "GameGetSoundSamples");
    }
    else
    {
        Result.OnAwake         = GameOnAwakeStub;
        Result.FixedUpdate     = GameFixedUpdateStub;
        Result.UpdateAndDraw   = GameUpdateAndDrawStub;
        Result.GetSoundSamples = GameGetSoundSamplesStub;
    }
    Sleep(200);
    return(Result);
}

internal void
Win32UnloadGameCode(game_functions *GameCode)
{
    if(GameCode->GameCodeDLL)
    {
        FreeLibrary(GameCode->GameCodeDLL);
        GameCode->GameCodeDLL = 0;
        GameCode->IsLoaded    = 0;
        GameCode->IsValid     = 0;
    }
    
    GameCode->OnAwake         = GameOnAwakeStub;
    GameCode->FixedUpdate     = GameFixedUpdateStub;
    GameCode->UpdateAndDraw   = GameUpdateAndDrawStub;
    GameCode->GetSoundSamples = GameGetSoundSamplesStub;
}

internal inline void
ClearTransientState(transient_state *TransientState)
{
    ClearArena(&TransientState->TransientArena);
}

internal win32_sound_data
Win32InitDSound(HWND WindowHandle, int32 SamplesPerSecond, int32 BufferSize)
{
    win32_sound_data Result = {};
    HRESULT DSoundError = {};
    
    HMODULE DSoundLib = LoadLibraryA("dsound.dll");
    if(!DSoundLib)
    {
        Check(0, "No Dsound :(\n");
    }
    
    direct_sound_create *DirectSoundCreate = (direct_sound_create *)GetProcAddress(DSoundLib, "DirectSoundCreate");
    DSoundError = DirectSoundCreate(0, &Result.DirectSoundObject, 0);
    if(SUCCEEDED(DSoundError))
    {
        Result.DirectSoundBufferFormat.wFormatTag      = WAVE_FORMAT_PCM;
        Result.DirectSoundBufferFormat.nChannels       = 2;
        Result.DirectSoundBufferFormat.nSamplesPerSec  = SamplesPerSecond;
        Result.DirectSoundBufferFormat.wBitsPerSample  = 16;
        Result.DirectSoundBufferFormat.nBlockAlign     = (Result.DirectSoundBufferFormat.nChannels * Result.DirectSoundBufferFormat.wBitsPerSample) / 8;
        Result.DirectSoundBufferFormat.nAvgBytesPerSec = Result.DirectSoundBufferFormat.nSamplesPerSec * Result.DirectSoundBufferFormat.nBlockAlign;
        Result.DirectSoundBufferFormat.cbSize = 0;
        
        DSoundError = Result.DirectSoundObject->SetCooperativeLevel(WindowHandle, DSSCL_PRIORITY);
        if(SUCCEEDED(DSoundError))
        {
            DSBUFFERDESC        DSBufferDesc = {};
            DSBufferDesc.dwSize  = sizeof(DSBUFFERDESC);
            DSBufferDesc.dwFlags = DSBCAPS_PRIMARYBUFFER;
            
            DSoundError = Result.DirectSoundObject->CreateSoundBuffer(&DSBufferDesc, &Result.PrimaryBuffer, 0);
            if(SUCCEEDED(DSoundError))
            {
                if(SUCCEEDED(Result.PrimaryBuffer->SetFormat(&Result.DirectSoundBufferFormat)))
                {
                    cl_Info("DSound Primary Buffer Created\n");
                }
                else
                {
                    cl_Error("Failure to set the primary buffer's format. Code: %lu", DSoundError);
                }
            }
            else
            {
                cl_Error("Failure to create the DSound primary sound buffer. Code: %lu", DSoundError);
            }
        }
        else
        {
            cl_Error("Failure to set Direct Sound's cooperative level. Code: %lu", DSoundError);
        }
        
        DSBUFFERDESC DSBufferDesc = {};
        DSBufferDesc.dwSize  = sizeof(DSBufferDesc);
        DSBufferDesc.dwFlags = 0;
        DSBufferDesc.dwBufferBytes = BufferSize;
        DSBufferDesc.lpwfxFormat = &Result.DirectSoundBufferFormat;
        
        DSoundError = Result.DirectSoundObject->CreateSoundBuffer(&DSBufferDesc, &Result.SecondaryBuffer, 0);
        if(SUCCEEDED(DSoundError))
        {
            cl_Info("DSound Secondary Buffer Created\n");
        }
        else
        {
            cl_Error("Failure to create the DSound secondary sound buffer. Code: %lu", DSoundError);
        }
    }
    else
    {
        cl_Error("Failure to get the DSound Object. Code: %lu", DSoundError);
    }
    
    return(Result);
}

internal void 
Win32ClearSoundBuffer(win32_sound_data *DSound, sound_output_data *SoundOutput)
{
    VOID *Region1 = 0;
    DWORD Region1Size = 0;
    VOID *Region2 = 0;
    DWORD Region2Size = 0;
    if(SUCCEEDED(DSound->SecondaryBuffer->Lock(0,
                                               SoundOutput->BufferSize,
                                               &Region1, &Region1Size,
                                               &Region2, &Region2Size,
                                               0)))
    {
        uint8 *DestSample = (uint8 *)Region1;
        for(DWORD ByteIndex = 0;
            ByteIndex < Region1Size;
            ++ByteIndex)
        {
            *DestSample++ = 0;
        }
        
        DestSample = (uint8 *)Region2;
        for(DWORD ByteIndex = 0;
            ByteIndex < Region2Size;
            ++ByteIndex)
        {
            *DestSample++ = 0;
        }
    }
    DSound->SecondaryBuffer->Unlock(Region1, Region1Size, Region2, Region2Size);
}

internal void 
Win32FillSoundBuffer(win32_sound_data *DSound, sound_output_data *SoundOutput, sound_buffer *SourceBuffer, 
                     int32 BytesToLock, int32 BytesToWrite)
{
    VOID *Region1 = 0;
    DWORD Region1Size = 0;
    VOID *Region2 = 0;
    DWORD Region2Size = 0;
    if(SUCCEEDED(DSound->SecondaryBuffer->Lock(BytesToLock,
                                               BytesToWrite,
                                               &Region1, &Region1Size,
                                               &Region2, &Region2Size,
                                               0)))
    {
        Assert(Region1Size % 2 == 0);
        Assert(Region2Size % 2 == 0);
        
        int16 *DestSample = (int16 *)Region1;
        int16 *SrcSample  = SourceBuffer->SampleBuffer;
        DWORD Region1SampleCount = Region1Size / SoundOutput->BytesPerSample; 
        for(DWORD SampleIndex = 0;
            SampleIndex < Region1SampleCount;
            ++SampleIndex)
        {
            *DestSample++ = *SrcSample++;
            *DestSample++ = *SrcSample++;
            
            SoundOutput->RunningSampleIndex++;
        }
        
        DestSample = (int16 *)Region2;
        DWORD Region2SampleCount = Region2Size / SoundOutput->BytesPerSample;
        for(DWORD SampleIndex = 0;
            SampleIndex < Region2SampleCount;
            ++SampleIndex)
        {
            *DestSample++ = *SrcSample++;
            *DestSample++ = *SrcSample++;
            
            SoundOutput->RunningSampleIndex++;
        }
    }
    DSound->SecondaryBuffer->Unlock(Region1, Region1Size, Region2, Region2Size);
}

internal void
Win32SetupXInput(game_state *State)
{
    HMODULE XInput = LoadLibraryA("xinput1_4.dll");
    if(XInput)
    {
        XInputGetState = (xinput_get_state *)GetProcAddress(XInput, "XInputGetState");
        XInputSetState = (xinput_set_state *)GetProcAddress(XInput, "XInputSetState");
        if(XInputGetState && XInputSetState)
        {
            //State->GameInput.ButtonLookup = ButtonLookup;
            State->GameInput.ButtonLookup[DPAD_UP]               = XINPUT_GAMEPAD_DPAD_UP;
            State->GameInput.ButtonLookup[DPAD_DOWN]             = XINPUT_GAMEPAD_DPAD_DOWN;
            State->GameInput.ButtonLookup[DPAD_LEFT]             = XINPUT_GAMEPAD_DPAD_LEFT;
            State->GameInput.ButtonLookup[DPAD_RIGHT]            = XINPUT_GAMEPAD_DPAD_RIGHT;
            State->GameInput.ButtonLookup[START_BUTTON]          = XINPUT_GAMEPAD_START;
            State->GameInput.ButtonLookup[BACK_BUTTON]           = XINPUT_GAMEPAD_BACK;
            State->GameInput.ButtonLookup[LEFT_THUMBSTICK_DOWN]  = XINPUT_GAMEPAD_LEFT_THUMB;
            State->GameInput.ButtonLookup[RIGHT_THUMBSTICK_DOWN] = XINPUT_GAMEPAD_RIGHT_THUMB;
            State->GameInput.ButtonLookup[LEFT_SHOULDER_BUTTON]  = XINPUT_GAMEPAD_LEFT_SHOULDER;
            State->GameInput.ButtonLookup[RIGHT_SHOULDER_BUTTON] = XINPUT_GAMEPAD_RIGHT_SHOULDER;
            State->GameInput.ButtonLookup[A_BUTTON]              = XINPUT_GAMEPAD_A;
            State->GameInput.ButtonLookup[B_BUTTON]              = XINPUT_GAMEPAD_B;
            State->GameInput.ButtonLookup[X_BUTTON]              = XINPUT_GAMEPAD_X;
            State->GameInput.ButtonLookup[Y_BUTTON]              = XINPUT_GAMEPAD_Y;
        }
        else
        {
            cl_Info("Failure to get XInput\n");
            return;
        }
    }
    else
    {
        XInputGetState = XInputGetStateStub;
        XInputSetState = XInputSetStateStub;

        cl_Info("Failed to load XInput1.4, there will be no controller support.\n");
        return;
    }
}

int CALLBACK
WinMain(HINSTANCE hInstance,
        HINSTANCE hPrevInstance,
        LPSTR lpCmdLine,
        int32 nShowCmd)
{
    WNDCLASS              Window         = {};
    time_data             Time           = {};
    game_state            State          = {};
    transient_state       TransientState = {};
    game_memory           GameMemory     = {};
    game_functions        Game           = {};
    wgl_function_pointers WGLFunctions   = {};
    gl_render_data        RenderData     = {};
    
    // NOTE(Sleepster): Accumulator is for Delta Time
    real64 Accumulator = {};
    real32 FPSTimer = 0;
    
    LARGE_INTEGER PerfCountFrequencyResult;
    QueryPerformanceFrequency(&PerfCountFrequencyResult);
    PerfCountFrequency = PerfCountFrequencyResult.QuadPart;
    
    Window.style = CS_OWNDC|CS_VREDRAW|CS_HREDRAW;
    Window.lpfnWndProc = Win32MainWindowCallback;
    Window.hInstance = hInstance;
    Window.lpszClassName = "MakeshiftWindow";
    
    SizeData = {100, 100, 1920, 1080};
    if(RegisterClass(&Window))
    {
        RECT rect = {0, 0, SizeData.Width, SizeData.Height};
        AdjustWindowRectEx(&rect, WS_OVERLAPPEDWINDOW, FALSE, WS_EX_CLIENTEDGE);
        SizeData.Width  = rect.right  - rect.left;
        SizeData.Height = rect.bottom - rect.top;
        
        Win32LoadWGLFunctions(Window, hInstance, &WGLFunctions);
        
        HWND WindowHandle =
            CreateWindowEx(WS_EX_CLIENTEDGE,
                           Window.lpszClassName,
                           "CloverEngine",
                           WS_OVERLAPPEDWINDOW|WS_VISIBLE|CS_OWNDC,
                           CW_USEDEFAULT,
                           CW_USEDEFAULT,
                           SizeData.Width,
                           SizeData.Height,
                           0,
                           0,
                           hInstance,
                           0);
        if(WindowHandle)
        {
            HDC WindowDC = GetDC(WindowHandle);
            
            GameMemory.PermanentStorage.BlockSize    = Megabytes(512);
            GameMemory.PermanentStorage.MemoryBlock  = VirtualAlloc(0, GameMemory.PermanentStorage.BlockSize, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
            GameMemory.PermanentStorage.BlockOffset  = (uint8 *)GameMemory.PermanentStorage.MemoryBlock;
            
            GameMemory.TransientStorage.BlockSize    = Megabytes(512);
            GameMemory.TransientStorage.MemoryBlock  = VirtualAlloc(0, GameMemory.TransientStorage.BlockSize, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
            GameMemory.TransientStorage.BlockOffset  = (uint8 *)GameMemory.TransientStorage.MemoryBlock;
            
            InitializeArena(&RenderData.VertexArena,        sizeof(vertex) * TRUE_MAX_VERTICES, &GameMemory.PermanentStorage);
            InitializeArena(&RenderData.UIVertexArena,      sizeof(vertex) * TRUE_MAX_VERTICES, &GameMemory.PermanentStorage);
            InitializeArena(&TransientState.TransientArena, Megabytes(200),                     &GameMemory.TransientStorage);
            
            RenderData.DrawFrame.Vertices                     = (vertex *)RenderData.VertexArena.Base;
            RenderData.DrawFrame.UIVertices                   = (vertex *)RenderData.UIVertexArena.Base;
            RenderData.DrawFrame.TransparentVertexBufferptr   = (vertex *)(RenderData.VertexArena.Base   + (RenderData.VertexArena.Capacity / 2));
            RenderData.DrawFrame.TransparentUIVertexBufferptr = (vertex *)(RenderData.UIVertexArena.Base + (RenderData.UIVertexArena.Capacity / 2));
            
            sound_output_data SoundOutput  = {};
            SoundOutput.ToneVolume         = 50;
            SoundOutput.ToneFreq           = 512;
            SoundOutput.SamplesPerSecond   = 48000;
            SoundOutput.WavePeriod         = SoundOutput.SamplesPerSecond / SoundOutput.ToneFreq;
            SoundOutput.BytesPerSample     = sizeof(int16) * 2; 
            SoundOutput.BufferSize         = SoundOutput.SamplesPerSecond * (sizeof(int16) * 2);
            SoundOutput.LatencyCursor      = SoundOutput.SamplesPerSecond / 15; 
            SoundOutput.RunningSampleIndex = 0;
            
            int16 *SampleBufferStorage = (int16 *)VirtualAlloc(0, SoundOutput.BufferSize, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
#if 0
            Memory.TransientStorage = ArenaCreate(Megabytes(512));
            Memory.PermanentStorage = ArenaCreate(Megabytes(512));
            
            RenderData.DrawFrame.Vertices = (vertex *)ArenaAlloc(&Memory.PermanentStorage, sizeof(vertex) * TRUE_MAX_VERTICES);
            RenderData.DrawFrame.UIVertices = (vertex *)ArenaAlloc(&Memory.PermanentStorage, sizeof(vertex) * TRUE_MAX_VERTICES);
#endif
            CloverResetRendererState(&RenderData);
            Win32LoadKeyData(&State);
            Win32LoadDefaultBindings(&State.GameInput);
            Win32SetupXInput(&State);
            
            const int32 PixelAttributes[] =
            {
                WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
                WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
                WGL_DOUBLE_BUFFER_ARB,  GL_TRUE,
                WGL_SWAP_METHOD_ARB,    WGL_SWAP_COPY_ARB,
                WGL_PIXEL_TYPE_ARB,     WGL_TYPE_RGBA_ARB,
                WGL_ACCELERATION_ARB,   WGL_FULL_ACCELERATION_ARB,
                WGL_COLOR_BITS_ARB,     32,
                WGL_ALPHA_BITS_ARB,     8,
                WGL_DEPTH_BITS_ARB,     24,
                0
            };
            
            const int32 ContextAttributes[] =
            {
                WGL_CONTEXT_MAJOR_VERSION_ARB, 4,
                WGL_CONTEXT_MINOR_VERSION_ARB, 3,
                WGL_CONTEXT_PROFILE_MASK_ARB,  WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
                WGL_CONTEXT_FLAGS_ARB,         WGL_CONTEXT_DEBUG_BIT_ARB,
                0
            };
            
            UINT NumPixelFormats;
            int32 PixelFormat = 0;
            if(!WGLFunctions.wglChoosePixelFormatARB(WindowDC, PixelAttributes, 0, 1, &PixelFormat, &NumPixelFormats))
            {
                Check(false, "Failed to choose the Main Pixel Format!\n");
            }
            
            PIXELFORMATDESCRIPTOR MainPixelFormat;
            DescribePixelFormat(WindowDC, PixelFormat, sizeof(PIXELFORMATDESCRIPTOR), &MainPixelFormat);
            SetPixelFormat(WindowDC, PixelFormat, &MainPixelFormat);
            
            HGLRC MainRenderingContext = WGLFunctions.wglCreateContextAttribsARB(WindowDC, 0, ContextAttributes);
            wglMakeCurrent(WindowDC, MainRenderingContext);
            gladLoadGL();
            
            
            // VSYNC
            WGLFunctions.wglSwapIntervalEXT(0);
            // VSYNC
            
            
            CloverSetupRenderer(&TransientState.TransientArena, &RenderData);
            RenderData.CloverRender = CloverRender;
            
            Game = Win32LoadGameCode(STR("CloverGame.dll"));
            
            // NOTE(Sleepster): ImGui Setup 
            IMGUI_CHECKVERSION();
            RenderData.CurrentImGuiContext = ImGui::CreateContext();
            ImGui::SetCurrentContext(RenderData.CurrentImGuiContext);
            
            ImGuiIO& io = ImGui::GetIO(); (void)io;
            io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;   // Enable Keyboard Controls
            io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;    // Enable Gamepad Controls
            
            io.WantCaptureKeyboard = 1;
            io.WantCaptureMouse = 1;
            io.DeltaTime = Time.Delta > 0 ? SIMRATE : Time.Delta;
            
            ImGui::StyleColorsDark();
            ImGui_ImplWin32_InitForOpenGL(WindowHandle);
            ImGui_ImplOpenGL3_Init();
            
            
            win32_sound_data DSound = Win32InitDSound(WindowHandle, SoundOutput.SamplesPerSecond, SoundOutput.BufferSize);
            Win32ClearSoundBuffer(&DSound, &SoundOutput);
            DSound.SecondaryBuffer->Play(0, 0, DSBPLAY_LOOPING);
            
            Game.OnAwake(&GameMemory, &RenderData, &State, &TransientState);
            
            LARGE_INTEGER LastCounter;
            QueryPerformanceCounter(&LastCounter);
            
            State.TestSound = CloverLoadWAVFile(&TransientState.TransientArena, STR("../data/res/sounds/Test2.wav")); 
            real64 CurrentTime = GetLastTime();
            
            Running = 1;
            while(Running)
            {
                MSG Message = {};
                Win32ProcessInputMessages(Message, WindowHandle, &State);
                State.GameInput.ButtonLookup[LEFT_THUMBSTICK_DOWN]  = XINPUT_GAMEPAD_LEFT_THUMB;

                for(uint32 ControllerIndex = 0;
                    ControllerIndex < 1;
                    ++ControllerIndex)
                {
                    XINPUT_STATE Controller;
                    if(XInputGetState(ControllerIndex, &Controller) == ERROR_SUCCESS)
                    {
                        XINPUT_GAMEPAD *Gamepad = &Controller.Gamepad;
                        for(uint32 ButtonIndex = 0;
                            ButtonIndex < GAMEPAD_BUTTON_COUNT;
                            ++ButtonIndex)
                        {
                            action_button *Button = &State.GameInput.Controller.GamepadButtons[ButtonIndex];
                            bool32 IsDown = (Gamepad->wButtons & ButtonLookup[ButtonIndex]);

                            Button->JustPressed  = !Button->JustPressed && !Button->IsDown &&  IsDown;
                            Button->JustReleased = !Button->JustPressed &&  Button->IsDown && !IsDown;
                            Button->IsDown       = IsDown; 

                            // TODO(Sleepster): We might need to seperate these 
                            if(IsDown || Button->JustReleased)
                            {
                                Button->HalfTransitionCount++;
                            }
                        }

                        State.GameInput.Controller.LeftStick  = {Gamepad->sThumbLX, Gamepad->sThumbLY};
                        State.GameInput.Controller.RightStick = {Gamepad->sThumbRX, Gamepad->sThumbRY};

                        State.GameInput.Controller.LeftTriggerValue = Gamepad->bLeftTrigger;
                        State.GameInput.Controller.RightTriggerValue = Gamepad->bRightTrigger;

                        XINPUT_VIBRATION Rumble = 
                        {
                            .wLeftMotorSpeed = State.GameInput.Controller.LeftRumble, 
                            .wRightMotorSpeed = State.GameInput.Controller.RightRumble
                        };
                        XInputSetState(ControllerIndex, &Rumble);

                        State.GameInput.Controller.LeftRumble = 0;
                        State.GameInput.Controller.RightRumble = 0;
                    }
                }

                //DATA RELOADING
#if CLOVER_SLOW
                FILETIME NewDLLWriteTime = Win32GetLastWriteTime(STR("CloverGame.dll"));
                if(CompareFileTime(&Game.LastWriteTime, &NewDLLWriteTime) != 0)
                {
                    Win32UnloadGameCode(&Game);
                    Game = Win32LoadGameCode(STR("CloverGame.dll"));
                    
                    // NOTE(Sleepster): Audio Engine setup, MiniAudio makes this REALLLLLLYYYYYYYY easy 
                    Time.CurrentTimeInSeconds = 0.0f;
                    Game.OnAwake(&GameMemory, &RenderData, &State, &TransientState);
                }
                
                // NOTE(Sleepster: Shader Reloading  
                filetime NewTextureWriteTime        = FileGetLastWriteTime(RenderData.GameAtlas.Filepath);    
                filetime NewVertexShaderWriteTime   = FileGetLastWriteTime(RenderData.BasicShader.VertexShader.Filepath);
                filetime NewFragmentShaderWriteTime = FileGetLastWriteTime(RenderData.BasicShader.FragmentShader.Filepath);
                
                if(!CloverCompareFiletime(NewTextureWriteTime, RenderData.GameAtlas.LastWriteTime))
                {
                    CloverReloadTexture(&RenderData, &RenderData.GameAtlas, 0);
                    Sleep(100);
                }
                
                if(!CloverCompareFiletime(NewVertexShaderWriteTime,   RenderData.BasicShader.VertexShader.LastWriteTime) ||
                   !CloverCompareFiletime(NewFragmentShaderWriteTime, RenderData.BasicShader.FragmentShader.LastWriteTime))
                {
                    RebuildShader(&TransientState.TransientArena, &RenderData.BasicShader);
                }
                
                if(!CloverCompareFiletime(NewVertexShaderWriteTime,   RenderData.gBufferShader.VertexShader.LastWriteTime) ||
                   !CloverCompareFiletime(NewFragmentShaderWriteTime, RenderData.gBufferShader.FragmentShader.LastWriteTime))
                {
                    RebuildShader(&TransientState.TransientArena, &RenderData.gBufferShader); 
                }
                
                if(!CloverCompareFiletime(NewVertexShaderWriteTime,   RenderData.LightingShader.VertexShader.LastWriteTime) ||
                   !CloverCompareFiletime(NewFragmentShaderWriteTime, RenderData.LightingShader.FragmentShader.LastWriteTime))
                {
                    RebuildShader(&TransientState.TransientArena, &RenderData.LightingShader); 
                }
#endif
                
                real64 NewTime     = GetLastTime();
                CurrentTime        = NewTime;
                
                Time.Delta = (real32)GetLastTime();
                Time.Current = (real32)CurrentTime;
                while(Accumulator >= SIMRATE)
                {
                    Game.FixedUpdate(&GameMemory, &RenderData, &State, &TransientState, Time);
                    Accumulator -= Time.Delta;
                    Time.CurrentTimeInSeconds = real32(GetCurrentTimeInSeconds());
                }
                
                Accumulator += Time.Delta;
                
                glViewport(0, 0, SizeData.Width, SizeData.Height);
                glClearColor(RenderData.ClearColor.R, RenderData.ClearColor.G, RenderData.ClearColor.B, RenderData.ClearColor.A);
                glClearDepth(0.0f);
                glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
                
                // Start the Dear ImGui frame
                ImGui_ImplOpenGL3_NewFrame();
                ImGui_ImplWin32_NewFrame();
                ImGui::NewFrame();
                
                RenderData.AspectRatio = (real32)SizeData.Width / (real32)SizeData.Height;
                Game.UpdateAndDraw(&GameMemory, &RenderData, &State, &TransientState, Time, SizeData);
                
                DWORD BytesToWrite = 0;
                DWORD BytesToLock = 0;
                DWORD TargetCursor;
                DWORD PlayCursorPosition;
                DWORD WriteCursorPosition;
                bool SoundIsValid = false;
                if(SUCCEEDED(DSound.SecondaryBuffer->GetCurrentPosition(&PlayCursorPosition, &WriteCursorPosition)))
                {
                    BytesToWrite = 0;
                    BytesToLock  = (SoundOutput.RunningSampleIndex * SoundOutput.BytesPerSample) % SoundOutput.BufferSize;
                    TargetCursor = (PlayCursorPosition + (SoundOutput.LatencyCursor * SoundOutput.BytesPerSample)) % SoundOutput.BufferSize;
                    
                    if(BytesToLock > TargetCursor)
                    {
                        BytesToWrite  = (SoundOutput.BufferSize - BytesToLock); 
                        BytesToWrite += TargetCursor;
                    }
                    else
                    {
                        BytesToWrite = TargetCursor - BytesToLock; 
                    }
                    
                    SoundIsValid = true; 
                }
                
                sound_buffer SoundBufferData      = {};
                SoundBufferData.SamplesPerSecond  = 48000;
                SoundBufferData.SampleOutputCount = BytesToWrite / SoundOutput.BytesPerSample; 
                SoundBufferData.SampleBuffer      = SampleBufferStorage; 
                
                Game.GetSoundSamples(&GameMemory, &SoundBufferData, &State, &TransientState);
                if(SoundIsValid)
                {
                    Win32FillSoundBuffer(&DSound, &SoundOutput, &SoundBufferData, BytesToLock, BytesToWrite);
                }
                
                ImGui::Render();
                CloverRender(&RenderData);
                ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
                SwapBuffers(WindowDC);
                
                ClearTransientState(&TransientState);
                
                // DELTA
                LARGE_INTEGER EndCounter;
                QueryPerformanceCounter(&EndCounter);
                
                DeltaCounter = real64(EndCounter.QuadPart - LastCounter.QuadPart);
                LastCounter = EndCounter;
                
                FPSTimer += Time.Delta;
                if(FPSTimer >= 1)
                {
                    Time.FPSCounter = int32(PerfCountFrequency / DeltaCounter);
                    Time.MSPerFrame = GetFPSTime();
                    FPSTimer = 0;
                }
                //printm("%.02fms\n", MSPerFrame);
                //printm("FPS: %d\n", FPS);
            }
        }
        else
        {
            Check(0, "Failure to create the Window!\n");
        }
    }
    else
    {
        Check(0, "Failure to Register the WindowClass\n");
    }
    return(0);
}
