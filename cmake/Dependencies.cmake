include_guard(GLOBAL)

include(FetchContent)

function(resolve_dependencies)

	set(dependenciesFile ${CMAKE_SOURCE_DIR}/dependencies.txt)
	if (EXISTS ${dependenciesFile})
		
		
		message(STATUS "Reading dependencies: ${dependenciesFile}")

		file(STRINGS ${dependenciesFile} contents)

		set(dependencies )

		foreach(file ${contents})

			if (file MATCHES ".*/([^/]*)\.git")
				
				set(name ${CMAKE_MATCH_1})

				set(path ${CMAKE_SOURCE_DIR}/../${name})
				
				if (IS_DIRECTORY ${path})
				
					string(TOUPPER ${name} name_uppercase)

					MESSAGE(STATUS "Using locally cloned repository ${path} for ${name}")

					set(FETCHCONTENT_SOURCE_DIR_${name_uppercase} ${path})
				endif()

				FetchContent_Declare(
					${name}
					GIT_REPOSITORY ${file}
					GIT_TAG main
				)

				list(APPEND dependencies ${name})

				Message(STATUS ${name})
			elseif(EXISTS ${file})
				FetchContent_Declare(
					${name}
					SOURCE_DIR ${file}
				)
			else()
				MESSAGE(SEND_ERROR "Unable to parse dependency ${file}")
			endif()
			
		endforeach()

		FetchContent_MakeAvailable(${dependencies})

	endif()

endfunction(resolve_dependencies)