// NOTE(Sleepster): Freetype must come first due to the #define internal static inside of the intrinsics header
#include "../data/deps/Freetype/include/ft2build.h"
#include FT_FREETYPE_H

// INTRINSICS
#include "Intrinsics.h"

// UTILS
#include "util/Math.h"
#include "util/Array.h"
#include "util/FileIO.h"
#include "util/MemoryArena.h"
#include "util/CustomStrings.h"

// GLAD
#define GLAD_OPENGL_IMPL
#include "../data/deps/OpenGL/glad/include/glad/glad.h"

// OPENGL HEADERS
#include "../data/deps/OpenGL/glext.h"
#include "../data/deps/OpenGL/wglext.h"
#include "../data/deps/OpenGL/glcorearb.h"

// IMGUI IMPl
#include "../data/deps/ImGui/imgui.h"
#include "../data/deps/ImGui/imgui_impl_win32.h"
#include "../data/deps/ImGUI/imgui_impl_opengl3.h"

// STB IMAGE TEXTURE LOADING
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../data/deps/stb/stb_image.h"
#include "../data/deps/stb/stb_image_write.h"

// MINIAUDIO ENGINE
#define MINIAUDIO_IMPLEMENTATION   
#include "../data/deps/MiniAudio/miniaudio.h"

// WINDOWS
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

// CLOVER HEADERS
#include "Clover_Globals.h"
#include "Clover.h"
#include "Clover_Audio.h"
#include "Clover_Renderer.h"
#include "Clover_Input.h"
#include "Win32_Clover.h"

// FILES FOR UNITY BUILD
#include "Clover_Draw.cpp"
#include "Clover_Input.cpp"


// NOTE(Sleepster): ImGui WNDPROC. It uses this for input
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);


internal inline real32
GetLastTime()
{
    return((1000 *(real32)DeltaCounter) / real32(PerfCountFrequency));
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
            return 0;
        }
        case WM_DESTROY:
        {
            DestroyWindow(WindowHandle);
            PostQuitMessage(0);
            return 0;
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
                    bool WasDown = ((Message.lParam & (1 << 30)) != 0);
                    bool IsDown = ((Message.lParam & (1 << 31)) == 0);
                    
                    KeyCodeID KeyCode = State->KeyCodeLookup[Message.wParam];
                    Key *Key = &State->GameInput.Keyboard.Keys[KeyCode];
                    Key->JustPressed = !Key->JustPressed && !Key->IsDown && IsDown;
                    Key->JustReleased = !Key->JustReleased && Key->IsDown && !IsDown;
                    Key->IsDown = IsDown;
                    
                    
                    bool AltKeyIsDown = ((Message.lParam & (1 << 29)) != 0);
                    if(VKCode == VK_F4 && AltKeyIsDown)
                    {
                        Running = false;
                    }
                }break;
                
                
                case WM_LBUTTONDOWN:
                case WM_RBUTTONDOWN:
                case WM_MBUTTONDOWN:
                case WM_XBUTTONDOWN:
                case WM_LBUTTONUP:
                case WM_RBUTTONUP:
                case WM_MBUTTONUP:
                case WM_XBUTTONUP:
                {
                    uint32 VKCode = (uint32)Message.wParam;
                    bool IsDown = (GetKeyState(VKCode) & (1 << 15));
                    
                    KeyCodeID KeyCode = State->KeyCodeLookup[Message.wParam];
                    Key *Key = &State->GameInput.Keyboard.Keys[KeyCode];
                    Key->JustPressed = !Key->JustPressed && !Key->IsDown && IsDown;
                    Key->JustReleased = !Key->JustReleased && Key->IsDown && !IsDown;
                    Key->IsDown = IsDown;
                }break;
                default:
                {
                    TranslateMessage(&Message);
                    DispatchMessage(&Message);
                }break;
            }
        }
        
        if(ImGui_ImplWin32_WndProcHandler(WindowHandle, Message.message, Message.wParam, Message.lParam) != 0)
            return;
        
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
        Result.UpdateAndRender = (game_update_and_render *) GetProcAddress(Result.GameCodeDLL, "GameUpdateAndRender");
    }
    else
    {
        Result.OnAwake         = GameOnAwakeStub;
        Result.FixedUpdate     = GameFixedUpdateStub;
        Result.UpdateAndRender = GameUpdateAndRenderStub;
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
    GameCode->UpdateAndRender = GameUpdateAndRenderStub;
}

int CALLBACK
WinMain(HINSTANCE hInstance,
        HINSTANCE hPrevInstance,
        LPSTR lpCmdLine,
        int32 nShowCmd)
{
    WNDCLASS              Window = {};
    time                  Time   = {};
    game_state            State  = {};
    game_memory           Memory = {};
    game_functions        Game          = {};
    wgl_function_pointers WGLFunctions  = {};
    gl_render_data        RenderData    = {};
    
    // NOTE(Sleepster): Accumulator is for Delta Time
    real32 Accumulator = {};
    
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
            
            LARGE_INTEGER LastCounter;
            QueryPerformanceCounter(&LastCounter);
            
            Memory.TemporaryStorage = ArenaCreate(Megabytes(200));
            Memory.PermanentStorage = ArenaCreate(Megabytes(100));
            
            RenderData.DrawFrame.Vertices = (vertex *)ArenaAlloc(&Memory.PermanentStorage, sizeof(vertex) * MAX_VERTICES);
            RenderData.DrawFrame.VertexBufferptr = &RenderData.DrawFrame.Vertices[0];
            
            Win32LoadKeyData(&State);
            Win32LoadDefaultBindings(&State.GameInput);
            
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
            
            
            CloverSetupRenderer(&Memory.TemporaryStorage, &RenderData);
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
            
            // Choose Dear ImGui style
            ImGui::StyleColorsDark();
            
            // Setup Platform/Renderer backends
            ImGui_ImplWin32_InitForOpenGL(WindowHandle);
            ImGui_ImplOpenGL3_Init();
            
            
            // NOTE(Sleepster): Audio Engine setup, MiniAudio makes this REALLLLLLYYYYYYYY easy 
            State.SFXData.AudioEngine = {};
            Assert(ma_engine_init(0, &State.SFXData.AudioEngine) == MA_SUCCESS);
            Assert(ma_engine_set_volume(&State.SFXData.AudioEngine, 0.1f) == MA_SUCCESS);
            
            Game.OnAwake(&Memory, &RenderData, &State);
            
            real32 FPSTimer = 0;
            Running = 1;
            while(Running)
            {
                MSG Message = {};
                Win32ProcessInputMessages(Message, WindowHandle, &State);
                
                Time.Delta   = SIMRATE/1000;
                Time.Current = GetLastTime();
                Time.Next    = Time.Current + Time.Delta;
                
                
                
                //DATA RELOADING
#if CLOVER_SLOW
                FILETIME NewDLLWriteTime = Win32GetLastWriteTime(STR("CloverGame.dll"));
                if(CompareFileTime(&Game.LastWriteTime, &NewDLLWriteTime) != 0)
                {
                    Win32UnloadGameCode(&Game);
                    Game = Win32LoadGameCode(STR("CloverGame.dll"));
                    
                    // NOTE(Sleepster): Audio Engine setup, MiniAudio makes this REALLLLLLYYYYYYYY easy 
                    
                    for(uint32 i = 0; i < MAX_SOUNDS; i++)
                    {
                        sound_instance *Sound = &State.SFXData.Instances[i];
                        ma_sound_stop(&Sound->Sound);
                        ma_sound_uninit(&Sound->Sound);
                        Sound->IsPlaying = 0;
                        Sound->IsActive = 0;
                    }
                    
                    for(uint32 i = 0; i < MAX_TRACKS; i++)
                    {
                        sound_instance *Sound = &State.SFXData.SoundTracks[i];
                        ma_sound_stop(&Sound->Sound);
                        ma_sound_uninit(&Sound->Sound);
                        Sound->IsPlaying = 0;
                        Sound->IsActive = 0;
                    }
                    
                    if(ma_engine_stop(&State.SFXData.AudioEngine) == MA_SUCCESS)
                    {
                        ma_engine_uninit(&State.SFXData.AudioEngine);
                    }
                    Assert(ma_engine_init(0, &State.SFXData.AudioEngine) == MA_SUCCESS);
                    Assert(ma_engine_set_volume(&State.SFXData.AudioEngine, 0.1f) == MA_SUCCESS);
                    
                    Game.OnAwake(&Memory, &RenderData, &State);
                }
                
                
                FILETIME NewTextureWriteTime        = Win32GetLastWriteTime(RenderData.GameAtlas.Filepath);    
                FILETIME NewVertexShaderWriteTime   = Win32GetLastWriteTime(RenderData.BasicShader.VertexShader.Filepath);
                FILETIME NewFragmentShaderWriteTime = Win32GetLastWriteTime(RenderData.BasicShader.FragmentShader.Filepath);
                
                if(CompareFileTime(&NewTextureWriteTime, &RenderData.GameAtlas.LastWriteTime) != 0)
                {
                    CloverReloadTexture(&RenderData, &RenderData.GameAtlas, 0);
                    Sleep(100);
                }
                
                if(CompareFileTime(&NewVertexShaderWriteTime,   &RenderData.BasicShader.VertexShader.LastWriteTime) != 0 ||
                   CompareFileTime(&NewFragmentShaderWriteTime, &RenderData.BasicShader.FragmentShader.LastWriteTime) != 0)
                {
                    glDeleteProgram(RenderData.BasicShader.Shader);
                    CloverCreateShader(&Memory.TemporaryStorage, RenderData.BasicShader.VertexShader.Filepath, RenderData.BasicShader.FragmentShader.Filepath);
                    Sleep(100);
                }
#endif
                
                
                
                while(Accumulator >= SIMRATE)
                {
                    Game.FixedUpdate(&Memory, &RenderData, &State, Time);
                    Accumulator -= Time.Delta;
                }
                Time.Alpha = Accumulator / Time.Delta;
                
                // Start the Dear ImGui frame
                ImGui_ImplOpenGL3_NewFrame();
                ImGui_ImplWin32_NewFrame();
                ImGui::NewFrame();
                
                RenderData.AspectRatio = (real32)SizeData.Width / (real32)SizeData.Height;
                Game.UpdateAndRender(&Memory, &RenderData, &State, Time, SizeData);
                
                ImGui::Render();
                glClearColor(RenderData.ClearColor.R, RenderData.ClearColor.G, RenderData.ClearColor.B, RenderData.ClearColor.A);
                glClearDepth(0.0f);
                glViewport(0, 0, SizeData.Width, SizeData.Height);
                glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
                CloverRender(&RenderData);
                ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
                
                SwapBuffers(WindowDC);
                
                ArenaReset(&Memory.TemporaryStorage);
                RenderData.DrawFrame.VertexBufferptr = &RenderData.DrawFrame.Vertices[0];
                RenderData.DrawFrame.QuadCount = 0;
                
                // DELTA
                LARGE_INTEGER EndCounter;
                QueryPerformanceCounter(&EndCounter);
                
                DeltaCounter = real64(EndCounter.QuadPart - LastCounter.QuadPart);
                real32 MSPerFrame = GetLastTime();
                LastCounter = EndCounter;
                
                Accumulator += Time.Delta;
                
                FPSTimer += Time.Delta;
                if(FPSTimer >= 10)
                {
                    Time.FPSCounter = int32(PerfCountFrequency / DeltaCounter);
                    FPSTimer = 0;
                }
                //printm("%.02fms\n", MSPerFrame);
                //printm("FPS: %d\n", FPS);
                
                if(IsKeyDown(KEY_ESCAPE, &State.GameInput))
                {
                    Running = 0;
                }
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
