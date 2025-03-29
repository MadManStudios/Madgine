include(Util)

once()

if (MADGINE_CONFIGURATION)

	set (MADGINE_TOOLING_PRESET "Clang-Debug" CACHE STRING "Specify preset to use to create the tooling binary")

	add_custom_target(MadgineTooling ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR}/../${MADGINE_TOOLING_PRESET}
		WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
		USES_TERMINAL)

	set_target_properties(MadgineTooling PROPERTIES BinaryName MadgineLauncher)

	set (MADGINE_TOOLING_BINARY $<TARGET_PROPERTY:MadgineTooling,BinaryName>)
	set (MADGINE_TOOLING_WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/../${MADGINE_TOOLING_PRESET}/bin)

endif()