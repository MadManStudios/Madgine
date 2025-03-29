
Project(MeshLoader)

set(sources "")
FolderSources(sources "" Madgine meshloaderlib.h)
	FolderSources(sources Madgine meshloader gpumeshdata.h gpumeshloader.cpp gpumeshloader.h meshdata.cpp meshdata.h meshloader.cpp meshloader.h)


add_plugin(MeshLoader Madgine Utility ${sources} PRECOMPILED_HEADER Madgine/meshloaderlib.h SOURCE_ROOT . EXTERNAL_DEPS assimp)

target_link_libraries(MeshLoader PRIVATE assimp)

target_link_plugins(MeshLoader Resources)