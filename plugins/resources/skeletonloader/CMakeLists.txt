
Project(SkeletonLoader)

set(sources "")
FolderSources(sources "" Madgine skeletonloaderlib.h)
	FolderSources(sources Madgine skeletonloader skeletondescriptor.h skeletonloader.cpp skeletonloader.h)


add_plugin(SkeletonLoader Madgine Resources ${sources} PRECOMPILED_HEADER Madgine/skeletonloaderlib.h SOURCE_ROOT .)

target_link_libraries(SkeletonLoader PRIVATE assimp)

target_link_plugins(SkeletonLoader Resources)