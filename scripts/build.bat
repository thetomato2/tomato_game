call T:\scripts\common.bat
set SourceFiles=  T:\src\game.cpp T:\src\entity.cpp T:\src\sim_region.cpp T:\src\world.cpp

pushd T:\cl_build
del *.pdb > NUL 2> NUl
CALL "C:/Program Files/Microsoft Visual Studio/2022/Preview/VC/Auxiliary/Build/vcvars64.bat"
    cl %CommonCompilerFLagsDebug% %SourceFiles% -o tomato_game  -LD /link -incremental:no -opt:ref -PDB:tomato_%random%.pdb -EXPORT:game_get_sound_samples -EXPORT:game_update_and_render
    cl %CommonCompilerFLagsDebug% T:\src\platform_win32.cpp /link %CommonLinkerFlags%
popd
