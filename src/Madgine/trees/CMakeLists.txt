
set(sources treeformat.cpp treeformat.h)
	
add_workspace_library(Trees ${sources} SOURCE_ROOT ../..)

target_link_libraries(Trees PUBLIC Generic)

install_to_workspace(Trees TARGETS Trees)