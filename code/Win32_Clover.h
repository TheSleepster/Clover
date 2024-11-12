/* date = August 27 2024 06:24 am*/

#ifndef WIN32_CLOVER_H
#define WIN32_CLOVER_H

#if 0
#include "../data/deps/SDL3/include/SDL3/SDL.h"
#include "../data/deps/SDL3/include/SDL3/SDL_gamepad.h"
#include "../data/deps/SDL3/include/SDL3/SDL_joystick.h"
#endif

#include "util/Math.h"
#include "util/Array.h"
#include "util/FileIO.h"
#include "util/String.h"

#include "Clover_Renderer.h"
#include "Clover_Platform.h"

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

// XINPUT
#include <xinput.h>

// DSOUND
#include <mmreg.h>
#include <dsound.h>

#include "../data/deps/OpenGL/glext.h"
#include "../data/deps/OpenGL/wglext.h"
#include "../data/deps/OpenGL/glcorearb.h"

struct wgl_function_pointers
{
    PFNWGLCHOOSEPIXELFORMATARBPROC    wglChoosePixelFormatARB;
    PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB;
    PFNWGLSWAPINTERVALEXTPROC         wglSwapIntervalEXT;
};

struct game_functions
{
    HMODULE  GameCodeDLL;
    FILETIME LastWriteTime;
    
    game_on_awake          *OnAwake;
    game_fixed_update      *FixedUpdate;
    game_update_and_draw   *UpdateAndDraw;
    game_get_sound_samples *GetSoundSamples;
    
    bool32 IsLoaded;
    bool32 IsValid;
};

struct win32_sound_data
{
    LPDIRECTSOUND        DirectSoundObject;
    LPDIRECTSOUNDBUFFER  PrimaryBuffer;
    LPDIRECTSOUNDBUFFER  SecondaryBuffer;
    WAVEFORMATEX         DirectSoundBufferFormat;
};

const uint16 ButtonLookup[]
{
    0x0001,
    0x0002,
    0x0004,
    0x0008,
    0x0010,
    0x0020,
    0x0040,
    0x0080,
    0x0100,
    0x0200,
    0x1000,
    0x2000,
    0x4000,
    0x8000
};

#define DIRECT_SOUND_CREATE(name) HRESULT WINAPI name(LPGUID lpGuid, LPDIRECTSOUND* ppDS, LPUNKNOWN pUnkOuter)
typedef DIRECT_SOUND_CREATE(direct_sound_create);

#define XINPUT_GET_STATE(name) DWORD name(DWORD dwUserIndex, XINPUT_STATE *pState)
typedef XINPUT_GET_STATE(xinput_get_state);
XINPUT_GET_STATE(XInputGetStateStub)
{
    return(ERROR_DEVICE_NOT_CONNECTED);
}

#define XINPUT_SET_STATE(name)  DWORD name(DWORD dwUserIndex, XINPUT_VIBRATION *pVibration)
typedef XINPUT_SET_STATE(xinput_set_state);
XINPUT_SET_STATE(XInputSetStateStub)
{
    return(ERROR_DEVICE_NOT_CONNECTED);
}

global_variable xinput_get_state *XInputGetState_ = XInputGetStateStub;
global_variable xinput_set_state *XInputSetState_ = XInputSetStateStub;

#define XInputGetState XInputGetState_
#define XInputSetState XInputSetState_

#endif // _WIN32_CLOVER_H
