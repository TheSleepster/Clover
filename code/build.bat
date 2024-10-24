@echo off
REM /Ox /O2 /Ot /arch:AVX2 for release 
REM -Bt+ for timing info
REM remove -Zi

Set opts=-DCLOVER_SLOW=1 -DCLOVER_PROFILE=0 -DENGINE=1

Set CommonCompilerFlags=-std:c++20 -permissive -fp:fast -GR- -EHa- -Od -Oi -Zi -W4 -Og -Wno-missing-braces -Wno-unused-function -Wno-unused-parameter -Wno-missing-field-initializers -Wno-nonportable-include-path -Wno-deprecated-declarations -Wno-char-subscripts -Wno-pointer-bool-conversion -Wno-switch -Wno-delayed-template-parsing-in-cxx20 -Wno-writable-strings -Wno-microsoft-include 
Set CommonLinkerFlags=-ignore:4099 -STACK:50000000 -incremental:no shell32.lib kernel32.lib user32.lib gdi32.lib opengl32.lib "../data/deps/ImGUI/ImGuiDEBUG.lib" "../data/deps/Freetype/freetype.lib" "../data/deps/MiniAudio/miniaudio.lib" "../data/deps/OpenGL/glad/src/Glad.lib" "../data/deps/yyjson/lib/yyjson.lib"
Set CommonIncludes=-I"../data/deps" -I"../data/deps/Freetype/include/"

Set Exports=-EXPORT:GameOnAwake -EXPORT:GameUpdateAndDraw -EXPORT:GameFixedUpdate

IF NOT EXIST ..\build mkdir ..\build
pushd ..\build
del *.obj
del *.pdb
clang-cl %opts% ../code/Win32_Clover.cpp %CommonIncludes% %CommonCompilerFlags% -MT -link %CommonLinkerFlags% -OUT:"CloverGame.exe" 
clang-cl %opts% ../code/Clover.cpp %CommonIncludes% %CommonCompilerFlags% -MT -LD -link -ignore:4099 "../data/deps/ImGUI/ImGuiDEBUG.lib" "../data/deps/MiniAudio/miniaudio.lib" "../data/deps/OpenGL/glad/src/Glad.lib" "../data/deps/Freetype/freetype.lib" "../data/deps/yyjson/lib/yyjson.lib" -PDB:CloverGame_%RANDOM%.pdb %Exports% -OUT:"CloverGame.dll" 
popd

@echo ====================
@echo Compilation Complete
@echo ====================

