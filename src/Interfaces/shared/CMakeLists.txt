
push_static()

if (NOT EMSCRIPTEN)

	set(BUILD_CURL_EXE OFF CACHE BOOL "" FORCE)
	set(CURL_USE_LIBPSL OFF CACHE BOOL "" FORCE)
	set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
	set(CURL_USE_SCHANNEL ${WIN32} CACHE BOOL "" FORCE)
	set(CURL_USE_SECTRANSP ${APPLE} CACHE BOOL "" FORCE)
	set(BUILD_STATIC_LIBS ON CACHE BOOL "" FORCE)
	set(CURL_DISABLE_INSTALL ON CACHE BOOL "" FORCE)
	add_subdirectory(curl-8.12.1)

	add_notices(libcurl_static
		READABLE_NAME "libcurl"
		LICENSE_FILE curl-8.12.1/COPYING
		CREDITS "client-side URL transfer library - https://curl.se/libcurl/")

endif()

pop_static()