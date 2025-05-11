include(Util)

once()

if (MADGINE_CONFIGURATION)

	set (MADGINE_PREBUILT_TOOLING "" CACHE PATH "Path to prebuilt tooling binaries")

	add_executable(MadgineTooling IMPORTED)	

	if (MADGINE_PREBUILT_TOOLING)		
		set_target_properties(MadgineTooling PROPERTIES IMPORTED_LOCATION ${MADGINE_PREBUILT_TOOLING}/MadgineLauncher${HOST_EXECUTABLE_SUFFIX})
	else()
		set (MADGINE_TOOLING_PRESET "Clang-Debug" CACHE STRING "Specify preset to use to create the tooling binary")

		add_custom_target(MadgineToolingBuild ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR}/../${MADGINE_TOOLING_PRESET}
			WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
			USES_TERMINAL)

		add_dependencies(MadgineTooling MadgineToolingBuild)

		set_target_properties(MadgineTooling PROPERTIES IMPORTED_LOCATION ${CMAKE_BINARY_DIR}/../${MADGINE_TOOLING_PRESET}/bin/MadgineLauncher${HOST_EXECUTABLE_SUFFIX})
	endif()

endif()