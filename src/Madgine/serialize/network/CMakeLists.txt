Project(NetworkSerialize)

set(sources networkbuffer.cpp networkbuffer.h networkmanager.cpp networkmanager.h)
	
add_workspace_library(NetworkSerialize ${sources} SOURCE_ROOT ../../..)

target_link_libraries(NetworkSerialize PUBLIC Meta Interfaces)

install_to_workspace(Madgine EXPORT_LIB TARGETS NetworkSerialize)

if (BUILD_TESTING)
	add_subdirectory(test)
endif()