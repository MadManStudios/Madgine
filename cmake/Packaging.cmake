include(Util)

once()

include(Workspace)
include(binaryinfo)

include(GetPrerequisites)

if (MADGINE_CONFIGURATION)
	file(GLOB lists "${MADGINE_CONFIGURATION}/*.list")

	add_custom_target(
		copy_data ALL
		COMMAND ${CMAKE_COMMAND} "-DLISTS=\"$<TARGET_PROPERTY:copy_data,DATA_LISTS>\"" -DTARGET=${CMAKE_BINARY_DIR}/data -DBINARY_DIR=${CMAKE_BINARY_DIR} -P ${workspace_file_dir}/util/listcopy.cmake
		WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
	)

	set_target_properties(copy_data
		PROPERTIES
		DATA_LISTS "${lists}")

endif ()

macro(enable_packaging)

	include(CPackComponent)
	set(CMAKE_INSTALL_DEFAULT_COMPONENT_NAME "Trash")
	set(CPACK_OUTPUT_CONFIG_FILE ${CMAKE_CURRENT_BINARY_DIR}/CPackConfig.cmake)
	set(CPACK_MODULE_PATH ${workspace_file_dir}/cpack)

endmacro()

macro(packaging)

	if (EXISTS LICENSE.rst)
		install(FILES LICENSE.rst DESTINATION . RENAME LICENSE)
	endif()

	if (UNIX)
		set(CPACK_GENERATOR STGZ DEB)
		set(CPACK_DEBIAN_PACKAGE_MAINTAINER ${CPACK_PACKAGE_VENDOR})
	endif(UNIX)

	if (WINDOWS)
		find_package(WindowsSDK)
		if (WINDOWSSDK_FOUND)
			find_program(signtool signtool.exe PATHS ${WINDOWSSDK_LATEST_DIR})
			if (signtool)
				set(CPACK_NSIS_FINALIZE_CMD "\\\"${signtool}\\\" sign /fd sha256 /tr http://ts.ssl.com /td sha256 /d \\\"${PROJECT_NAME}\\\" /a %1") 
			endif()
		endif()
	endif (WINDOWS)

	set(CPACK_PACKAGE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/packages)
	set(CPACK_PACKAGE_INSTALL_DIRECTORY ${PROJECT_NAME})
	set(CPACK_RESOURCE_FILE_LICENSE ${CMAKE_CURRENT_SOURCE_DIR}/LICENSE.rst)
	set(CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL ON)
	set(CPACK_NSIS_PACKAGE_NAME ${PROJECT_NAME}-${PROJECT_VERSION})
	set(CPACK_NSIS_DISPLAY_NAME ${PROJECT_NAME})

	include(CPack)
	

endmacro()

add_custom_target(
    Remove_dependencies
    COMMAND ${CMAKE_COMMAND} -E echo "Removing old dependencies"
	COMMAND ${CMAKE_COMMAND} -E remove_directory "${CMAKE_BINARY_DIR}/dependencies_bin"
)



function(collect_target_dependencies target)	

	add_dependencies(${target} Remove_dependencies)

	set(plugin_pattern ${ARGN})

	configure_file(${workspace_file_dir}/gatherDependencies.cmake.in ${CMAKE_BINARY_DIR}/gatherDependencies-${target}.cmake.in @ONLY)
	configure_file(${workspace_file_dir}/gatherDependencies-run.cmake.in ${CMAKE_BINARY_DIR}/gatherDependencies-run-${target}.cmake @ONLY)
	FILE(GENERATE OUTPUT ${CMAKE_BINARY_DIR}/gatherDependencies-${target}-$<CONFIG>.cmake INPUT ${CMAKE_BINARY_DIR}/gatherDependencies-${target}.cmake.in)

	add_custom_command(TARGET ${target} POST_BUILD COMMAND ${CMAKE_COMMAND} -P ${CMAKE_BINARY_DIR}/gatherDependencies-run-${target}.cmake)

	install(DIRECTORY ${CMAKE_BINARY_DIR}/dependencies_bin/${target}/ DESTINATION bin COMPONENT ${target})

endfunction(collect_target_dependencies)

function(collect_custom_dependencies target name binary)

	add_custom_target(
		create_${name}_dep_folder
		DEPENDS Remove_dependencies
		COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/dependencies_bin/${name}"
	)

	add_dependencies(${target} create_${name}_dep_folder)
	
	GET_PREREQUISITES(${binary} Dependencies 1 1 "" "")

	get_filename_component(bin_path ${binary} DIRECTORY)

	foreach(dep ${Dependencies})
		GP_RESOLVE_ITEM(${binary} ${dep} "${bin_path}" "" path)
		GP_FILE_TYPE(${binary} ${path} type)
		if (NOT ${type} STREQUAL "local")
			add_custom_command(TARGET ${target} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy ${path} ${CMAKE_BINARY_DIR}/dependencies_bin/${name})
			#MESSAGE(STATUS ${dep} ${type} ${path})
		endif()
	endforeach()

endfunction(collect_custom_dependencies)
