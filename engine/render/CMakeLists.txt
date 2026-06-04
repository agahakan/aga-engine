add_library(aga_render src/mesh.cpp src/plugin.cpp src/wgpu_renderer.cpp)
add_library(aga::render ALIAS aga_render)

target_include_directories(aga_render PUBLIC include)
target_link_libraries(
  aga_render
  PUBLIC aga::core
         aga::window
         aga::transform
         aga::camera
         ${AGA_GLM_TARGET}
         aga::wgpu)

if(UNIX AND NOT APPLE AND NOT EMSCRIPTEN)
  find_package(Vulkan REQUIRED)
  target_compile_definitions(aga_render PRIVATE GLFW_EXPOSE_NATIVE_X11)
  target_link_libraries(aga_render PUBLIC Vulkan::Vulkan)
endif()

aga_target_warnings(aga_render)
