include(FetchContent)
include(WgpuNative)

set(FETCHCONTENT_UPDATES_DISCONNECTED ON CACHE BOOL "" FORCE)

if(NOT AGA_FETCH_DEPS)
  find_package(flecs CONFIG REQUIRED)
  find_package(glfw3 CONFIG REQUIRED)
  find_package(glm CONFIG REQUIRED)
else()
  FetchContent_Declare(
    flecs
    GIT_REPOSITORY https://github.com/SanderMertens/flecs.git
    GIT_TAG v4.0.5
    GIT_SHALLOW TRUE)
  set(FLECS_BUILD_TESTS OFF CACHE BOOL "" FORCE)
  set(FLECS_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
  FetchContent_MakeAvailable(flecs)

  if(EMSCRIPTEN)
    add_library(glfw INTERFACE)
    add_library(glfw::glfw ALIAS glfw)
    target_link_options(glfw INTERFACE "SHELL:-sUSE_GLFW=3")
  else()
    FetchContent_Declare(
      glfw
      GIT_REPOSITORY https://github.com/glfw/glfw.git
      GIT_TAG 3.4
      GIT_SHALLOW TRUE)
    set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_WAYLAND OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_X11 ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(glfw)
  endif()

  FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG 1.0.1
    GIT_SHALLOW TRUE)
  FetchContent_MakeAvailable(glm)
endif()

if(TARGET flecs::flecs_static)
  set(AGA_FLECS_TARGET flecs::flecs_static)
elseif(TARGET flecs_static)
  set(AGA_FLECS_TARGET flecs_static)
elseif(TARGET flecs::flecs)
  set(AGA_FLECS_TARGET flecs::flecs)
elseif(TARGET flecs)
  set(AGA_FLECS_TARGET flecs)
else()
  message(FATAL_ERROR "flecs target not found")
endif()

if(TARGET glfw::glfw)
  set(AGA_GLFW_TARGET glfw::glfw)
elseif(TARGET glfw)
  set(AGA_GLFW_TARGET glfw)
else()
  message(FATAL_ERROR "glfw target not found")
endif()

if(TARGET glm::glm)
  set(AGA_GLM_TARGET glm::glm)
elseif(TARGET glm)
  set(AGA_GLM_TARGET glm)
else()
  message(FATAL_ERROR "glm target not found")
endif()

aga_fetch_wgpu_native()
