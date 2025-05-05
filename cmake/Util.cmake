
if (NOT COMMAND include_guard)

	macro(include_guard)

		set(__filename "${CMAKE_CURRENT_LIST_FILE}")
		get_property(already_included GLOBAL PROPERTY "pr_${__filename}")
		if(already_included)
			return()
		endif()
		set_property(GLOBAL PROPERTY "pr_${__filename}" TRUE)

	endmacro(include_guard)

endif()

if (NOT COMMAND once)

	macro(once)
		include_guard(GLOBAL)
	endmacro(once)

endif()

once()

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
		set(HOST_EXECUTABLE_SUFFIX ".exe")
    else()
		set(HOST_EXECUTABLE_SUFFIX "")
    endif()
else()
    set(HOST_EXECUTABLE_SUFFIX ${CMAKE_EXECUTABLE_SUFFIX})
endif()

#set (CMAKE_CXX_VISIBILITY_PRESET hidden)
#set (CMAKE_C_VISIBILITY_PRESET hidden)

if (ANDROID)
	set(CMAKE_POSITION_INDEPENDENT_CODE ON)
else()
	set(CMAKE_POSITION_INDEPENDENT_CODE ${BUILD_SHARED_LIBS})
endif()

if (GCC OR CLANG)
	add_compile_options(
		$<$<COMPILE_LANGUAGE:CXX,C>:-Wno-extra-qualification>
		$<$<COMPILE_LANGUAGE:CXX,C>:-Wno-instantiation-after-specialization>
		$<$<COMPILE_LANGUAGE:CXX,C>:-Wno-dll-attribute-on-redeclaration> 
		$<$<COMPILE_LANGUAGE:CXX,C>:-Wno-pragma-pack> 
		$<$<COMPILE_LANGUAGE:CXX,C>:-Wno-undefined-var-template>)	
	if (CLANG AND CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 15)
		add_compile_options($<$<COMPILE_LANGUAGE:CXX,C>:-Wno-deprecated-non-prototype>)
	endif ()
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

	if (NOT CLANG)
		add_compile_options($<$<COMPILE_LANGUAGE:C,CXX>:/Zc:preprocessor>)
	endif()
 
	# Set compiler options.
	set(variables
		CMAKE_C_FLAGS
		CMAKE_C_FLAGS_DEBUG
		CMAKE_C_FLAGS_MINSIZEREL
		CMAKE_C_FLAGS_RELEASE
		CMAKE_C_FLAGS_RELWITHDEBINFO
		CMAKE_CXX_FLAGS
		CMAKE_CXX_FLAGS_DEBUG
		CMAKE_CXX_FLAGS_MINSIZEREL
		CMAKE_CXX_FLAGS_RELEASE
		CMAKE_CXX_FLAGS_RELWITHDEBINFO
	)
	if(NOT BUILD_SHARED_LIBS)
		message(STATUS
		"MSVC -> forcing use of statically-linked runtime."
		)
		foreach(variable ${variables})
			if(${variable} MATCHES "/MD")
				string(REGEX REPLACE "/MD" "/MT" ${variable} "${${variable}}")
				set(${variable} "${${variable}}" CACHE INTERNAL "")
			endif()
		endforeach()
	else()
		message(STATUS
		"MSVC -> forcing use of dynamically-linked runtime."
		)
		foreach(variable ${variables})
			if(${variable} MATCHES "/MT")
				string(REGEX REPLACE "/MT" "/MD" ${variable} "${${variable}}")
				set(${variable} "${${variable}}" CACHE INTERNAL "")
			endif()
		endforeach()
	endif()
endif()

if (OSX)
    # Fix linking on 10.14+. See https://stackoverflow.com/questions/54068035
    LINK_DIRECTORIES(/usr/local/lib)
endif()