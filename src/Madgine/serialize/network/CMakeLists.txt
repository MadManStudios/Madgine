Project(NetworkSerialize)

set(sources networkbuffer.cpp networkbuffer.h networkmanager.cpp networkmanager.h)
	
add_workspace_library(NetworkSerialize ${sources} SOURCE_ROOT ../../..)

target_link_libraries(NetworkSerialize PUBLIC Meta Platform)

if (BUILD_SHARED_LIBS)
	install_to_workspace(NetworkSerialize TARGETS NetworkSerialize)
endif()

if (BUILD_TESTING)
	add_subdirectory(test)
endif()