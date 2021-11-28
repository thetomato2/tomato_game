cmake_minimum_required(VERSION 3.20)

project ("TomatoGame"
        VERSION 0.1
        DESCRIPTION "For testing CMake stuff"
        LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(triple x86_64-pc-windows-gnu)

set(CMAKE_C_COMPILER clang)
set(CMAKE_C_COMPILER_TARGET ${triple})
set(CMAKE_CXX_COMPILER clang++)
set(CMAKE_CXX_COMPILER_TARGET ${triple})

set (CMAKE_CXX_FLAGS "-fdeclspec -target x86_64-pc-windows-gnu")
add_executable(Win32TomatoGame WIN32
    # "./src/tomato_framework.h"
    # "./src/base_types.h"
    # "./src/macros.h"
    # "./src/console.h"
    # "./src/utils.h"
    # "./src/utils.cpp"
    # "./src/tomato_game.h"
    # "./src/tomato_game.cpp"
    # "./src/win32_tomato_game.h"
    "./src/win32_tomato_game.cpp"
    )

    # where win32 headers in Arch are located:
    # ./x86_64-w64-mingw32/include/tchar.h
    # ./i686-w64-mingw32/include/tchar.h
    #
set(WIN32_INCLUDE_DIRS
    # "/usr/x86_w64-mingw32/include/"
    "/usr/i686-w64-mingw32/include/"
    "/mnt/c/dev/include/for_emacs/c++/"
    )

target_compile_definitions(Win32TomatoGame PRIVATE _WIN32 _UNICODE UNICODE TOM_INTERNAL EMACS)
target_include_directories(Win32TomatoGame PUBLIC ${WIN32_INCLUDE_DIRS})
