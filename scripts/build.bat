@echo off

set CommonCompilerFLagsDebug= /GS /W3 /Zc:wchar_t /Zi /Od /Oi /Zc:inline /fp:fast /D "_DEBUG" /D "TOM_INTERNAL" /D "_UNICODE" /D "UNICODE" /WX- /MTd /std:c++latest /FC  /EHsc /nologo
set CommonCompilerFLagsRelease= /GS /GL /W3 /Gy /Zc:wchar_t /Zi /Gm- /O2 /Zc:inline /fp:fast /D "NDEBUG"  /D "_UNICODE" /D "UNICODE" /WX- /Gd /Oi /MD /std:c++latest /FC  /EHsc /nologo
set CommonLinkerFlags= -incremental:no -opt:ref user32.lib gdi32.lib winmm.lib Ole32.lib

IF NOT EXIST .\cl_build mkdir .\cl_build


pushd .\cl_build
del *.pdb > NUL 2> NUl
CALL "C:/Program Files/Microsoft Visual Studio/2022/Preview/VC/Auxiliary/Build/vcvars64.bat"
    cl %CommonCompilerFLagsDebug% ..\src\game.cpp -LD /link -incremental:no -opt:ref -PDB:tomato_%random%.pdb -EXPORT:game_get_sound_samples -EXPORT:game_update_and_render
    cl %CommonCompilerFLagsDebug% ..\src\win32_layer.cpp /link %CommonLinkerFlags%
popd
