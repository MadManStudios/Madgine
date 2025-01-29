include(util/config)

Project(Client)     

set(sources "")
FolderSources(sources "" src)
	FolderSources(sources src Madgine clientforward.h clientlib.h)
		FolderSources(sources Madgine window layoutloader.cpp layoutloader.h mainwindow.cpp mainwindow.h mainwindowcomponent.cpp mainwindowcomponent.h mainwindowcomponentcollector.cpp mainwindowcomponentcollector.h toolwindow.cpp toolwindow.h)

add_plugin(Client Madgine Core ${sources} PRECOMPILED_HEADER src/Madgine/clientlib.h)

target_link_plugins(Client Render)

target_link_libraries(Client PUBLIC Meta Modules)

set_config(MADGINE_MAINWINDOW_LAYOUT CLIENT LAYOUT)

configure_file(src/Madgine/clientconfig.in.h clientconfig.h)
target_include_directories(Client PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>)



set(sources "")
FolderSources(sources "" tools)
	FolderSources(sources tools Madgine_Tools)
		FolderSources(sources Madgine_Tools gamepad gamepadviewer.cpp gamepadviewer.h)
		FolderSources(sources Madgine_Tools im3d im3drenderpass.cpp im3drenderpass.h)
			FolderSources(sources im3d shaders im3d.PS_hlsl im3d.VS_hlsl im3d.sl)
		FolderSources(sources Madgine_Tools imgui clientimroot.cpp clientimroot.h projectmanager.cpp projectmanager.h)
		FolderSources(sources Madgine_Tools shaders imgui.PS_hlsl imgui.VS_hlsl)

add_tools(Client ${sources} PRECOMPILED_HEADER tools/Madgine_Tools/clienttoolslib.h)

target_link_plugins(ClientTools Tools)

