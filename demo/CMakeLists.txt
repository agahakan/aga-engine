add_executable(aga_demo src/demo_module.cpp src/main.cpp)
target_link_libraries(aga_demo PRIVATE aga::engine)
aga_target_warnings(aga_demo)
aga_copy_wgpu_runtime(aga_demo)

if(EMSCRIPTEN)
  set_target_properties(aga_demo PROPERTIES SUFFIX ".html")
  target_link_options(aga_demo PRIVATE "--shell-file=${CMAKE_CURRENT_SOURCE_DIR}/web/index.html")
endif()

