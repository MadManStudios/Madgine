include_guard(GLOBAL)

include(Util)

cmake_policy(SET CMP0022 NEW)

set(workspace_file_dir ${CMAKE_CURRENT_LIST_DIR} CACHE INTERNAL "")

 
function(install_header name)    
	
	install(DIRECTORY src/ DESTINATION ${name}/include
      COMPONENT ${name}
	  FILES_MATCHING PATTERN "*.h"
    )	

endfunction(install_header)

function(install_to_workspace name)

	set(options)
	set(oneValueArgs)
	set(multiValueArgs TARGETS)
	cmake_parse_arguments(OPTIONS "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

	foreach(target ${OPTIONS_TARGETS})
		get_target_property(target_type ${target} TYPE)
		if (target_type STREQUAL STATIC_LIBRARY)
			list(REMOVE_ITEM OPTIONS_TARGETS ${target})
		endif()
	endforeach()

	install(
		TARGETS ${OPTIONS_TARGETS} 
		RUNTIME DESTINATION bin COMPONENT ${name}
		BUNDLE DESTINATION App COMPONENT ${name}
		ARCHIVE DESTINATION lib COMPONENT ${name}
	)

	foreach(target ${OPTIONS_TARGETS})
		get_target_property(TARGET_SOURCE_DIR ${target} SOURCE_DIR)
		if (EXISTS ${TARGET_SOURCE_DIR}/data)
			install(DIRECTORY ${TARGET_SOURCE_DIR}/data DESTINATION . COMPONENT ${name})
		endif()

		#target_include_directories(${target} INTERFACE $<INSTALL_INTERFACE:$<INSTALL_PREFIX>/${target}/include>)
	endforeach()

endfunction(install_to_workspace)


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

macro(target_ship_folder target path)
	get_target_property(component ${target} INSTALL_COMPONENT)
	if (component)
		install(DIRECTORY ${path} DESTINATION . COMPONENT ${component})
	endif()
endmacro(target_ship_folder)

macro(target_ship_file target path)
	get_target_property(component ${target} INSTALL_COMPONENT)
	if (component)
		install(FILES ${path} DESTINATION . COMPONENT ${component})
	endif()
endmacro(target_ship_file)

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


macro(push_static)
	set(OLD_BUILD_SHARED_LIBS ${BUILD_SHARED_LIBS})
	set(OLD_BUILD_TESTING ${BUILD_TESTING})

	set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
	set(BUILD_TESTING OFF)
endmacro()

macro(pop_static)
	set(BUILD_SHARED_LIBS ${OLD_BUILD_SHARED_LIBS} CACHE BOOL "" FORCE)
	set(BUILD_TESTING ${OLD_BUILD_TESTING})
endmacro()

#Iterate over all files in platform

set (globbing_expr "${CMAKE_CURRENT_LIST_DIR}/platform/*.cmake")

file(GLOB platforms ${globbing_expr})

foreach(platform ${platforms})
	message(STATUS "Adding platform-code: ${platform}")
	include (${platform})
endforeach()
