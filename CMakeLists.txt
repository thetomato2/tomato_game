cmake_minimum_required(VERSION 3.21)

project ("Tomato_Game"
        VERSION 0.1
        DESCRIPTION "A game, made by me"
        LANGUAGES CXX)


set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_UNITY_BUILD OFF)


# Note: by default ENABLE_DEVELOPER_MODE is True
# This means that all analysis (sanitizers, static analysis)
# is enabled and all warnings are treated as errors
# if you want to switch this behavior, change TRUE to FALSE
# set(ENABLE_DEVELOPER_MODE
#     TRUE
#     CACHE BOOL "Enable 'developer mode'")

# # Change this to false if you want to disable warnings_as_errors in developer mode
# set(OPT_WARNINGS_AS_ERRORS_DEVELOPER_DEFAULT TRUE)



set(SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/src)
set(SHADER_DIR ${CMAKE_CURRENT_LIST_DIR}/shaders)
set(DATA_DIR ${CMAKE_CURRENT_LIST_DIR}/data)
set(FONT_DIR ${CMAKE_CURRENT_LIST_DIR}/fonts)
set(EXTERN_DIR ${CMAKE_CURRENT_LIST_DIR}/extern)
# set(BUILD_DIR ${CMAKE_CURRENT_LIST_DIR}/build)

# set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${BUILD_DIR})
# set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${BUILD_DIR})
# set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${BUILD_DIR})

list(APPEND COMPILE_FLAGS
    # -arch:AVX
    # -arch:AVX2
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

set(CMAKE_CXX_FLAG -ftime-trace)

list(APPEND COMPILE_DEFS
    TOM_INTERNAL
    TOM_WIN32
    )

list(APPEND LINKER_FLAGS
    # -STACK:0x100000,0x100000
    # -incremental:no
    # -opt:ref
    )

list(APPEND COMMON_LINKER_LIBS
    user32.lib
    gdi32.lib
    winmm.lib
    kernel32.lib
    Ole32.lib
    d3d11.lib
    d3dcompiler.lib
    dxguid.lib
    )

file(GLOB SHADERS
    ${SHADER_DIR}/*.hlsl
    )

file(GLOB DATA
    ${DATA_DIR}/*.ico
    )

file(GLOB FONTS
    ${FONT_DIR}/*.ttf
    )


# add_library(stb STATIC
#     ${EXTERN_DIR}/stb/stb_image.h
#     ${EXTERN_DIR}/stb/stb_image.cpp
#     )

add_executable(tomato_game WIN32
    ${SOURCE_DIR}/winmain.cc
    ${SOURCE_DIR}/tom_app.cc
    ${SOURCE_DIR}/tom_app.hh
    # ${SOURCE_DIR}/tom_camera.cc
    # ${SOURCE_DIR}/tom_camera.hh
    ${SOURCE_DIR}/tom_color.hh
    ${SOURCE_DIR}/tom_core.hh
    ${SOURCE_DIR}/tom_d3d11.cc
    ${SOURCE_DIR}/tom_d3d11.hh
    ${SOURCE_DIR}/tom_DS5.cc
    ${SOURCE_DIR}/tom_DS5.hh
    ${SOURCE_DIR}/tom_dx_error.hh
    ${SOURCE_DIR}/tom_entity.cc
    ${SOURCE_DIR}/tom_entity.hh
    ${SOURCE_DIR}/tom_extern.cc
    ${SOURCE_DIR}/tom_file_io.cc
    ${SOURCE_DIR}/tom_file_io.hh
    # ${SOURCE_DIR}/tom_font.cc
    # ${SOURCE_DIR}/tom_font.hh
    ${SOURCE_DIR}/tom_game.cc
    ${SOURCE_DIR}/tom_game.hh
    ${SOURCE_DIR}/tom_globals.hh
    ${SOURCE_DIR}/tom_graphics.cc
    ${SOURCE_DIR}/tom_graphics.hh
    ${SOURCE_DIR}/tom_input.cc
    ${SOURCE_DIR}/tom_input.hh
    ${SOURCE_DIR}/tom_intrinsic.hh
    ${SOURCE_DIR}/tom_math.hh
    ${SOURCE_DIR}/tom_memory.hh
    ${SOURCE_DIR}/tom_rng_nums.hh
    ${SOURCE_DIR}/tom_sim.cc
    ${SOURCE_DIR}/tom_sim.hh
    # ${SOURCE_DIR}/tom_sound.cc
    # ${SOURCE_DIR}/tom_sound.hh
    ${SOURCE_DIR}/tom_string.hh
    ${SOURCE_DIR}/tom_texture.hh
    ${SOURCE_DIR}/tom_texture.cc
    ${SOURCE_DIR}/tom_thread.cc
    ${SOURCE_DIR}/tom_thread.hh
    ${SOURCE_DIR}/tom_time.hh
    ${SOURCE_DIR}/tom_types.hh
    ${SOURCE_DIR}/tom_win32.cc
    ${SOURCE_DIR}/tom_win32.hh
    # ${SOURCE_DIR}/tom_world.cc
    # ${SOURCE_DIR}/tom_world.hh
    ${SOURCE_DIR}/winmain.cc
)

target_include_directories(tomato_game PRIVATE ${SOURCE_DIR})

target_compile_options(tomato_game  PRIVATE
    ${Compile_FLags}
    -mavx
    -mavx2
    # -march=knl
    )

target_compile_definitions(tomato_game  PRIVATE ${COMPILE_DEFS})
target_link_libraries(tomato_game PRIVATE ${COMMON_LINKER_LIBS})


target_link_options(tomato_game PRIVATE
    ${LINKER_FLAGS}
    # -NODEFAULTLIB
)

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY
VS_STARTUP_PROJECT tomato_game )
    
add_custom_command(TARGET tomato_game 
    PRE_BUILD
        COMMAND ctime -begin ${CMAKE_CURRENT_LIST_DIR}/data/tomato_game.ctm
)

add_custom_command(
    TARGET tomato_game POST_BUILD
    COMMAND ctime -end ${CMAKE_CURRENT_LIST_DIR}/data/tomato_game.ctm
    # COMMAND ${CMAKE_COMMAND} -E copy_directory ${SHADER_DIR} $<TARGET_FILE_DIR:tomato_editor>/shaders DEPENDS ${SHADERS}
    # COMMAND ${CMAKE_COMMAND} -E copy_directory ${DATA_DIR} $<TARGET_FILE_DIR:tomato_editor>/data DEPENDS ${DATA}
    # COMMAND ${CMAKE_COMMAND} -E copy_directory ${FONT_DIR} $<TARGET_FILE_DIR:tomato_editor>/fonts DEPENDS ${FONTS}
    # COMMAND ${CMAKE_COMMAND} -E remove -f ${CMAKE_CURRENT_LIST_DIR}/build_times.txt
    # COMMAND ctime -csv ${CMAKE_CURRENT_LIST_DIR}/data/tomato_editor.ctm >> ${CMAKE_CURRENT_LIST_DIR}/build_times.txt
)
