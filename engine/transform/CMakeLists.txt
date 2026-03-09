add_library(aga_transform src/transform.cpp)
add_library(aga::transform ALIAS aga_transform)

target_include_directories(aga_transform PUBLIC include)
target_link_libraries(aga_transform PUBLIC aga::core ${AGA_GLM_TARGET})
aga_target_warnings(aga_transform)

