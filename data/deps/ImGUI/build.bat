del *.lib 
del *.pdb

call shell.bat

cl -Z7 -Od -Oi imgui*.cpp -c
lib *.obj -OUT:ImGuiDEBUG.lib
del *.obj
