@echo off
REM /Ox /O2 /Ot /arch:AVX2 for release 
REM -Bt+ for timing info
REM remove -Zi

Set opts=-DCLOVER_SLOW=1 -DCLOVER_PROFILE=0

Set CommonCompilerFlags=-nologo -std:c++20 -permissive -F52428800 -fp:fast -Fm -GR- -EHa- -Od -Oi -Zi -W4 -wd4200 -wd4996 -wd4706 -wd4530 -wd4100 -wd4201 -wd4505 -wd4652 -wd4653 -wd4459 -wd4065 -wd4018
Set CommonLinkerFlags=-ignore:4099 -incremental:no shell32.lib kernel32.lib user32.lib gdi32.lib opengl32.lib "../data/deps/ImGUI/ImGuiDEBUG.lib" "../data/deps/Freetype/freetype.lib" "../data/deps/MiniAudio/miniaudio.lib" "../data/deps/OpenGL/glad/src/Glad.lib" "../data/deps/yyjson/lib/yyjson.lib"
Set CommonIncludes=-I"../data/deps" -I"../data/deps/Freetype/include/"

Set Exports=-EXPORT:GameOnAwake -EXPORT:GameUpdateAndDraw -EXPORT:GameFixedUpdate

IF NOT EXIST ..\build mkdir ..\build
pushd ..\build
del *.obj
del *.pdb
del *.rdi
cl %opts% ../code/Win32_Clover.cpp %CommonIncludes% %CommonCompilerFlags% -MT -link %CommonLinkerFlags% -OUT:"CloverGame.exe" 
cl %opts% ../code/Clover.cpp %CommonIncludes% %CommonCompilerFlags% -MT -LD -link-ignore:4099 "../data/deps/ImGUI/ImGuiDEBUG.lib" "../data/deps/MiniAudio/miniaudio.lib" "../data/deps/OpenGL/glad/src/Glad.lib" "../data/deps/Freetype/freetype.lib" "../data/deps/yyjson/lib/yyjson.lib" -PDB:CloverGame_%RANDOM%.pdb %Exports% -OUT:"CloverGame.dll" 
popd
