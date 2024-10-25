Project(Im3D)

set(sources im3d.cpp im3d.h im3d_internal.h)
	
add_workspace_library(Im3D ${sources} SOURCE_ROOT ..)

target_include_directories(Im3D PUBLIC $<TARGET_PROPERTY:FontLoader,INTERFACE_INCLUDE_DIRECTORIES>)

target_link_libraries(Im3D PUBLIC ImGui)

install_to_workspace(Madgine EXPORT_LIB TARGETS Im3D)