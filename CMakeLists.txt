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

option(Emacs "Emacs" OFF)
option(argb_converter "ARGB_Converter" OFF)

if (MSVC)
    list(APPEND Common_Compile_Flags
        -fp:fast
        -fp:except-
        # -FC
        # -Gm-
        -GR-
        # -GS
        -EHsc
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
        _HAS_EXCEPTIONS=0
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



add_library(Tomato_Game_Lib SHARED
    ${Source_Dir}/tomato_game.cpp
    ${Source_Dir}/tomato_game.hpp
    ${Source_Dir}/tomato_intrinsic.hpp
    ${Source_Dir}/tomato_math.hpp
    ${Source_Dir}/tomato_platform.h
    ${Source_Dir}/tomato_world.cpp
    ${Source_Dir}/tomato_world.hpp
    )

set_source_files_properties(${Source_Dir}/tomato_world.cpp PROPERTIES HEADER_FILE_ONLY TRUE)

set_target_properties(Tomato_Game_Lib PROPERTIES OUTPUT_NAME tomato_game)
target_include_directories(Tomato_Game_Lib PUBLIC ${Source_Dir})

if (MSVC)
    target_compile_options(Tomato_Game_Lib PRIVATE
        ${Comman_Compile_FLags}
        # -MTd
        )

    set_target_properties( Tomato_Game_Lib PROPERTIES
        VS_DEBUGGER_WORKING_DIRECTORY "$(TargetDir)")

    target_compile_definitions(Tomato_Game_Lib PRIVATE
        ${Common_Compile_Defs}
        _MSVC
        )

    target_link_options(Tomato_Game_Lib PRIVATE
        ${Common_Linker_Flags}
        -EXPORT:game_get_sound_samples
        -EXPORT:game_update_and_render
        )
endif()

add_executable(Win32_Tomato_Game WIN32
    ${Source_Dir}/win32_tomato.cpp
    )

target_include_directories(Win32_Tomato_Game PRIVATE ${Source_Dir})

if (MSVC)
    add_dependencies(Win32_Tomato_Game Tomato_Game_Lib)

    target_compile_options(Win32_Tomato_Game PRIVATE
        ${Common_Compile_FLags}
        # -MTd
        )

    set_target_properties( Win32_Tomato_Game PROPERTIES
        VS_DEBUGGER_WORKING_DIRECTORY "$(TargetDir)")

    target_compile_definitions(Win32_Tomato_Game PRIVATE ${Common_Compile_Defs})

    target_link_libraries(Win32_Tomato_Game PRIVATE
        ${Common_Linker_Libs}
        )

    target_link_options(Tomato_Game_Lib PRIVATE
        ${Common_Linker_Flags}
        # -NODEFAULTLIB
        )

    set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY
        VS_STARTUP_PROJECT Win32_Tomato_Game)
endif()

add_executable(argb_converter
    ${Source_Dir}/win32_argb_converter.cpp
    ${Extern_Dir}/stb_image.h
    )

target_include_directories(argb_converter PRIVATE ${Source_Dir})

if (MSVC)
    target_compile_options(argb_converter PRIVATE
        ${Common_Compile_FLags}
        # -MTd
        )

    set_target_properties(argb_converter PROPERTIES
        VS_DEBUGGER_WORKING_DIRECTORY "$(TargetDir)")

    target_compile_definitions(argb_converter PRIVATE ${Common_Compile_Defs})

    target_link_libraries(argb_converter PRIVATE
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

    target_compile_definitions(Win32_Tomato_Game PRIVATE _WIN32 _UNICODE UNICODE TOM_INTERNAL EMACS)
    target_include_directories(Win32_Tomato_Game PUBLIC ${WIN32_INCLUDE_DIRS})
endif()
