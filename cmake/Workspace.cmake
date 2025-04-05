include(Util)

once()

cmake_policy(SET CMP0022 NEW)


include(CMakeDependentOption)
   
set(workspace_file_dir ${CMAKE_CURRENT_LIST_DIR} CACHE INTERNAL "")

set(WORKSPACE_HOOKS)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin CACHE INTERNAL "")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/bin CACHE INTERNAL "")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/bin CACHE INTERNAL "")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO ${CMAKE_BINARY_DIR}/bin CACHE INTERNAL "")

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

option(BUILD_SHARED_LIBS "Build shared libraries (.dll/.so) instead of static ones (.lib/.a)" ON)

if (NOT support_shared)
	MESSAGE(STATUS "Forcing static libraries as shared libraries are not supported on that platform!")
	set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
endif()

if (NOT BUILD_SHARED_LIBS)
	MESSAGE(STATUS "Enabling STATIC_BUILD=1")
	add_definitions(-DSTATIC_BUILD=1)
endif()

add_definitions(-DBINARY_DIR="${CMAKE_BINARY_DIR}")
add_definitions(-DSOURCE_DIR="${CMAKE_SOURCE_DIR}")

 
function(install_header name)    
	
	install(DIRECTORY src/ DESTINATION ${name}/include
      COMPONENT ${name}
	  FILES_MATCHING PATTERN "*.h"
    )	

endfunction(install_header)

function(install_to_workspace name)

	set(options EXPORT_LIB)
	set(oneValueArgs)
	set(multiValueArgs TARGETS)
	cmake_parse_arguments(OPTIONS "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

	if (OPTIONS_EXPORT_LIB)
		install(
			TARGETS ${OPTIONS_TARGETS}
			EXPORT ${name}
			RUNTIME DESTINATION bin COMPONENT ${name}
			LIBRARY DESTINATION lib COMPONENT ${name}
			BUNDLE DESTINATION App COMPONENT ${name}
			INCLUDES DESTINATION ${name}/include
			ARCHIVE DESTINATION ${name}/lib COMPONENT ${name}			
		)
	else()
		install(
			TARGETS ${OPTIONS_TARGETS} 
			EXPORT ${name}
			RUNTIME DESTINATION bin COMPONENT ${name}
			LIBRARY DESTINATION lib COMPONENT ${name}
			BUNDLE DESTINATION App COMPONENT ${name}
		)
	endif()

	foreach(target ${OPTIONS_TARGETS})
		get_target_property(TARGET_SOURCE_DIR ${target} SOURCE_DIR)
		if (EXISTS ${TARGET_SOURCE_DIR}/data)
			install(DIRECTORY ${TARGET_SOURCE_DIR}/data DESTINATION . COMPONENT ${name})
		endif()

		target_include_directories(${target} INTERFACE $<INSTALL_INTERFACE:$<INSTALL_PREFIX>/${target}/include>)
	endforeach()

endfunction(install_to_workspace)


function(export_to_workspace name)

	install_header(${name})

    install(
	    EXPORT ${name}
	    FILE ${name}Config.cmake
	    DESTINATION ${name}/lib/cmake/${name}
		COMPONENT ${name}
    )
    
endfunction(export_to_workspace)

#Customization-point for different platforms (e.g. Android)
macro(add_workspace_application name)

	add_executable(${name} ${ARGN})
	set_target_properties(${name} PROPERTIES FOLDER "Application") 

endmacro(add_workspace_application)

macro(add_workspace_executable name)

	add_executable(${name} ${ARGN})
	set_target_properties(${name} PROPERTIES FOLDER "Executable") 

endmacro(add_workspace_executable)

macro(add_workspace_library name)

	set(options)
	set(oneValueArgs SOURCE_ROOT PRECOMPILED_HEADER)
	set(multiValueArgs)
	cmake_parse_arguments(LIB_CONFIG "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})	
	
	add_library(${name} ${LIB_CONFIG_UNPARSED_ARGUMENTS})

	target_compile_definitions(${name} INTERFACE BUILD_${name})

	get_target_property(sourceDir ${name} SOURCE_DIR)

	if (NOT LIB_CONFIG_SOURCE_ROOT)
		set(LIB_CONFIG_SOURCE_ROOT src)
	endif()

	if (NOT EXISTS ${sourceDir}/${LIB_CONFIG_SOURCE_ROOT})
		MESSAGE(SEND_ERROR "Source directory '${sourceDir}/${LIB_CONFIG_SOURCE_ROOT}' does not exist for Library '${name}'. Use SOURCE_ROOT or create the directory.")
	endif()
	
	target_include_directories(${name} INTERFACE $<BUILD_INTERFACE:${sourceDir}/${LIB_CONFIG_SOURCE_ROOT}>)

	get_filename_component(abs_source_root ${LIB_CONFIG_SOURCE_ROOT} ABSOLUTE)
	set_target_properties(${name} PROPERTIES 
									SOURCE_ROOT ${abs_source_root}
									FOLDER "Library")

	if (LIB_CONFIG_PRECOMPILED_HEADER)
	
		get_filename_component(abs_precompile_include ${LIB_CONFIG_PRECOMPILED_HEADER} ABSOLUTE)

		cmake_log("Precompiling header ${abs_precompile_include} for target ${name}")

		target_precompile_headers(${name} PRIVATE $<$<COMPILE_LANGUAGE:CXX>:${abs_precompile_include}> )

		file(RELATIVE_PATH precompile_include ${abs_source_root} ${abs_precompile_include})

		set_target_properties(${name} PROPERTIES PRECOMPILED_HEADER ${precompile_include})

	endif()

	if (IWYU)
		set_property(TARGET ${name} PROPERTY CXX_INCLUDE_WHAT_YOU_USE
		"${IWYU};-Xiwyu;--pch_in_code;-Xiwyu;--prefix_header_includes=remove;-Xiwyu;--max_line_length=200;--driver-mode=cl")
	endif (IWYU)

endmacro(add_workspace_library)

macro(add_workspace_interface_library name)

	set(options)
	set(oneValueArgs SOURCE_ROOT PRECOMPILED_HEADER)
	set(multiValueArgs)
	cmake_parse_arguments(LIB_CONFIG "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})	
	
	add_library(${name} INTERFACE ${LIB_CONFIG_UNPARSED_ARGUMENTS})	

	set_target_properties(${name} PROPERTIES 
									FOLDER "Library")

	if (NOT LIB_CONFIG_SOURCE_ROOT)
		MESSAGE(SEND_ERROR "Source directory must always be set for Interface Library '${name}'. Use SOURCE_ROOT to specify it.")
	endif()
	
	target_include_directories(${name} INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/${LIB_CONFIG_SOURCE_ROOT}>)

endmacro(add_workspace_interface_library)


function(install_interface_to_workspace name)

	set(options)
	set(oneValueArgs SOURCE_ROOT)
	set(multiValueArgs TARGETS)
	cmake_parse_arguments(OPTIONS "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

		install(
			TARGETS ${OPTIONS_TARGETS} 
			EXPORT ${name}
		)

	if (NOT OPTIONS_SOURCE_ROOT)
		MESSAGE(SEND_ERROR "Source directory must be always set for Interface Library installs. Use SOURCE_ROOT to specify it.")
	endif()

	foreach(target ${OPTIONS_TARGETS})
		
		get_filename_component(TARGET_SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/${OPTIONS_SOURCE_ROOT} ABSOLUTE)

		if (EXISTS ${TARGET_SOURCE_DIR}/data)
			install(DIRECTORY ${TARGET_SOURCE_DIR}/data DESTINATION . COMPONENT ${name})
		endif()

		target_include_directories(${target} INTERFACE $<INSTALL_INTERFACE:$<INSTALL_PREFIX>/${target}/include>)
	endforeach()

endfunction(install_interface_to_workspace)

function(get_dependencies list target)

	list(FIND ${list} ${target} found_target)

	if (found_target EQUAL -1)
		list(APPEND ${list} ${target})

		get_target_property(dependencies ${target} MANUALLY_ADDED_DEPENDENCIES)

		if (dependencies)
			foreach(dep ${dependencies})
				get_dependencies(${list} ${dep})
			endforeach()
		endif()

		get_target_property(dependencies ${target} LINK_LIBRARIES)

		if (dependencies)
			foreach(dep ${dependencies})
				if (TARGET ${dep})
					get_dependencies(${list} ${dep})
				endif()
			endforeach()
		endif()

	endif()

	set(${list} ${${list}} PARENT_SCOPE)

endfunction(get_dependencies)


SET(CMAKE_DEBUG_POSTFIX "" CACHE STRING "" FORCE) #Some libs set this value
set(USE_MSVC_RUNTIME_LIBRARY_DLL ${BUILD_SHARED_LIBS} CACHE BOOL "" FORCE)

macro(push_static)
	set(OLD_BUILD_SHARED_LIBS ${BUILD_SHARED_LIBS})
	set(OLD_BUILD_TESTING ${BUILD_TESTING})

	set(BUILD_SHARED_LIBS OFF)
	set(BUILD_TESTING OFF)
endmacro()

macro(pop_static)
	set(BUILD_SHARED_LIBS ${OLD_BUILD_SHARED_LIBS})
	set(BUILD_TESTING ${OLD_BUILD_TESTING})
endmacro()

#Iterate over all files in platform

set (globbing_expr "${CMAKE_CURRENT_LIST_DIR}/platform/*.cmake")

file(GLOB platforms ${globbing_expr})

foreach(platform ${platforms})
	message(STATUS "Adding platform-code: ${platform}")
	include (${platform})
endforeach()
