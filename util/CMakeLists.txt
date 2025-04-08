include(ExternalProject)

if (MADGINE_CONFIGURATION)
	set (DEPENDS MadgineTooling)
endif ()

set (MADGINE_SHADERGEN_PATH "" CACHE FILEPATH "")

add_executable(ShaderGen IMPORTED GLOBAL)

if (MADGINE_SHADERGEN_PATH)
	set(shadergenPath ${MADGINE_SHADERGEN_PATH})
else()
    cmake_path(GET CMAKE_BINARY_DIR PARENT_PATH GLOBAL_BINARIES)
	set(shadergenPath ${GLOBAL_BINARIES}/shadergen/src/ShaderGen${HOST_EXECUTABLE_SUFFIX})
	ExternalProject_Add(ShaderGenExt
		PREFIX ${GLOBAL_BINARIES}/shadergen 
		GIT_REPOSITORY https://github.com/MadManStudios/MadgineShadergen.git
		GIT_TAG main
		CMAKE_ARGS -DCMAKE_BUILD_TYPE=Release
		BUILD_BYPRODUCTS ${shadergenPath}
		BUILD_ALWAYS true
		INSTALL_COMMAND ""
		DEPENDS ${DEPENDS}
		USES_TERMINAL_CONFIGURE true
		USES_TERMINAL_BUILD true)

	add_dependencies(ShaderGen ShaderGenExt)	
endif()


set_target_properties(ShaderGen 
	PROPERTIES ShaderGenTargets ""
	IMPORTED_LOCATION ${shadergenPath})
	
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