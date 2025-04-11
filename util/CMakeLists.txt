include(ExternalProject)

add_executable(ShaderGen IMPORTED GLOBAL)

set(endpoint)

if (WINDOWS)
	set(endpoint windows-latest)
elseif (OSX)
	set(endpoint macos-latest)
elseif (LINUX)
	set(endpoint ubunutu-latest)
else()
	MESSAGE(SEND_ERROR "Unknown host platform to fetch ShaderGen")
endif()

FetchContent_Declare(
  ShaderGenExt 
  URL https://github.com/MadManStudios/MadgineShadergen/releases/download/latest-main/${endpoint}.zip
)

FetchContent_MakeAvailable(ShaderGenExt)

set(shadergenPath ${shadergenext_SOURCE_DIR}/ShaderGen${HOST_EXECUTABLE_SUFFIX})

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