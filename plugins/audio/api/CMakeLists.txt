Project(Audio)

set(sources "")
FolderSources(sources "" src)
	FolderSources(sources src Madgine audioforward.h audiolib.h)
		FolderSources(sources Madgine audio audioapi.cpp audioapi.h)

add_plugin(Audio Madgine API ${sources} PRECOMPILED_HEADER src/Madgine/audiolib.h)

plugin_group(Audio EXCLUSIVE)

target_link_plugins(Audio AudioLoader Behavior)

