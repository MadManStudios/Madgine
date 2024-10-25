
Project(ImGui)

set(sources imgui.cpp imgui.h imguiaddons.cpp imguiaddons.h imgui_demo.cpp imgui_draw.cpp imgui_stacklayout.cpp imgui_tables.cpp imgui_widgets.cpp imgui_internal.h misc/freetype/imgui_freetype.cpp)
	
add_workspace_library(ImGui ${sources} SOURCE_ROOT ..)

target_link_libraries(ImGui PUBLIC Meta Modules)
target_link_libraries(ImGui PRIVATE freetype)

install_to_workspace(Madgine EXPORT_LIB TARGETS ImGui)
