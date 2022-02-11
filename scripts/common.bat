@echo off

set CommonCompilerFLagsDebug= /GS /W3 /Zc:wchar_t /Zi /Od /Oi /Zc:inline /fp:fast /D "_DEBUG" /D "TOM_INTERNAL" /D "_UNICODE" /D "UNICODE" /WX- /MTd /std:c++latest /FC  /EHsc /nologo
set CommonCompilerFLagsRelease= /GS /GL /W3 /Gy /Zc:wchar_t /Zi /Gm- /O2 /Zc:inline /fp:fast /D "NDEBUG"  /D "_UNICODE" /D "UNICODE" /WX- /Gd /Oi /MD /std:c++latest /FC  /EHsc /nologo
set CommonLinkerFlags= -incremental:no -opt:ref user32.lib gdi32.lib winmm.lib Ole32.lib

IF NOT EXIST T:\cl_build mkdir T:\cl_build
