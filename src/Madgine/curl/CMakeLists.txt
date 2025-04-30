Project(Curl)

add_subdirectory(shared)
	
set(sources curl.cpp curl.h)
	
add_workspace_library(MadgineCurl ${sources} SOURCE_ROOT ../..)

target_link_libraries(MadgineCurl PRIVATE libcurl_static)
target_link_libraries(MadgineCurl PUBLIC Generic)

install_to_workspace(Curl TARGETS MadgineCurl)