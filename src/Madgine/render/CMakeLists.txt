Project(Render)
	
add_workspace_interface_library(Render SOURCE_ROOT ../..)

install_interface_to_workspace(Madgine TARGETS Render SOURCE_ROOT ../..)