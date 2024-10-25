Project(OboeApi)

if (NOT ANDROID)
	return()
endif()

add_subdirectory(shared)

if (NOT TARGET oboe)
	return()
endif()


set(sources "")
FolderSources(sources "" src)
FolderSources(sources src Madgine)
	FolderSources(sources Madgine audio oboeapi.cpp oboeapi.h)

add_plugin(OboeApi Madgine Audio ${sources} PRECOMPILED_HEADER src/Madgine/oboeapilib.h)

target_link_plugins(OboeApi Audio)

target_link_libraries(OboeApi PRIVATE oboe)

