cmake_minimum_required(VERSION 3.22.1)
project(meridian_android LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Reuse the existing Meridian C++ engine in-place. No source duplication:
# these files are compiled straight out of the desktop tree.
set(MERIDIAN_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../../../../../src")

add_library(meridian_native SHARED
    # --- reused verbatim from the desktop Meridian core -------------------
    ${MERIDIAN_ROOT}/core/vt/screen_buffer.cpp
    ${MERIDIAN_ROOT}/core/vt/ansi_parser.cpp
    ${MERIDIAN_ROOT}/core/vt/graphics.cpp

    # --- new, Android-specific --------------------------------------------
    pty/AndroidPTY.cpp
    bridge/TerminalCore.cpp
    jni/meridian_jni.cpp
)

target_include_directories(meridian_native PRIVATE
    ${MERIDIAN_ROOT}
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_options(meridian_native PRIVATE -Wall -Wextra -O2 -fvisibility=hidden)

target_link_libraries(meridian_native android log c++_shared)
target_link_options(meridian_native PRIVATE "-Wl,-z,max-page-size=16384")

