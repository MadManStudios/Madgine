
PROJECT(Resources)

add_subdirectory(shared)

set(sources "")
FolderSources(sources "" src)
	FolderSources(sources src Madgine)
		FolderSources(sources Madgine resources resourcebase.cpp resourcebase.h resourceloaderbase.cpp resourceloaderbase.h resourceloadercollector.cpp resourceloadercollector.h resourceloader.cpp resourceloader.h resourcemanager.cpp resourcemanager.h)

add_plugin(Resources Madgine Core ${sources})

target_link_plugins(Resources Root)

if (MSVC)
	target_link_options(Resources PUBLIC "/NATVIS:${CMAKE_CURRENT_LIST_DIR}/src/Resources.natvis")
endif()

set(sources "")
FolderSources(sources "" tools)
	FolderSources(sources tools Madgine_Tools resourcestoolconfig.cpp resourcestoolconfig.h resourcestoolsforward.h resourcestoolslib.h)


add_tools(Resources ${sources} PRECOMPILED_HEADER tools/Madgine_Tools/resourcestoolslib.h)


if (MADGINE_CONFIGURATION)
	get_target_property(data_lists copy_data DATA_LISTS)
	set(local_lists ${data_lists})
	list(FILTER data_lists EXCLUDE REGEX ".*resources\.list")
	list(FILTER local_lists INCLUDE REGEX ".*resources\.list")

	add_custom_target(bake_resource_lists
		COMMAND ./${MADGINE_TOOLING_BINARY} 
					-t
					-npc
					-lp ${MADGINE_CONFIGURATION}/plugins.ini
					--bake ${local_lists}
					--bake-output-list ${CMAKE_BINARY_DIR}/resources.list
		WORKING_DIRECTORY ${MADGINE_TOOLING_WORKING_DIRECTORY}
		DEPENDS MadgineTooling ${local_lists})

	add_dependencies(copy_data bake_resource_lists)

	list(APPEND data_lists ${CMAKE_BINARY_DIR}/resources.list)
	
	set_target_properties(copy_data
		PROPERTIES
		DATA_LISTS "${data_lists}")
endif()