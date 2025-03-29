Project(CLI)

set(sources cli.cpp cli.h parameter.h parameterbase.cpp parameterbase.h)
	
add_workspace_library(CLI ${sources} SOURCE_ROOT ../..)

target_link_libraries(CLI PUBLIC Interfaces)

install_to_workspace(Madgine EXPORT_LIB TARGETS CLI)