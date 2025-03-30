Project(Curl)

set(BUILD_CURL_EXE OFF CACHE BOOL "" FORCE)
set(CURL_USE_LIBPSL OFF CACHE BOOL "" FORCE)
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(CURL_USE_SCHANNEL ${WIN32} CACHE BOOL "" FORCE)
set(CURL_USE_SECTRANSP ${APPLE} CACHE BOOL "" FORCE)
add_subdirectory(curl-8.12.1)

add_notices(libcurl
	READABLE_NAME "libcurl"
	LICENSE_FILE curl-8.12.1/COPYING
	CREDITS "client-side URL transfer library - https://curl.se/libcurl/")

	
set(sources curl.cpp curl.h)
	
add_workspace_library(MadgineCurl ${sources} SOURCE_ROOT ../..)

target_link_libraries(MadgineCurl PRIVATE libcurl)
target_link_libraries(MadgineCurl PUBLIC Generic)

install_to_workspace(Madgine EXPORT_LIB TARGETS MadgineCurl)