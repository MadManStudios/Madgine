Project(Server)

set(sources "")
FolderSources(sources "" src)
	FolderSources(sources src Madgine serverforward.h serverlib.h)
		FolderSources(sources Madgine server server.cpp server.h serverapibase.cpp serverapibase.h serverapicollector.cpp serverapicollector.h serverinstance.cpp serverinstance.h serverlog.cpp serverlog.h)

add_plugin(Server Madgine Core ${sources} PRECOMPILED_HEADER src/Madgine/serverlib.h)

target_link_libraries(Server PUBLIC Modules Meta)

