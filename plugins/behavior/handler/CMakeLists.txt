Project(Handler)

set(sources "")
FolderSources(sources "" src)
	FolderSources(sources src Madgine handler.cpp handler.h handlermanager.cpp handlermanager.h)
		
add_plugin(Handler Madgine Behavior ${sources} PRECOMPILED_HEADER src/Madgine/handlerlib.h)

target_link_plugins(Handler Behavior Client App)

