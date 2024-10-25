cmake_minimum_required (VERSION 3.8)

cmake_policy(VERSION 3.18)
cmake_policy(SET CMP0048 NEW)
cmake_policy(SET CMP0011 NEW)
cmake_policy(SET CMP0049 NEW)
cmake_policy(SET CMP0070 NEW)

if (NOT CXX_STANDARD_OVERRIDE)
	set(CMAKE_CXX_STANDARD 20 CACHE INTERNAL "")
else()
	set(CMAKE_CXX_STANDARD ${CXX_STANDARD_OVERRIDE} CACHE INTERNAL "")
endif()
set(CMAKE_CXX_STANDARD_REQUIRED ON CACHE INTERNAL "")
set(CMAKE_CXX_EXTENSIONS OFF CACHE INTERNAL "")


project(Madgine VERSION 0.1.0)


list(APPEND CMAKE_MODULE_PATH
	"${CMAKE_CURRENT_SOURCE_DIR}/cmake"
	"${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules"
)

if (MADGINE_AS_SDK)
	set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} PARENT_SCOPE)
endif(MADGINE_AS_SDK)

set (MADGINE_CONFIGURATION "" CACHE FILEPATH "A directory containing configuration data for a release build of the engine")

if (MADGINE_CONFIGURATION)
	if (NOT EXISTS ${MADGINE_CONFIGURATION})
		MESSAGE(FATAL_ERROR "The provided directory for MADGINE_CONFIGURATION does not exist: ${MADGINE_CONFIGURATION}")
	endif ()
	if (NOT IS_ABSOLUTE ${MADGINE_CONFIGURATION})
		set (MADGINE_CONFIGURATION ${CMAKE_SOURCE_DIR}/${MADGINE_CONFIGURATION} CACHE FILEPATH "A directory containing configuration data for a release build of the engine" FORCE)
	endif ()

	file(GLOB configs "${MADGINE_CONFIGURATION}/*.cmake")
	foreach(config ${configs})
		message(STATUS "Adding configuration-code: ${config}")
		include (${config})
	endforeach()

	include(util/ini)

	file(GLOB inis "${MADGINE_CONFIGURATION}/*.ini")
	foreach(ini ${inis})
		get_filename_component(name ${ini} NAME_WE)
		string(TOUPPER ${name} name)
		read_ini_file(${ini} ${name})
	endforeach()

endif()


include(Workspace)
include(Packaging)
include(PrecompiledHeader)
include(util/Project)
include(Plugins)
include(Tools)
include(Testing)
include(Legal)

set (CPACK_NSIS_MODIFY_PATH ON)
set (CPACK_PACKAGE_VENDOR "MadMan Studios")

#set (CPACK_PACKAGE_EXECUTABLES "Maditor;Maditor")

enable_packaging()


add_subdirectory(util ${CMAKE_BINARY_DIR}/../util)

add_subdirectory(src)

add_subdirectory(plugins)

add_subdirectory(samples)

if (MADGINE_EXTERNAL_FILE_HIERARCHY)
	add_subdirectory(${MADGINE_EXTERNAL_FILE_HIERARCHY} External)
endif()

set (LAUNCHER_LOCATION ${CMAKE_CURRENT_LIST_DIR}/launcher CACHE INTERNAL "")
function(create_launcher MADGINE_BINARY_NAME)
	if (TARGET MadgineTooling)
		set_target_properties(MadgineTooling PROPERTIES BinaryName ${MADGINE_BINARY_NAME})
	endif()
	set(MADGINE_SERVER OFF)
	add_subdirectory(${LAUNCHER_LOCATION} launcher)
endfunction()
function(create_server MADGINE_BINARY_NAME)
	if (TARGET MadgineTooling)
		set_target_properties(MadgineTooling PROPERTIES BinaryName ${MADGINE_BINARY_NAME})
	endif()
	set(MADGINE_SERVER ON)
	add_subdirectory(${LAUNCHER_LOCATION} server)
endfunction()

if (NOT MADGINE_AS_SDK)
	add_subdirectory(docs)
	create_launcher(MadgineLauncher)
	if (MODULES_ENABLE_PLUGINS OR PLUGINSELECTION_Core_Server)		
		create_server(MadgineServer)
	endif()
endif()


if (BUILD_TESTING)
	add_subdirectory(test)
endif()


export_to_workspace(Madgine)

cpack_add_component(Madgine REQUIRED)

cpack_add_component_group(Renderer EXPANDED)
