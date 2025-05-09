include(ExternalProject)

add_executable(ShaderGen IMPORTED GLOBAL)

set(endpoint)

if (CMAKE_HOST_WIN32)
	set(endpoint windows-latest)
elseif ("${CMAKE_HOST_SYSTEM_NAME}" STREQUAL "Darwin")
	set(endpoint macos-latest)
elseif ("${CMAKE_HOST_SYSTEM_NAME}" STREQUAL "Linux")
	set(endpoint ubuntu-latest)
else()
	MESSAGE(FATAL_ERROR "Unknown host platform to fetch ShaderGen")
endif()

FetchContent_Declare(
  ShaderGenExt 
  URL https://github.com/MadManStudios/MadgineShadergen/releases/latest/download/${endpoint}.zip
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


add_library(dxcompiler SHARED IMPORTED GLOBAL)

set_target_properties(dxcompiler 
	PROPERTIES
	IMPORTED_LOCATION ${shadergenext_SOURCE_DIR}/dxcompiler${CMAKE_SHARED_LIBRARY_SUFFIX}
	IMPORTED_IMPLIB ${shadergenext_SOURCE_DIR}/dxcompiler${CMAKE_IMPORT_LIBRARY_SUFFIX})
