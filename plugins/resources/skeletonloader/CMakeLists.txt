
Project(SkeletonLoader)

set(sources "")
FolderSources(sources "" Madgine skeletonloaderlib.h)
	FolderSources(sources Madgine skeletonloader skeletondescriptor.h skeletonloader.cpp skeletonloader.h)


add_plugin(SkeletonLoader Madgine Utility ${sources} PRECOMPILED_HEADER Madgine/skeletonloaderlib.h SOURCE_ROOT . EXTERNAL_DEPS assimp)

target_link_libraries(SkeletonLoader PRIVATE assimp)

target_link_plugins(SkeletonLoader Resources)