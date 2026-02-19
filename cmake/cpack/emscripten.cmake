
set(CPACK_GENERATOR External)
set(CPACK_EXTERNAL_ENABLE_STAGING ON)
configure_file(${workspace_file_dir}/cpack/emscripten/cpack.cmake.in cpack.cmake.in @ONLY)
file(GENERATE OUTPUT cpack.cmake INPUT ${CMAKE_CURRENT_BINARY_DIR}/cpack.cmake.in)
set(CPACK_EXTERNAL_PACKAGE_SCRIPT ${CMAKE_CURRENT_BINARY_DIR}/cpack.cmake )