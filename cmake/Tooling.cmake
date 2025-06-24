include(Util)

once()

if (MADGINE_CONFIGURATION)

	set (MADGINE_PREBUILT_TOOLING "" CACHE PATH "Path to prebuilt tooling binaries")

	add_executable(MadgineTooling IMPORTED GLOBAL)	

	if (MADGINE_PREBUILT_TOOLING)		
		set(MADGINE_TOOLING_PREFIX ${MADGINE_PREBUILT_TOOLING} CACHE INTERNAL "")
	else()
		set (MADGINE_TOOLING_PRESET "Clang-Debug" CACHE STRING "Specify preset to use to create the tooling binary")

		add_custom_target(MadgineToolingBuild ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR}/../${MADGINE_TOOLING_PRESET}
			WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
			USES_TERMINAL)

		add_dependencies(MadgineTooling MadgineToolingBuild)

		set(MADGINE_TOOLING_PREFIX ${CMAKE_BINARY_DIR}/../${MADGINE_TOOLING_PRESET}/bin CACHE INTERNAL "")
	endif()

endif()