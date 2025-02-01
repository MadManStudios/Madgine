Project(Audio)

add_subdirectory(shared)

set(sources "")
FolderSources(sources "" src)
	FolderSources(sources src Madgine audioforward.h audiolib.h)
		FolderSources(sources Madgine audio audioapi.cpp audioapi.h audiobuffer.h audioloader.cpp audioloader.h oggdecode.cpp oggdecode.h oggdecodebuf.cpp oggdecodebuf.h)

add_plugin(Audio Madgine API ${sources} PRECOMPILED_HEADER src/Madgine/audiolib.h)

plugin_group(Audio EXCLUSIVE)

target_link_plugins(Audio Behavior Resources)

target_link_libraries(Audio PUBLIC vorbisfile)

