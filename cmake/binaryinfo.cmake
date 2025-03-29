include(Util)

once()

set(binaryinfo_dir ${CMAKE_CURRENT_LIST_DIR} CACHE INTERNAL "")

macro(generate_binary_info target)

	if (MODULES_ENABLE_PLUGINS)

		target_sources(${target} PRIVATE ${binaryinfo_dir}/binaryinfo.cpp)

		set_target_properties(${target} PROPERTIES ENABLE_EXPORTS ON)

		set (major_version ${PROJECT_VERSION_MAJOR})
		if (NOT major_version)
			set(major_version 1)
		endif()

		set (minor_version ${PROJECT_VERSION_MINOR})
		if (NOT minor_version)
			set(minor_version 0)
		endif()

		set (patch_number ${PROJECT_VERSION_PATCH})
		if (NOT patch_number)
			set(patch_number 0)
		endif()

		
		target_compile_definitions(${target} PRIVATE 
		PROJECT_NAME=${target}
		BINARY_PROJECT_ROOT=\"${PROJECT_SOURCE_DIR}\"
		BINARY_BINARY_DIR=\"${CMAKE_RUNTIME_OUTPUT_DIRECTORY}\"
		BINARY_MAJOR_VERSION=${major_version}
		BINARY_MINOR_VERSION=${minor_version}
		BINARY_PATCH_NUMBER=${patch_number}
		BINARY_FILENAME=\"$<TARGET_FILE_NAME:${target}>\"
		TOOLS_NAME=\"$<TARGET_PROPERTY:${target},TOOLS_NAME>\"
		PRECOMPILED_HEADER_PATH=\"$<TARGET_PROPERTY:${target},PRECOMPILED_HEADER>\"
		BINARY_SOURCE_ROOT=\"$<TARGET_PROPERTY:${target},SOURCE_ROOT>\"
		PLUGIN_DEPENDENCIES=$<$<BOOL:$<TARGET_PROPERTY:${target},PLUGIN_DEPENDENCIES>>:\"$<JOIN:$<TARGET_PROPERTY:${target},PLUGIN_DEPENDENCIES>,",">\",>nullptr
		PLUGIN_GROUP_DEPENDENCIES=$<$<BOOL:$<TARGET_PROPERTY:${target},PLUGIN_GROUP_DEPENDENCIES>>:\"$<JOIN:$<TARGET_PROPERTY:${target},PLUGIN_GROUP_DEPENDENCIES>,",">\",>nullptr)


	endif()

endmacro()
