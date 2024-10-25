push_static()


add_definitions(-DPA_ENABLE_MSVC_DEBUG_OUTPUT)
set(PA_ENABLE_DEBUG_OUTPUT ON CACHE BOOL "" FORCE)
set(PA_BUILD_STATIC ON CACHE BOOL "" FORCE)
set(PA_BUILD_SHARED OFF CACHE BOOL "" FORCE)
add_subdirectory(portaudio-19.7.0)

add_notices(portaudio_static 
		READABLE_NAME PortAudio
		LICENSE_FILE portaudio-19.7.0/LICENSE.txt 
		CREDITS "Audio-IO: PortAudio - https://www.portaudio.com/")


pop_static()