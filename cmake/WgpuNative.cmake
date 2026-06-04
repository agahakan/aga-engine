include(FetchContent)

function(aga_fetch_wgpu_native)
  if(TARGET aga_wgpu)
    return()
  endif()

  if(EMSCRIPTEN)
    add_library(aga_wgpu INTERFACE)
    add_library(aga::wgpu ALIAS aga_wgpu)
    target_compile_definitions(aga_wgpu INTERFACE AGA_BROWSER_WEBGPU=1)
    target_link_options(aga_wgpu INTERFACE "SHELL:--use-port=emdawnwebgpu" "SHELL:-sASYNCIFY=1")
    return()
  endif()

  set(AGA_WGPU_VERSION v29.0.0.0 CACHE STRING "wgpu-native release tag")

  string(TOLOWER "${CMAKE_SYSTEM_PROCESSOR}" AGA_CPU)
  if(AGA_CPU MATCHES "^(x86_64|amd64)$")
    set(AGA_WGPU_ARCH x86_64)
  elseif(AGA_CPU MATCHES "^(aarch64|arm64)$")
    set(AGA_WGPU_ARCH aarch64)
  else()
    message(FATAL_ERROR "unsupported wgpu-native cpu: ${CMAKE_SYSTEM_PROCESSOR}")
  endif()

  if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set(AGA_WGPU_ARCHIVE "wgpu-linux-${AGA_WGPU_ARCH}-release.zip")
    set(AGA_WGPU_LIBRARY "lib/libwgpu_native.so")
  elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    set(AGA_WGPU_ARCHIVE "wgpu-macos-${AGA_WGPU_ARCH}-release.zip")
    set(AGA_WGPU_LIBRARY "lib/libwgpu_native.dylib")
  elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    if(MSVC)
      set(AGA_WGPU_ARCHIVE "wgpu-windows-${AGA_WGPU_ARCH}-msvc-release.zip")
    else()
      set(AGA_WGPU_ARCHIVE "wgpu-windows-${AGA_WGPU_ARCH}-gnu-release.zip")
    endif()
    set(AGA_WGPU_LIBRARY "lib/wgpu_native.dll.lib")
  else()
    message(FATAL_ERROR "unsupported wgpu-native platform: ${CMAKE_SYSTEM_NAME}")
  endif()

  set(AGA_WGPU_URL
      "https://github.com/gfx-rs/wgpu-native/releases/download/${AGA_WGPU_VERSION}/${AGA_WGPU_ARCHIVE}")

  FetchContent_Declare(wgpu_native URL "${AGA_WGPU_URL}" DOWNLOAD_EXTRACT_TIMESTAMP TRUE)
  FetchContent_GetProperties(wgpu_native)
  if(NOT wgpu_native_POPULATED)
    if(POLICY CMP0169)
      cmake_policy(PUSH)
      cmake_policy(SET CMP0169 OLD)
    endif()
    FetchContent_Populate(wgpu_native)
    if(POLICY CMP0169)
      cmake_policy(POP)
    endif()
  endif()

  add_library(aga_wgpu SHARED IMPORTED GLOBAL)
  add_library(aga::wgpu ALIAS aga_wgpu)
  set_target_properties(
    aga_wgpu
    PROPERTIES IMPORTED_LOCATION "${wgpu_native_SOURCE_DIR}/${AGA_WGPU_LIBRARY}"
               IMPORTED_NO_SONAME TRUE
               INTERFACE_INCLUDE_DIRECTORIES "${wgpu_native_SOURCE_DIR}/include")

  target_compile_definitions(aga_wgpu INTERFACE AGA_NATIVE_WGPU=1)
endfunction()

function(aga_copy_wgpu_runtime target)
  if(EMSCRIPTEN)
    return()
  endif()

  add_custom_command(
    TARGET ${target}
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:aga_wgpu>
            $<TARGET_FILE_DIR:${target}>
    VERBATIM)

  if(UNIX AND NOT APPLE)
    set_property(TARGET ${target} APPEND PROPERTY BUILD_RPATH "$ORIGIN")
  elseif(APPLE)
    set_property(TARGET ${target} APPEND PROPERTY BUILD_RPATH "@loader_path")
  endif()
endfunction()
