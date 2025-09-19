
Project(MeshLoader)

set(sources "")
FolderSources(sources "" src)
	FolderSources(sources src Madgine meshloaderlib.h)
		FolderSources(sources Madgine meshloader gpumeshdata.h gpumeshloader.cpp gpumeshloader.h meshdata.cpp meshdata.h meshloader.cpp meshloader.h)


add_plugin(MeshLoader Madgine Resources ${sources} PRECOMPILED_HEADER src/Madgine/meshloaderlib.h)

target_link_libraries(MeshLoader PRIVATE assimp)

target_link_plugins(MeshLoader Resources)