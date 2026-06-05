Project(FilesystemSerialize)

set(sources filemanager.cpp filemanager.h)
	
add_workspace_library(FilesystemSerialize ${sources} SOURCE_ROOT ../../..)

target_link_libraries(FilesystemSerialize PUBLIC Meta Platform)

install_to_workspace(FilesystemSerialize TARGETS FilesystemSerialize)