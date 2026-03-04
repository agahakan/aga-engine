if(NOT DEFINED ENV{EMSDK})
  find_program(EMCMAKE emcmake)
  if(EMCMAKE)
    get_filename_component(EMSCRIPTEN_ROOT "${EMCMAKE}" DIRECTORY)
    get_filename_component(EMSCRIPTEN_ROOT "${EMSCRIPTEN_ROOT}" DIRECTORY)
  endif()
else()
  set(EMSCRIPTEN_ROOT "$ENV{EMSDK}/upstream/emscripten")
endif()

if(NOT EMSCRIPTEN_ROOT)
  message(FATAL_ERROR "emscripten not found; enter a shell with emscripten or set EMSDK")
endif()

set(AGA_EMSCRIPTEN_TOOLCHAIN_CANDIDATES
    "${EMSCRIPTEN_ROOT}/cmake/Modules/Platform/Emscripten.cmake"
    "${EMSCRIPTEN_ROOT}/share/emscripten/cmake/Modules/Platform/Emscripten.cmake")

foreach(candidate IN LISTS AGA_EMSCRIPTEN_TOOLCHAIN_CANDIDATES)
  if(EXISTS "${candidate}")
    include("${candidate}")
    return()
  endif()
endforeach()

message(FATAL_ERROR "could not find Emscripten.cmake under ${EMSCRIPTEN_ROOT}")
