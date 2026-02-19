include_guard(DIRECTORY)



option(BUILD_SHARED_LIBS "Build shared libraries (.dll/.so) instead of static ones (.lib/.a)" ON)



macro(cmake_log)
	if (USE_CMAKE_LOG)
		MESSAGE(STATUS "cmake diagnostics: " ${ARGN})
	endif()
endmacro(cmake_log)


if (CMAKE_BUILD_TYPE STREQUAL "")
	message (FATAL_ERROR "No Build Type Specified!")
endif()

if (WIN32)
	set (WINDOWS 1 CACHE INTERNAL "")
	cmake_log("Build Platform Windows")
endif()

if (CMAKE_ANDROID_ARCH_ABI)
	set (ANDROID 1 CACHE INTERNAL "")
	cmake_log("Build Platform Android")
endif()

if ("${CMAKE_SYSTEM_NAME}" STREQUAL "Linux")
	set (LINUX 1 CACHE INTERNAL "")
	cmake_log("Build Platform Linux")
endif()

if ("${CMAKE_SYSTEM_NAME}" STREQUAL "Darwin")
	set (OSX 1 CACHE INTERNAL "")
	cmake_log("Build Platform OSX")
endif() 
  
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
	set(GCC 1 CACHE INTERNAL "")
	cmake_log("Build Compiler Gcc")
endif()

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
	set(CLANG 1 CACHE INTERNAL "")
	cmake_log("Build Compiler Clang")
endif()

if (CMAKE_CROSSCOMPILING)
    if (CMAKE_HOST_WIN32)
		set(HOST_EXECUTABLE_SUFFIX ".exe" CACHE INTERNAL "")
    else()
		set(HOST_EXECUTABLE_SUFFIX "" CACHE INTERNAL "")
    endif()
else()
    set(HOST_EXECUTABLE_SUFFIX ${CMAKE_EXECUTABLE_SUFFIX} CACHE INTERNAL "")
endif()

#set (CMAKE_CXX_VISIBILITY_PRESET hidden)
#set (CMAKE_C_VISIBILITY_PRESET hidden)


if (GCC OR CLANG)
	add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-Wno-invalid-offsetof>)

	if (CLANG)
		add_compile_options(
			$<$<COMPILE_LANGUAGE:CXX,C>:-Wno-extra-qualification>
			$<$<COMPILE_LANGUAGE:CXX,C>:-Wno-instantiation-after-specialization>
			$<$<COMPILE_LANGUAGE:CXX,C>:-Wno-dll-attribute-on-redeclaration> 
			$<$<COMPILE_LANGUAGE:CXX,C>:-Wno-pragma-pack> 
			$<$<COMPILE_LANGUAGE:CXX,C>:-Wno-undefined-var-template>)	
		if (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 15)
			add_compile_options($<$<COMPILE_LANGUAGE:CXX,C>:-Wno-deprecated-non-prototype>)
		endif ()
	endif()
	
	if (NOT MSVC)
		add_compile_options(-Wall -fpermissive)
	else ()
		add_compile_options($<$<COMPILE_LANGUAGE:CXX,C>:-Wno-microsoft-cast>)
	endif()
	if (EMSCRIPTEN) #TODO add more
		set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--no-undefined")
		add_compile_options(-Wno-implicit-function-declaration)
	endif()
endif ()

if (MSVC)
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /ignore:4217")
	set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /ignore:4217")

	#add_compile_options(/JMC)

	if (NOT CLANG)
		add_compile_options($<$<COMPILE_LANGUAGE:C,CXX>:/Zc:preprocessor>)
	endif()

endif()

set(CMAKE_MACOSX_RPATH TRUE)
set(CMAKE_BUILD_RPATH_USE_ORIGIN TRUE)
if (OSX)
	# Why is this needed? Is it a bug?
	set(CMAKE_SHARED_LIBRARY_RPATH_ORIGIN_TOKEN "@executable_path")
	# Fix linking on 10.14+. See https://stackoverflow.com/questions/54068035
    LINK_DIRECTORIES(/opt/homebrew/lib)
endif()


set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO ${CMAKE_BINARY_DIR}/bin)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set(CMAKE_FOLDER "External")

if (NOT WIN32)
	set (outDir ${CMAKE_BINARY_DIR}/bin)

	set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${outDir})
	set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG ${outDir})
	set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE ${outDir})
	set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELWITHDEBINFO ${outDir})

	set(CMAKE_INSTALL_RPATH $ORIGIN/)
	set(CMAKE_BUILD_RPATH $ORIGIN/)

endif()


get_property(support_shared GLOBAL PROPERTY TARGET_SUPPORTS_SHARED_LIBS)

if (NOT support_shared)
	MESSAGE(STATUS "Forcing static libraries as shared libraries are not supported on that platform!")
	set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
endif()

if (ANDROID)
	set(CMAKE_POSITION_INDEPENDENT_CODE ON)
else()
	set(CMAKE_POSITION_INDEPENDENT_CODE ${BUILD_SHARED_LIBS})
endif()

if (NOT BUILD_SHARED_LIBS)
	MESSAGE(STATUS "Enabling STATIC_BUILD=1")
	add_definitions(-DSTATIC_BUILD=1)
endif()

add_definitions(-DBINARY_DIR="${CMAKE_BINARY_DIR}")
add_definitions(-DSOURCE_DIR="${CMAKE_SOURCE_DIR}")


SET(CMAKE_DEBUG_POSTFIX "" CACHE STRING "" FORCE) #Some libs set this value
if (BUILD_SHARED_LIBS)
	set(CMAKE_MSVC_RUNTIME_LIBRARY MultiThreaded$<$<CONFIG:Debug>:Debug>DLL CACHE INTERNAL "")
else()
	set(CMAKE_MSVC_RUNTIME_LIBRARY MultiThreaded$<$<CONFIG:Debug>:Debug> CACHE INTERNAL "")
endif()
