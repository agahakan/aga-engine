add_library(aga_input src/plugin.cpp)
add_library(aga::input ALIAS aga_input)

target_include_directories(aga_input PUBLIC include)
target_link_libraries(aga_input PUBLIC aga::core aga::window ${AGA_GLFW_TARGET})
aga_target_warnings(aga_input)

