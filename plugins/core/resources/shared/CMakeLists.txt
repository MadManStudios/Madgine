
set(ASSIMP_BUILD_ALL_IMPORTERS_BY_DEFAULT OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_ALL_EXPORTERS_BY_DEFAULT OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_FBX_IMPORTER ON)
set(ASSIMP_BUILD_COLLADA_IMPORTER ON)
set(ASSIMP_BUILD_STL_IMPORTER ON)
set(ASSIMP_INSTALL OFF CACHE BOOL "" FORCE)
set(ASSIMP_WARNINGS_AS_ERRORS OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_TESTS OFF CACHE BOOL "" FORCE)
add_subdirectory(assimp-5.2.5)

add_notices(assimp 
	READABLE_NAME "Asset Importer" 
	LICENSE_FILE assimp-5.2.5/LICENSE 
	CREDITS "Model-Loading: Open Asset Import Library (assimp) - https://github.com/assimp/assimp")

	
set(DISABLE_FORCE_DEBUG_POSTFIX ON)
set(FT_DISABLE_ZLIB ON CACHE BOOL "" FORCE)
#comment definition of FT_CONFIG_OPTION_USE_ZLIB out in ftconfig.h
add_subdirectory(freetype-2.12.1)
set_target_properties(freetype PROPERTIES EXCLUDE_FROM_ALL TRUE)

add_notices(freetype 
	READABLE_NAME FreeType
	LICENSE_FILE freetype-2.12.1/docs/FTL.txt 
	CREDITS "Font-Loading: FreeType - https://freetype.org/")


add_subdirectory(msdfgen)
set_target_properties(lib_msdfgen PROPERTIES EXCLUDE_FROM_ALL TRUE)

add_notices(lib_msdfgen 
	READABLE_NAME msdfgen 
	LICENSE_FILE msdfgen/LICENSE.txt 
	CREDITS "Font-Generation: Multi-channel signed distance field generator - https://github.com/Chlumsky/msdfgen")


push_static()

if (NOT IOS AND NOT EMSCRIPTEN)
	add_subdirectory(libogg-1.3.4)

	add_library(Ogg::ogg ALIAS ogg)

	add_notices(ogg 
		READABLE_NAME Ogg 
		LICENSE_FILE libogg-1.3.4/COPYING 
		CREDITS "Multimedia-Format: Ogg - https://xiph.org/ogg/")


	add_subdirectory(libvorbis-1.3.7)

	add_notices(vorbis 
		READABLE_NAME Vorbis 
		LICENSE_FILE libvorbis-1.3.7/COPYING 
		CREDITS "Audio-Compression: Vorbis - https://xiph.org/vorbis/")
endif()

pop_static()
