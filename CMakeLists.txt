cmake_minimum_required(VERSION 3.20)

project ("Tomato_Game"
        VERSION 0.1
        DESCRIPTION "My personal game thing"
        LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_UNITY_BUILD ON)

set(Source_Dir ${CMAKE_CURRENT_LIST_DIR}/src)
set(Extern_Dir ${CMAKE_CURRENT_LIST_DIR}/external)
set(Build_Dir ${CMAKE_CURRENT_LIST_DIR}/build)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${Build_Dir})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${Build_Dir})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${Build_Dir})
set(CMAKE_SUPPRESS_REGENERATION true) #feck off ZERO_CHECK

option(Emacs "Emacs" OFF)
option(ArgbConverter "ARGB_Converter" OFF)

if (MSVC)
    list(APPEND Common_Compile_Flags
        -fp:fast
        -fp:except-
        # -FC
        # -Gm-
        -GR-
        # -GS
        # -EHsc
        # -Od
        # -Oi
        -WX
        -W4
        # -wd4201
        # -wd4100
        # -wd4189
        # -wd4505
        # -wd4127
        # -Z7
        -Zc:inline
        -Zc:wchar_t
        # -Zo
        )

    list(APPEND Common_Compile_Defs
        TOM_INTERNAL
        )

    list(APPEND Common_Linker_Flags
        # -STACK:0x100000,0x100000
        -incremental:no
        -opt:ref
        )

    list(APPEND Common_Linker_Libs
        user32.lib
        gdi32.lib
        winmm.lib
        kernel32.lib
        Ole32.lib
        )
endif()



add_library(TomatoGameLib SHARED
    ${Source_Dir}/Common.hpp
    ${Source_Dir}/Entity.hpp
    ${Source_Dir}/Image.hpp
    ${Source_Dir}/Intrinsic.hpp
    ${Source_Dir}/Game.cpp
    ${Source_Dir}/Game.hpp
    ${Source_Dir}/Math.hpp
    ${Source_Dir}/Platform.h
    ${Source_Dir}/SimRegion.cpp
    ${Source_Dir}/SimRegion.hpp
    ${Source_Dir}/World.cpp
    ${Source_Dir}/World.hpp
    ${Source_Dir}/Utils.hpp
    )

# because I have a real unity build going on,
# without this cmake will shove the cpp files into a unity.cpp
# HACK: fix these or leave it?
# set_source_files_properties(
#     ${Source_Dir}/world.cpp
#     ${Source_Dir}/sim_region.cpp
#     PROPERTIES HEADER_FILE_ONLY TRUE)

set_target_properties(TomatoGameLib PROPERTIES OUTPUT_NAME tomato_game)
target_include_directories(TomatoGameLib PUBLIC ${Source_Dir})

if (MSVC)
    target_compile_options(TomatoGameLib PRIVATE
        ${Comman_Compile_FLags}
        # -MTd
        )

    set_target_properties( TomatoGameLib PROPERTIES
        VS_DEBUGGER_WORKING_DIRECTORY "$(TargetDir)")

    target_compile_definitions(TomatoGameLib PRIVATE
        ${Common_Compile_Defs}
        _MSVC
        )

    target_link_options(TomatoGameLib PRIVATE
        ${Common_Linker_Flags}
        -EXPORT:game_get_sound_samples
        -EXPORT:game_update_and_render
        )
endif()

add_executable(TomatoGameWin32 WIN32
    ${Source_Dir}/PlatformWin32.cpp
    )

target_include_directories(TomatoGameWin32 PRIVATE ${Source_Dir})

if (MSVC)
    add_dependencies(TomatoGameWin32 TomatoGameLib)

    target_compile_options(TomatoGameWin32 PRIVATE
        ${Common_Compile_FLags}
        # -MTd
        )

    set_target_properties( TomatoGameWin32 PROPERTIES
        VS_DEBUGGER_WORKING_DIRECTORY "$(TargetDir)")

    target_compile_definitions(TomatoGameWin32 PRIVATE ${Common_Compile_Defs})

    target_link_libraries(TomatoGameWin32 PRIVATE
        ${Common_Linker_Libs}
        )

    target_link_options(TomatoGameLib PRIVATE
        ${Common_Linker_Flags}
        # -NODEFAULTLIB
        )

    set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY
        VS_STARTUP_PROJECT TomatoGameWin32)
endif()

add_executable(ArgbConverter
    ${Source_Dir}/ArgbConverterWin32.cpp
    ${Extern_Dir}/stb_image.h
    )

target_include_directories(ArgbConverter PRIVATE ${Source_Dir})

if (MSVC)
    target_compile_options(ArgbConverter PRIVATE
        ${Common_Compile_FLags}
        # -MTd
        )

    set_target_properties(ArgbConverter PROPERTIES
        VS_DEBUGGER_WORKING_DIRECTORY "$(TargetDir)")

    target_compile_definitions(ArgbConverter PRIVATE ${Common_Compile_Defs})

    target_link_libraries(ArgbConverter PRIVATE
        ${Common_Linker_Libs}
        )

endif()



if (Emacs)
    set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

    set (CMAKE_CXX_FLAGS "-fdeclspec -target x86_64-pc-windows-gnu")
    set(triple x86_64-pc-windows-gnu)

    set(CMAKE_C_COMPILER clang)
    set(CMAKE_C_COMPILER_TARGET ${triple})
    set(CMAKE_CXX_COMPILER clang++)
    set(CMAKE_CXX_COMPILER_TARGET ${triple})

    set(WIN32_INCLUDE_DIRS
        # "/usr/x86_w64-mingw32/include/"
        "/usr/i686-w64-mingw32/include/"
        "/mnt/c/dev/include/for_emacs/c++/"
        )

    target_compile_definitions(TomatoGameWin32 PRIVATE _WIN32 _UNICODE UNICODE TOM_INTERNAL EMACS)
    target_include_directories(TomatoGameWin32 PUBLIC ${WIN32_INCLUDE_DIRS})
endif()
