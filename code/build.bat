@echo off
REM /Ox /O2 /Ot /arch:AVX2 for release 
REM -Bt+ for timing info
REM remove -Zi

Set opts=-DCLOVER_SLOW=1 -DCLOVER_PROFILE=0

Set CommonCompilerFlags=-std:c++20 -permissive -fp:fast -Fm -GR- -EHa- -Od -Oi -Zi -W4 -wd4189 -wd4200 -wd4996 -wd4706 -wd4530 -wd4100 -wd4201 -wd4505 -wd4652 -wd4653
Set CommonLinkerFlags=-ignore:4099 -incremental:no shell32.lib kernel32.lib user32.lib gdi32.lib opengl32.lib "../data/deps/ImGUI/ImGuiDEBUG.lib" "../data/deps/Freetype/freetype.lib" "../data/deps/MiniAudio/miniaudio.lib"
Set CommonIncludes=-I"../data/deps" -I"../data/deps/Freetype/include/"

IF NOT EXIST ..\build mkdir ..\build
pushd ..\build
del *.pdb
cl %opts% ../code/Win32_Clover.cpp %CommonIncludes% %CommonCompilerFlags% -MT -link %CommonLinkerFlags% -OUT:"CloverGame.exe" 
popd

@echo ====================
@echo Compilation Complete
@echo ====================
