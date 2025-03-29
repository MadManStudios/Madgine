Project(CodeGen)

set(sources codegen.cpp codegen.h codegen_cmake.cpp codegen_cmake.h codegen_cpp.cpp codegen_cpp.h codegen_internal.h codegen_shader.cpp codegen_shader.h statement.cpp statement.h)
	
add_workspace_library(CodeGen ${sources} SOURCE_ROOT ../..)

target_link_libraries(CodeGen PUBLIC Meta)

install_to_workspace(Madgine EXPORT_LIB TARGETS CodeGen)