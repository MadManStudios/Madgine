project(imgui_node_editor)

set(_imgui_node_editor_Sources
    imgui_node_editor.h
    crude_json.cpp
    crude_json.h
    imgui_bezier_math.h
    imgui_bezier_math.inl
    imgui_canvas.cpp
    imgui_canvas.h
    imgui_extra_math.h
    imgui_extra_math.inl
    imgui_node_editor_api.cpp
    imgui_node_editor_internal.h
    imgui_node_editor_internal.inl
    imgui_node_editor.cpp
    misc/imgui_node_editor.natvis
)

add_workspace_library(${PROJECT_NAME} ${_imgui_node_editor_Sources} SOURCE_ROOT ..)

target_link_libraries(${PROJECT_NAME} PUBLIC ImGui)

target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

install_to_workspace(Madgine EXPORT_LIB TARGETS ${PROJECT_NAME})