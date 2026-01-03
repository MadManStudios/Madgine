include(Util)

once()

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
		execute_process(COMMAND ${CMAKE_COMMAND} -DLISTS=$<TARGET_PROPERTY:copy_data,DATA_LISTS> -DTARGET=$<INSTALL_PREFIX>/assets/data -P ${workspace_file_dir}/util/listcopy.cmake WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})" COMPONENT MadgineLauncher)
	elseif (NOT EMSCRIPTEN)
		install(CODE 		
		"execute_process(COMMAND ${CMAKE_COMMAND} -DLISTS=$<TARGET_PROPERTY:copy_data,DATA_LISTS> -DTARGET=$<INSTALL_PREFIX>/data -P ${workspace_file_dir}/util/listcopy.cmake WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})" COMPONENT MadgineLauncher)
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

	if (OSX)
		set(CPACK_GENERATOR DragNDrop)
		set(CPACK_BUNDLE_NAME ${PROJECT_NAME})	
	elseif (UNIX)
		set(CPACK_GENERATOR STGZ DEB)
		set(CPACK_DEBIAN_PACKAGE_MAINTAINER ${CPACK_PACKAGE_VENDOR})
	endif(OSX)

	if (WINDOWS)
		set(CPACK_GENERATOR NSIS)
		set(CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL ON)
		set(CPACK_NSIS_PACKAGE_NAME ${PROJECT_NAME}${MADGINE_CONFIGURATION_SUFFIX}-${PROJECT_VERSION})	
		set(CPACK_NSIS_DISPLAY_NAME ${PROJECT_NAME})

		find_package(WindowsSDK)
		if (WINDOWSSDK_FOUND)
			find_program(signtool signtool.exe PATHS ${WINDOWSSDK_LATEST_DIR})
			if (signtool)
				set(CPACK_NSIS_FINALIZE_CMD "\\\"${signtool}\\\" sign /fd sha256 /tr http://ts.ssl.com /td sha256 /d \\\"${PROJECT_NAME}\\\" /a %1") 
			endif()
		endif()		
	endif (WINDOWS)

	if (EMSCRIPTEN)
		set(CPACK_GENERATOR ZIP)
		set(CPACK_INCLUDE_TOPLEVEL_DIRECTORY OFF)
	endif()

	if (ANDROID)
		set(CPACK_GENERATOR External)
		set(CPACK_EXTERNAL_ENABLE_STAGING ON)
		file(GENERATE OUTPUT apk.cmake INPUT ${workspace_file_dir}/cpack/android/apk.cmake.in)
		set(CPACK_EXTERNAL_PACKAGE_SCRIPT ${CMAKE_CURRENT_BINARY_DIR}/apk.cmake)
		set(CPACK_PACKAGE_FILE_NAME  ${PROJECT_NAME}${MADGINE_CONFIGURATION_SUFFIX}-${PROJECT_VERSION}-${CMAKE_SYSTEM_NAME}-${CMAKE_ANDROID_ARCH_ABI})
	endif(ANDROID)

	include(CPack)
	

endmacro()
