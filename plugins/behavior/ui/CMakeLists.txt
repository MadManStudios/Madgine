

Project(UI)     

set(sources "")
FolderSources(sources "" src)
	FolderSources(sources src Madgine uiforward.h uilib.h)
		FolderSources(sources Madgine input handler.cpp handler.h uimanager.cpp uimanager.h widgethandler.cpp widgethandler.h)

add_plugin(UI Madgine Behavior ${sources} PRECOMPILED_HEADER src/Madgine/uilib.h)

target_link_plugins(UI Widgets App)


