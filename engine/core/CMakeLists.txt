add_library(aga_core src/app.cpp)
add_library(aga::core ALIAS aga_core)

target_include_directories(aga_core PUBLIC include)
target_link_libraries(aga_core PUBLIC aga::options ${AGA_FLECS_TARGET})
aga_target_warnings(aga_core)

