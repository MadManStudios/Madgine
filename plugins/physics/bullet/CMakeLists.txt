
Project(Bullet)

add_subdirectory(shared)

set(sources "")
FolderSources(sources "" src)
	FolderSources(sources src Madgine)
		FolderSources(sources Madgine physics collisionshapemanager.cpp collisionshapemanager.h physicsmanager.cpp physicsmanager.h rigidbody.cpp rigidbody.h rigidbodydata.cpp rigidbodydata.h softbodyskeleton.cpp softbodyskeleton.h)


add_plugin(Bullet Madgine Physics ${sources} PRECOMPILED_HEADER src/Madgine/bulletlib.h)

target_link_plugins(Bullet Scene)

target_link_libraries(Bullet PRIVATE BulletDynamics BulletCollision BulletSoftBody LinearMath)

set(sources "")
FolderSources(sources "" tools)
	FolderSources(sources tools Madgine_Tools)
		FolderSources(sources Madgine_Tools bulletdebug bulletdebug.cpp bulletdebug.h)

add_tools(Bullet ${sources} PRECOMPILED_HEADER tools/Madgine_Tools/bullettoolslib.h)

target_link_plugins(BulletTools Bullet ClientTools)
target_link_libraries(BulletTools PRIVATE BulletCollision LinearMath)

if (BUILD_TESTING)
	add_subdirectory(test)
endif()