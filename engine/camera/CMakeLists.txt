add_library(aga_camera src/camera.cpp)
add_library(aga::camera ALIAS aga_camera)

target_include_directories(aga_camera PUBLIC include)
target_link_libraries(aga_camera PUBLIC aga::core aga::transform ${AGA_GLM_TARGET})
aga_target_warnings(aga_camera)

