include(Util)

once()

include(FetchContent)

function(resolve_dependencies)

	set(dependenciesFile ${CMAKE_SOURCE_DIR}/dependencies.txt)
	if (EXISTS ${dependenciesFile})
		
		
		message(STATUS "Reading dependencies: ${dependenciesFile}")

		file(READ ${dependenciesFile} contents)

		# Convert file contents into a CMake list (where each element in the list
		# is one line of the file)		
		string(REGEX REPLACE ";" "\\\\;" contents "${contents}")
		string(REGEX REPLACE "\n" ";" contents "${contents}")

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
			else()
				MESSAGE(SEND_ERROR "Unable to parse dependency ${file}")
			endif()
			
		endforeach()

		FetchContent_MakeAvailable(${dependencies})

	endif()

endfunction(resolve_dependencies)