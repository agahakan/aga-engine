add_library(aga_window src/plugin.cpp src/window.cpp)
add_library(aga::window ALIAS aga_window)

target_include_directories(aga_window PUBLIC include)
target_link_libraries(aga_window PUBLIC aga::core ${AGA_GLFW_TARGET})
target_compile_definitions(aga_window PUBLIC GLFW_INCLUDE_NONE)

if(UNIX AND NOT APPLE AND NOT EMSCRIPTEN)
  find_package(X11 REQUIRED)
  target_link_libraries(aga_window PUBLIC X11::X11)
  target_compile_definitions(aga_window PUBLIC GLFW_EXPOSE_NATIVE_X11)
endif()

aga_target_warnings(aga_window)
