@echo off
REM /Ox /O2 /Ot /arch:AVX2 for release 
REM -Bt+ for timing info
REM remove -Zi

Set opts=-DCLOVER_SLOW=1 -DENGINE=1 -DCLOVER_PROFILE=0 

Set CommonCompilerFlags=-W4 -std:c++20 -permissive -fp:fast -Fm -GR- -EHa- -Od -Oi -Zi -wd4996 -wd4100 -wd4505
Set CommonLinkerFlags=-ignore:4099 -incremental:no shell32.lib kernel32.lib user32.lib gdi32.lib opengl32.lib "../data/deps/ImGUI/ImGuiDEBUG.lib" "../data/deps/Freetype/freetype.lib" "../data/deps/OpenGL/glad/src/Glad.lib"
Set CommonIncludes=-I"../data/deps" -I"../data/deps/Freetype/include/"

Set Exports=-EXPORT:GameOnAwake -EXPORT:GameUpdateAndDraw -EXPORT:GameFixedUpdate -EXPORT:GameGetSoundSamples

IF NOT EXIST ..\build mkdir ..\build
pushd ..\build
del *.pdb
cl %opts% ../code/Win32_Clover.cpp %CommonIncludes% %CommonCompilerFlags% -MT -link %CommonLinkerFlags% -OUT:"CloverGame.exe" 
cl %opts% ../code/Clover.cpp %CommonIncludes% %CommonCompilerFlags% -MT -LD -link -ignore:4099 "../data/deps/ImGUI/ImGuiDEBUG.lib" -PDB:CloverGame_%RANDOM%.pdb %Exports% -OUT:"CloverGame.dll" 
popd
