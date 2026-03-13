add_library(aga_engine INTERFACE)
add_library(aga::engine ALIAS aga_engine)

target_include_directories(aga_engine INTERFACE include)
target_link_libraries(
  aga_engine
  INTERFACE aga::core
            aga::window
            aga::input
            aga::transform
            aga::camera
            aga::render)

