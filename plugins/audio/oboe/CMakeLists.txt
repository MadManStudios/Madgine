Project(OboeApi)

add_subdirectory(shared)


set(sources "")
FolderSources(sources "" src)
FolderSources(sources src Madgine)
	FolderSources(sources Madgine audio oboeapi.cpp oboeapi.h)

	
if (NOT TARGET oboe)
	set(stub STUB)
endif()


add_plugin(OboeApi Madgine Audio ${sources} PRECOMPILED_HEADER src/Madgine/oboeapilib.h ${stub} 
	UNIQUE_COMPONENTS 
		VIRTUAL Engine::Audio::AudioApi Engine::Root::RootComponent Engine::Audio::OboeApi)

target_link_plugins(OboeApi Audio)

if (TARGET oboe)
	target_link_libraries(OboeApi PRIVATE oboe)
endif()

