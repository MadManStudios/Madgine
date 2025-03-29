Project(PortAudioApi)

add_subdirectory(shared)

if (NOT TARGET portaudio_static)
	return()
endif()


set(sources "")
FolderSources(sources "" src)
FolderSources(sources src Madgine)
	FolderSources(sources Madgine audio portaudioapi.cpp portaudioapi.h)

add_plugin(PortAudioApi Madgine Audio ${sources} PRECOMPILED_HEADER src/Madgine/portaudioapilib.h)

target_link_plugins(PortAudioApi Audio)

target_link_libraries(PortAudioApi PRIVATE portaudio_static)

