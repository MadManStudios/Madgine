Project(Ini)

set(sources inifile.cpp inifile.h inisection.cpp inisection.h)
	
add_workspace_library(Ini ${sources} SOURCE_ROOT ..)

target_link_libraries(Ini PUBLIC Interfaces)

install_to_workspace(Ini TARGETS Ini)