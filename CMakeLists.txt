cmake_minimum_required(VERSION 3.27)

project(
  aga_engine
  VERSION 0.1.0
  DESCRIPTION "a small c++23 game engine"
  LANGUAGES C CXX)

option(AGA_BUILD_DEMO "build the demo game" ON)
option(AGA_FETCH_DEPS "download third party dependencies with fetchcontent" ON)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(CompilerWarnings)
include(Dependencies)

add_library(aga_options INTERFACE)
add_library(aga::options ALIAS aga_options)
target_compile_features(aga_options INTERFACE cxx_std_23)
target_compile_definitions(aga_options INTERFACE GLM_FORCE_RADIANS
                                                 GLM_FORCE_DEPTH_ZERO_TO_ONE
                                                 GLM_ENABLE_EXPERIMENTAL)
if(EMSCRIPTEN)
  target_link_options(aga_options INTERFACE "SHELL:-sSTACK_SIZE=2097152")
endif()

add_subdirectory(engine/core)
add_subdirectory(engine/window)
add_subdirectory(engine/input)
add_subdirectory(engine/transform)
add_subdirectory(engine/camera)
add_subdirectory(engine/render)
add_subdirectory(engine/aga)

if(AGA_BUILD_DEMO)
  add_subdirectory(demo)
endif()
