
@echo off


set CommonCompilerFLagsDebug= /GS /W3 /Zc:wchar_t /Zi /Od   /Zc:inline /fp:fast /D "_DEBUG" /D "TOM_INTERNAL" /D "_UNICODE" /D "UNICODE" /WX- /MDd /std:c++latest /FC  /EHsc /nologo

set CommonCompilerFLagsRelease= /GS /GL /W3 /Gy /Zc:wchar_t /Zi /Gm- /O2 /Zc:inline /fp:fast /D "NDEBUG"  /D "_UNICODE" /D "UNICODE" /WX- /Gd /Oi /MD /std:c++latest /FC  /EHsc /nologo

set CommonLinkerFlags= -incremental:no -opt:ref user32.lib gdi32.lib winmm.lib Ole32.lib

IF NOT EXIST .\cl_build mkdir .\cl_build

pushd .\cl_build
CALL "C:/Program Files/Microsoft Visual Studio/2022/Preview/VC/Auxiliary/Build/vcvars64.bat"
    cl %CommonCompilerFLagsDebug% ..\src\TomatoGame.cpp /LD /link /EXPORT:GameGetSoundSamples /EXPORT:GameUpdateAndRender
    cl %CommonCompilerFLagsDebug% ..\src\Win32TomatoGame.cpp /link %CommonLinkerFlags%
popd
