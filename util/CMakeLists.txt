include(ExternalProject)

set(shadergenSourceDir ${CMAKE_CURRENT_SOURCE_DIR}/shadergen)
set(shadergenBinaryDir ${CMAKE_CURRENT_BINARY_DIR}/shadergen)

if (MADGINE_CONFIGURATION)
	set (DEPENDS MadgineTooling)
endif ()

add_executable(ShaderGen IMPORTED GLOBAL)

if (NOT SKIP_UTIL_BUILD)
	ExternalProject_Add(ShaderGenExt
		PREFIX shadergen 
		BINARY_DIR ${shadergenBinaryDir}/build
		SOURCE_DIR ${shadergenSourceDir}
		CMAKE_ARGS -DCMAKE_BUILD_TYPE=Release
		BUILD_BYPRODUCTS ${shadergenBinaryDir}/ShaderGen${HOST_EXECUTABLE_SUFFIX}
		BUILD_COMMAND ${CMAKE_COMMAND} --build . --target ShaderGen
		BUILD_ALWAYS true
		INSTALL_COMMAND ""
		DEPENDS ${DEPENDS}
		USES_TERMINAL_CONFIGURE true
		USES_TERMINAL_BUILD true)

	add_dependencies(ShaderGen ShaderGenExt)	
endif()


set_target_properties(ShaderGen 
	PROPERTIES ShaderGenTargets ""
	IMPORTED_LOCATION ${shadergenBinaryDir}/ShaderGen${HOST_EXECUTABLE_SUFFIX})
	
function(add_shadergen_target target flag)

	get_target_property(exclude ${target} EXCLUDE_FROM_ALL)
	if (NOT exclude)
		add_dependencies(${target} ShaderGen)

		get_target_property(targets ShaderGen ShaderGenTargets)
		set (targets ${targets} ${flag})
		list(REMOVE_DUPLICATES targets)
		set_target_properties(ShaderGen PROPERTIES ShaderGenTargets "${targets}")
	endif()

endfunction(add_shadergen_target)