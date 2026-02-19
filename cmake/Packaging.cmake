include_guard(GLOBAL)

include(Workspace)
include(binaryinfo)



if (MADGINE_CONFIGURATION)
	file(GLOB lists "${MADGINE_CONFIGURATION}/*.list")

	add_custom_target(
		copy_data ALL
		COMMAND ${CMAKE_COMMAND} "-DLISTS=\"$<TARGET_PROPERTY:copy_data,DATA_LISTS>\"" -DTARGET=${CMAKE_BINARY_DIR}/data -P ${workspace_file_dir}/util/listcopy.cmake
		WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
	)

	set_target_properties(copy_data
		PROPERTIES
		DATA_LISTS "${lists}")

	if (ANDROID)
		install(CODE "file(MAKE_DIRECTORY $<INSTALL_PREFIX>/assets/data)
		execute_process(COMMAND \"${CMAKE_COMMAND}\" -DLISTS=$<TARGET_PROPERTY:copy_data,DATA_LISTS> -DTARGET=$<INSTALL_PREFIX>/assets/data -P ${workspace_file_dir}/util/listcopy.cmake WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})" COMPONENT MadgineLauncher)
	elseif (NOT EMSCRIPTEN)
		install(CODE 		
		"execute_process(COMMAND \"${CMAKE_COMMAND}\" -DLISTS=$<TARGET_PROPERTY:copy_data,DATA_LISTS> -DTARGET=$<INSTALL_PREFIX>/data -P ${workspace_file_dir}/util/listcopy.cmake WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})" COMPONENT MadgineLauncher)
	endif ()

endif ()

macro(enable_packaging)

	include(CPackComponent)
	set(CMAKE_INSTALL_DEFAULT_COMPONENT_NAME "Trash")
	set(CPACK_OUTPUT_CONFIG_FILE ${CMAKE_CURRENT_BINARY_DIR}/CPackConfig.cmake)
	set(CPACK_MODULE_PATH ${workspace_file_dir}/cpack/modules)

endmacro()

macro(packaging)

	if (EXISTS LICENSE.rst)
		install(FILES LICENSE.rst DESTINATION . RENAME LICENSE)
		set(CPACK_RESOURCE_FILE_LICENSE ${CMAKE_CURRENT_SOURCE_DIR}/LICENSE.rst)	
	endif()
		
	set(CPACK_PACKAGE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/packages)
	set(CPACK_PACKAGE_INSTALL_DIRECTORY ${PROJECT_NAME})	
	set(CPACK_PACKAGE_FILE_NAME  ${PROJECT_NAME}${MADGINE_CONFIGURATION_SUFFIX}-${PROJECT_VERSION}-${CMAKE_SYSTEM_NAME})

	get_property(MADGINE_CUSTOM_CPACK_SCRIPT GLOBAL PROPERTY MADGINE_CUSTOM_CPACK_SCRIPT)
	if (MADGINE_CUSTOM_CPACK_SCRIPT)
		include(${MADGINE_CUSTOM_CPACK_SCRIPT})
	elseif (OSX)
		include(${workspace_file_dir}/cpack/osx.cmake)
	elseif (ANDROID)
		include(${workspace_file_dir}/cpack/android.cmake)
	elseif (EMSCRIPTEN)
		include(${workspace_file_dir}/cpack/emscripten.cmake)
	elseif (UNIX)
		include(${workspace_file_dir}/cpack/linux.cmake)
	elseif (WINDOWS)
		include(${workspace_file_dir}/cpack/windows.cmake)
	else()	
		MESSAGE(SEND_ERROR "Unsupported platform for packaging!")
	endif(OSX)

	include(CPack)
	

endmacro()

macro(set_custom_packaging file)
	get_property(MADGINE_CUSTOM_CPACK_SCRIPT GLOBAL PROPERTY MADGINE_CUSTOM_CPACK_SCRIPT)
	if (MADGINE_CUSTOM_CPACK_SCRIPT)
		MESSAGE(SEND_ERROR "Only one custom CPack script can be set. You already set '${MADGINE_CUSTOM_CPACK_SCRIPT}'!")
	else()		
		if (IS_ABSOLUTE ${file})
			set(MADGINE_CUSTOM_CPACK_SCRIPT ${file})
		else()
			set(MADGINE_CUSTOM_CPACK_SCRIPT ${CMAKE_CURRENT_LIST_DIR}/${file})
		endif()
		set_property(GLOBAL PROPERTY MADGINE_CUSTOM_CPACK_SCRIPT "${MADGINE_CUSTOM_CPACK_SCRIPT}")
	endif()
endmacro()
