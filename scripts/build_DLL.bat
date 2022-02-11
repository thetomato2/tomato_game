call common.bat

pushd T:\cl_build
del *.pdb > NUL 2> NUl
CALL "C:/Program Files/Microsoft Visual Studio/2022/Preview/VC/Auxiliary/Build/vcvars64.bat"
    cl %CommonCompilerFLagsDebug% T:\src\tomato_game.cpp -LD /link -incremental:no -opt:ref -PDB:tomato_%random%.pdb -EXPORT:game_get_sound_samples -EXPORT:game_update_and_render
popd
