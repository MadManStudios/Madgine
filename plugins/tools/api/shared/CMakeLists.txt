
add_subdirectory(imgui)
set_target_properties(ImGui PROPERTIES EXCLUDE_FROM_ALL TRUE)
target_include_directories(ImGui PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)

if (MSVC)
	target_link_options(ImGui PUBLIC "/NATVIS:${CMAKE_CURRENT_LIST_DIR}/imgui/misc/debuggers/imgui.natvis")
endif ()

add_notices(ImGui 
	LICENSE_FILE imgui/LICENSE.txt 
	CREDITS "Tooling-UI: Dear ImGui - https://github.com/ocornut/imgui")

	
add_subdirectory(im3d)
set_target_properties(Im3D PROPERTIES EXCLUDE_FROM_ALL TRUE)


add_subdirectory(NodeEditor)

add_notices(imgui_node_editor 
	READABLE_NAME "ImGui Node Editor" 
	LICENSE_FILE NodeEditor/LICENSE 
	CREDITS "Nodegraph-Editor: Node Editor in ImGui - https://github.com/thedmd/imgui-node-editor")

