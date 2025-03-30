
push_static()

if (NOT IOS AND NOT EMSCRIPTEN)
	add_subdirectory(libogg-1.3.5)

	#add_library(Ogg::ogg ALIAS ogg)

	add_notices(ogg 
		READABLE_NAME Ogg 
		LICENSE_FILE libogg-1.3.5/COPYING 
		CREDITS "Multimedia-Format: Ogg - https://xiph.org/ogg/")


	add_subdirectory(libvorbis-1.3.7)

	add_notices(vorbis 
		READABLE_NAME Vorbis 
		LICENSE_FILE libvorbis-1.3.7/COPYING 
		CREDITS "Audio-Compression: Vorbis - https://xiph.org/vorbis/")
endif()

pop_static()
