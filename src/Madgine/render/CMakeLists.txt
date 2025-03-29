Project(Render)
	
add_workspace_interface_library(RenderLib SOURCE_ROOT ../..)

install_interface_to_workspace(Madgine TARGETS RenderLib SOURCE_ROOT ../..)