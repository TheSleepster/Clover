@echo off
REM /Ox /O2 /Ot /arch:AVX2 for release 
REM -Bt+ for timing info
REM remove -Zi

Set opts=-DCLOVER_SLOW=1 -DENGINE=1 -DCLOVER_PROFILE=0 

Set CommonCompilerFlags=-std:c++20 -permissive -fp:fast -Fm -GR- -EHa- -Od -Oi -Zi -W4 -wd4189 -wd4200 -wd4996 -wd4706 -wd4530 -wd4100 -wd4201 -wd4505 -wd4652 -wd4653 -wd4127 -wd4459
Set CommonLinkerFlags=-ignore:4099 -STACK:50000000 -incremental:no shell32.lib kernel32.lib user32.lib gdi32.lib opengl32.lib winmm.lib advapi32.lib ole32.lib setupapi.lib imm32.lib version.lib oleaut32.lib "../data/deps/ImGUI/ImGuiDEBUG.lib" "../data/deps/Freetype/freetype.lib" "../data/deps/MiniAudio/miniaudio.lib" "../data/deps/OpenGL/glad/src/Glad.lib" "../data/deps/yyjson/lib/yyjson.lib" "../data/deps/SDL3/lib/SDL3-static.lib" "../data/deps/SDL3/lib/SDL_uclibc.lib"
Set CommonIncludes=-I"../data/deps" -I"../data/deps/Freetype/include/" -I"../data/deps/SDL3/include/"

Set Exports=-EXPORT:GameOnAwake -EXPORT:GameUpdateAndDraw -EXPORT:GameFixedUpdate

IF NOT EXIST ..\build mkdir ..\build
pushd ..\build
del *.pdb
cl %opts% ../code/Win32_Clover.cpp %CommonIncludes% %CommonCompilerFlags% -MT -link %CommonLinkerFlags% -OUT:"CloverGame.exe" 
cl %opts% ../code/Clover.cpp %CommonIncludes% %CommonCompilerFlags% -MT -LD -link -ignore:4099 "../data/deps/ImGUI/ImGuiDEBUG.lib" "../data/deps/MiniAudio/miniaudio.lib" "../data/deps/OpenGL/glad/src/Glad.lib" "../data/deps/Freetype/freetype.lib" "../data/deps/yyjson/lib/yyjson.lib" -PDB:CloverGame_%RANDOM%.pdb %Exports% -OUT:"CloverGame.dll" 

popd

@echo ====================
@echo Compilation Complete
@echo ====================
