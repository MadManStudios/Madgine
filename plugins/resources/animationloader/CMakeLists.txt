
Project(AnimationLoader)

set(sources "")
FolderSources(sources "" Madgine animationloaderlib.h)
	FolderSources(sources Madgine animationloader animationdescriptor.h animationloader.cpp animationloader.h)



add_plugin(AnimationLoader Madgine Resources ${sources} PRECOMPILED_HEADER Madgine/animationloaderlib.h SOURCE_ROOT .)

target_link_libraries(AnimationLoader PRIVATE assimp)

target_link_plugins(AnimationLoader SkeletonLoader)