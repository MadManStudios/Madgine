include(Util)

once()

cmake_policy(SET CMP0003 NEW)
cmake_policy(SET CMP0004 NEW)

set(emscripten_file_dir ${CMAKE_CURRENT_LIST_DIR} CACHE INTERNAL "")

if (EMSCRIPTEN)
	
	set (MADGINE_FORCE_DATA_COLLECT ON CACHE INTERNAL "")

	include(Plugins)

	function (add_workspace_application target)

		add_executable(${target} ${ARGN})

		set_target_properties(${target} PROPERTIES SUFFIX ".html")

		_target_link_libraries(${target} PRIVATE "--preload-file \"${CMAKE_BINARY_DIR}/data@/data\"")

		file(GENERATE OUTPUT ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$<TARGET_FILE_BASE_NAME:${target}>.bat CONTENT "emrun ./$<TARGET_FILE_NAME:${target}>")

	endfunction (add_workspace_application)

	function(target_link_libraries target)
		if (NOT ARGN)
			return ()
		endif()
		set(args ${ARGN})
		list(GET args 0 vis)
		if (NOT vis MATCHES "INTERFACE|PUBLIC|PRIVATE")
			set(vis PUBLIC)
		else()
			list(REMOVE_AT args 0)
		endif()

		foreach(lib ${args})
			if (TARGET ${lib})
				get_target_property(type ${lib} TYPE)

				if (type STREQUAL "SHARED_LIBRARY")
					target_include_directories(${target} ${vis} $<TARGET_PROPERTY:${lib},INTERFACE_INCLUDE_DIRECTORIES>)
					add_dependencies(${target} ${lib})
				else()
					_target_link_libraries(${target} ${vis} ${lib})
				endif()
			else()
				_target_link_libraries(${target} ${vis} ${lib})
			endif()
		endforeach()
	endfunction(target_link_libraries)

	function(install_to_workspace name)

		set(options)
		set(oneValueArgs)
		set(multiValueArgs TARGETS)
		cmake_parse_arguments(OPTIONS "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

		foreach(target ${OPTIONS_TARGETS})
			
			get_target_property(SUFFIX ${target} SUFFIX)

			if (SUFFIX STREQUAL ".html")
				list(REMOVE_ITEM OPTIONS_TARGETS ${target})

				install(
					FILES 
						$<TARGET_FILE:${target}>
						$<TARGET_FILE_DIR:${target}>/$<TARGET_FILE_BASE_NAME:${target}>.data
						$<TARGET_FILE_DIR:${target}>/$<TARGET_FILE_BASE_NAME:${target}>.wasm 
						$<TARGET_FILE_DIR:${target}>/$<TARGET_FILE_BASE_NAME:${target}>.js 
					DESTINATION bin 
					COMPONENT ${name})
			endif()

		endforeach()

	endfunction(install_to_workspace)

endif()