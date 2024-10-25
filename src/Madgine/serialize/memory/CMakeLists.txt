Project(MemorySerialize)

set(sources memorybuffer.cpp memorybuffer.h memorymanager.cpp memorymanager.h)
	
add_workspace_library(MemorySerialize ${sources} SOURCE_ROOT ../../..)

target_link_libraries(MemorySerialize PUBLIC Meta)

install_to_workspace(Madgine EXPORT_LIB TARGETS MemorySerialize)
