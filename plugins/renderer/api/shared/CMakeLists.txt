
	
set(DISABLE_FORCE_DEBUG_POSTFIX ON)
set(FT_DISABLE_ZLIB ON CACHE BOOL "" FORCE)
set(FT_DISABLE_BROTLI ON CACHE BOOL "" FORCE)
#comment definition of FT_CONFIG_OPTION_USE_ZLIB out in ftconfig.h
add_subdirectory(freetype-2.12.1)
set_target_properties(freetype PROPERTIES EXCLUDE_FROM_ALL TRUE)

add_notices(freetype 
	READABLE_NAME FreeType
	LICENSE_FILE freetype-2.12.1/docs/FTL.TXT 
	CREDITS "Font-Loading: FreeType - https://freetype.org/")


add_subdirectory(msdfgen)
set_target_properties(lib_msdfgen PROPERTIES EXCLUDE_FROM_ALL TRUE)

add_notices(lib_msdfgen 
	READABLE_NAME msdfgen 
	LICENSE_FILE msdfgen/LICENSE.txt 
	CREDITS "Font-Generation: Multi-channel signed distance field generator - https://github.com/Chlumsky/msdfgen")

