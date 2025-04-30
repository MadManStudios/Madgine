Project(Tools)

add_subdirectory(shared)

set(sources "")
FolderSources(sources "" src ) 
	FolderSources(sources src Madgine_Tools toolbase.cpp toolbase.h toolscollector.cpp toolscollector.h)
		FolderSources(sources Madgine_Tools filesystem filebrowser.cpp filebrowser.h)
		FolderSources(sources Madgine_Tools inspector functiontool.cpp functiontool.h inspector.cpp inspector.h)
		FolderSources(sources Madgine_Tools logviewer logviewer.cpp logviewer.h)
		FolderSources(sources Madgine_Tools memory memoryviewer.cpp memoryviewer.h)
		FolderSources(sources Madgine_Tools messagelogger messagelogger.cpp messagelogger.h)
		FolderSources(sources Madgine_Tools metrics metrics.cpp metrics.h)
		FolderSources(sources Madgine_Tools pluginmanager pluginmanager.cpp pluginmanager.h)
		FolderSources(sources Madgine_Tools profiler profiler.cpp profiler.h)
		FolderSources(sources Madgine_Tools renderer dialogs.cpp dialogs.h imguidemo.cpp imguidemo.h imroot.cpp imroot.h)
if (MODULES_ENABLE_TASK_TRACKING)
		FolderSources(sources Madgine_Tools tasktracker tasktracker.cpp tasktracker.h)
endif()
		FolderSources(sources Madgine_Tools testtool testtool.cpp testtool.h)
	

add_plugin(Tools Madgine Core ${sources} PRECOMPILED_HEADER src/Madgine_Tools/toolslib.h API_PLUGIN)

target_link_plugins(Tools)

target_link_libraries(Tools PUBLIC ImGui Im3D CodeGen MadgineCurl)


