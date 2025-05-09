include(Util)

once()

include(Workspace)
include(util/ini)
include(Packaging)
include(Shaders)
include(Tooling)

set(MODULES_ENABLE_PLUGINS ON CACHE INTERNAL "")

if (MADGINE_CONFIGURATION)	
	if (NOT EXISTS ${MADGINE_CONFIGURATION}/plugins.ini)
		MESSAGE(WARNING "You provided a configuration directory without a plugins.ini in it. If this was intended, ignore this warning.")
	else()
		set(MODULES_ENABLE_PLUGINS OFF CACHE INTERNAL "")
		
		MESSAGE(STATUS "Using ${MADGINE_CONFIGURATION}/plugins.ini for plugin selection.")
	endif()
endif ()


set(BUILD_SHARED_LIBS ${MODULES_ENABLE_PLUGINS} CACHE BOOL "") #Provide default value OFF for given plugin config

if (MODULES_ENABLE_PLUGINS AND NOT BUILD_SHARED_LIBS)
	MESSAGE(FATAL_ERROR "Currently static builds with plugins are not supported!")
endif()

set(PLUGIN_LIST "" CACHE INTERNAL "")

if (NOT MODULES_ENABLE_PLUGINS)

	add_custom_command(OUTPUT ${CMAKE_BINARY_DIR}/components.cpp 
		COMMAND ./${MADGINE_TOOLING_BINARY} 
					-t
					-npc
					-lp ${MADGINE_CONFIGURATION}/plugins.ini
					-epc ${CMAKE_BINARY_DIR}/components.cpp
		WORKING_DIRECTORY ${MADGINE_TOOLING_WORKING_DIRECTORY}
		DEPENDS MadgineTooling ${MADGINE_CONFIGURATION}/plugins.ini)
	add_custom_target(GenerateComponentsSource DEPENDS ${CMAKE_BINARY_DIR}/components.cpp)

	function(patch_toplevel_target target)
		get_target_property(target_flag ${target} PATCH_TOPLEVEL)
		if (NOT target_flag)
			set_target_properties(${target} PROPERTIES PATCH_TOPLEVEL TRUE)		
			set_property(SOURCE ${CMAKE_BINARY_DIR}/components.cpp PROPERTY GENERATED 1)
			target_sources(${target} PRIVATE ${CMAKE_BINARY_DIR}/components.cpp)
			add_dependencies(${target} GenerateComponentsSource)
		endif()
	endfunction(patch_toplevel_target)

endif ()


macro(add_plugin name base type)

	set(options)
	set(oneValueArgs INSTALL_COMPONENT)
	set(multiValueArgs EXTERNAL_DEPS API_PLUGIN IMPORTED_DEPS)
	cmake_parse_arguments(PLUGIN_CONFIG "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})	

	add_workspace_library(${name} ${PLUGIN_CONFIG_UNPARSED_ARGUMENTS})

	set_target_properties(${name} PROPERTIES 
		OUTPUT_NAME Plugin_${base}_${type}_${name}
		PLUGIN_BASE ${base}
		PLUGIN_TYPE ${type}
		FOLDER "Plugins")

	generate_binary_info(${name})

	if (MADGINE_CONFIGURATION)
		set(PLUGIN_CONFIG_INSTALL_COMPONENT MadgineLauncher)
	elseif (NOT PLUGIN_CONFIG_INSTALL_COMPONENT)
		set(PLUGIN_CONFIG_INSTALL_COMPONENT ${name})
	endif()

	compile_shaders(${name} ${PLUGIN_CONFIG_INSTALL_COMPONENT})

	set(installPlugin TRUE)

	if (NOT MODULES_ENABLE_PLUGINS AND NOT PLUGINS_${type}_${name})		
		MESSAGE (STATUS "Excluding Plugin '${name}' from ALL build.")
		set_target_properties(${name} PROPERTIES EXCLUDE_FROM_ALL TRUE)
		set(installPlugin FALSE)
	endif()

	set(PLUGIN_LIST ${PLUGIN_LIST} ${name} CACHE INTERNAL "")	

	if (installPlugin)
		cpack_add_component_group(${type}Group
					DISPLAY_NAME ${type})
		if (("API_PLUGIN" IN_LIST PLUGIN_CONFIG_KEYWORDS_MISSING_VALUES) OR PLUGIN_CONFIG_API_PLUGIN)
			plugin_group(${name} ${PLUGIN_CONFIG_API_PLUGIN})
			cpack_add_component_group(${name}Group
						DISPLAY_NAME ${name}
						PARENT_GROUP ${type}Group)
			cpack_add_component(${name}
						DISPLAY_NAME API
						GROUP ${name}Group)
		else()
			cpack_add_component(${PLUGIN_CONFIG_INSTALL_COMPONENT} GROUP ${type}Group)
		endif()

		install_to_workspace(${PLUGIN_CONFIG_INSTALL_COMPONENT} TARGETS ${name} ${PLUGIN_CONFIG_EXTERNAL_DEPS})
		install(IMPORTED_RUNTIME_ARTIFACTS ${PLUGIN_CONFIG_IMPORTED_DEPS} RUNTIME DESTINATION bin COMPONENT ${PLUGIN_CONFIG_INSTALL_COMPONENT})

	endif()
	
	#foreach(project ${PROJECTS_DEPENDING_ON_ALL_PLUGINS})
	#	target_link_plugins(${project} ${name})
	#endforeach()

endmacro(add_plugin)

function(target_link_plugins target)

	set(options NO_FATAL)
	set(oneValueArgs)
	set(multiValueArgs)
	cmake_parse_arguments(DEPENDENCIES "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})	

	get_target_property(plugin_dependencies ${target} PLUGIN_DEPENDENCIES)
	if (NOT plugin_dependencies)
		set(plugin_dependencies)
	endif()

	get_target_property(excludeSelf ${target} EXCLUDE_FROM_ALL)

	foreach(plugin ${DEPENDENCIES_UNPARSED_ARGUMENTS})

		get_target_property(exclude ${plugin} EXCLUDE_FROM_ALL)
		if (NOT exclude)
			target_link_libraries(${target} PUBLIC ${plugin})
			list(APPEND plugin_dependencies ${plugin})
			#MESSAGE(STATUS "Linking ${plugin} to ${target}")
		else()
			if (NOT excludeSelf AND NOT DEPENDENCIES_NO_FATAL)
				MESSAGE(FATAL_ERROR "Trying to link ${plugin} to ${target}, but ${plugin} is not enabled in the plugin configuration.")
			endif()
		endif()

	endforeach()

	if (plugin_dependencies)
		list(REMOVE_DUPLICATES plugin_dependencies)
		set_target_properties(${target} PROPERTIES PLUGIN_DEPENDENCIES "${plugin_dependencies}")
	endif()

	if (NOT MODULES_ENABLE_PLUGINS)
		get_target_property(target_type ${target} TYPE)
		if (target_type STREQUAL "EXECUTABLE")
			patch_toplevel_target(${target})
		endif ()
	endif()

endfunction(target_link_plugins)

function(target_link_plugin_groups target)

	get_target_property(plugin_group_dependencies ${target} PLUGIN_GROUP_DEPENDENCIES)
	if (NOT plugin_group_dependencies)
		set(plugin_group_dependencies)
	endif()

	list(APPEND plugin_group_dependencies ${ARGN})
	
	if (plugin_group_dependencies)
		list(REMOVE_DUPLICATES plugin_group_dependencies)
		set_target_properties(${target} PROPERTIES PLUGIN_GROUP_DEPENDENCIES "${plugin_group_dependencies}")
	endif()

endfunction(target_link_plugin_groups)
	
function(target_depend_on_all_plugins target)
	
	if (NOT MODULES_ENABLE_PLUGINS)
		target_link_plugins(${target} ${PLUGIN_LIST} NO_FATAL)
	else()
		add_dependencies(${target} ${PLUGIN_LIST})
	endif()

	MESSAGE(STATUS "Linking ${target} -> ${PLUGIN_LIST}")

	#set(PROJECTS_DEPENDING_ON_ALL_PLUGINS ${PROJECTS_DEPENDING_ON_ALL_PLUGINS} ${target} CACHE INTERNAL "")

endfunction()

function(plugin_group name)
	set(options EXCLUSIVE ATLEAST_ONE)
	set(oneValueArgs)
	set(multiValueArgs)
	cmake_parse_arguments(PLUGIN_GROUP "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})	

	if (PLUGIN_GROUP_UNPARSED_ARGUMENTS)
		MESSAGE(WARNING "Unrecognized options for plugin group: '${LIB_CONFIG_UNPARSED_ARGUMENTS}'")
	endif()

	if (PLUGIN_GROUP_EXCLUSIVE OR PLUGIN_GROUP_ATLEAST_ONE)
		get_target_property(plugin_group_definitions Modules PLUGIN_GROUP_DEFINITIONS)
		if (NOT plugin_group_definitions)
			set(plugin_group_definitions)
		endif()

		if (PLUGIN_GROUP_EXCLUSIVE)
			set(exclusive true)
		else()
			set(exclusive false)
		endif()

		if (PLUGIN_GROUP_ATLEAST_ONE)
			set(atleast_one true)
		else()
			set(atleast_one false)
		endif()

		list(APPEND plugin_group_definitions "setupSection(\"${name}\", ${exclusive}, ${atleast_one})")
	
		set_target_properties(Modules PROPERTIES PLUGIN_GROUP_DEFINITIONS "${plugin_group_definitions}")
	endif()
endfunction()