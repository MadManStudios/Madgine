Project(Curl)

set(BUILD_CURL_EXE OFF CACHE BOOL "" FORCE)
set(CURL_USE_LIBPSL OFF CACHE BOOL "" FORCE)
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(CURL_USE_SCHANNEL ON CACHE BOOL "" FORCE)
add_subdirectory(curl-8.12.1)

add_notices(libcurl_shared
	READABLE_NAME "libcurl"
	LICENSE_FILE curl-8.12.1/COPYING
	CREDITS "client-side URL transfer library - https://curl.se/libcurl/")

	
set(sources curl.cpp curl.h)
	
add_workspace_library(Curl ${sources} SOURCE_ROOT ../..)

target_link_libraries(Curl PRIVATE libcurl)
target_link_libraries(Curl PUBLIC Generic)

install_to_workspace(Madgine EXPORT_LIB TARGETS Curl)