
PROJECT(ShaderGen)

if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  add_compile_options(-fms-extensions -Wno-language-extension-token)
endif()

set(CMAKE_CXX_STANDARD 14)


set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/.. CACHE INTERNAL "")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/.. CACHE INTERNAL "")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/.. CACHE INTERNAL "")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO ${CMAKE_BINARY_DIR}/.. CACHE INTERNAL "")


set (HLSL_BUILD_DXILCONV OFF CACHE BOOL "" FORCE)
set (HLSL_COPY_GENERATED_SOURCES ON CACHE BOOL "" FORCE)
set (HLSL_EMBED_VERSION ON CACHE BOOL "" FORCE)

include(ext/DirectXShaderCompiler/cmake/caches/PredefinedParams.cmake)

add_subdirectory(ext/DirectXShaderCompiler)

set(SPIRV_CROSS_CLI OFF CACHE BOOL "" FORCE)

add_subdirectory(ext/SPIRV-Cross)


add_executable(ShaderGen main.cpp shadergenGLSL.cpp shadergenGLSLES.cpp shadergenHLSL.cpp shadergenSPIRV.cpp)

target_link_libraries(ShaderGen PRIVATE dxcompiler spirv-cross-glsl spirv-cross-hlsl spirv-cross-core)

target_include_directories(ShaderGen PRIVATE ext/DirectXShaderCompiler/include)