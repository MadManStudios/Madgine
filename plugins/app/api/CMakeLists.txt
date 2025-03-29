Project(App)

set(sources "")
FolderSources(sources "" src)
	FolderSources(sources src Madgine appforward.h applib.h)
		FolderSources(sources Madgine app application.cpp application.h globalapibase.cpp globalapibase.h globalapicollector.cpp globalapicollector.h)

add_plugin(App Madgine Core ${sources} PRECOMPILED_HEADER src/Madgine/applib.h)

target_link_libraries(App PUBLIC Meta Modules)

target_link_plugins(App Debugger)

