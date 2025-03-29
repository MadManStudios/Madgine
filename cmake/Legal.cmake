include(Util)

once()

function(add_notices target)
	
	set(options)
	set(oneValueArgs LICENSE_FILE CREDITS READABLE_NAME)
	set(multiValueArgs)
	cmake_parse_arguments(NOTICES_CONFIG "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})	

	get_target_property(IMPORTED ${target} IMPORTED)

	if (IMPORTED)
		set_target_properties(${target} PROPERTIES IMPORTED_GLOBAL TRUE)
	elseif(NOTICES_CONFIG_LICENSE_FILE)
		set (NOTICES_CONFIG_LICENSE_FILE "${CMAKE_CURRENT_SOURCE_DIR}/${NOTICES_CONFIG_LICENSE_FILE}")
	endif()

	if (NOTICES_CONFIG_LICENSE_FILE)
		set_target_properties(${target} PROPERTIES LICENSE_FILE ${NOTICES_CONFIG_LICENSE_FILE})
	endif()

	if (NOTICES_CONFIG_CREDITS)
		set_target_properties(${target} PROPERTIES CREDITS ${NOTICES_CONFIG_CREDITS})
	endif()

	if (NOTICES_CONFIG_READABLE_NAME)
		set_target_properties(${target} PROPERTIES READABLE_NAME ${NOTICES_CONFIG_READABLE_NAME})
	endif()

endfunction(add_notices)

function(write_notices target)

	MESSAGE(STATUS "Generating legal notices...")

	get_dependencies(targetList ${target})

	set(CREDITS_TEXT "This software uses third-party libraries. Those include:")
	
	set(LICENSES_TEXT 
			"Copyright (c) 2023 MadManRises
	
	The respective licenses and copyrights of the used third-party libraries are listed in the following.")
	
	MESSAGE(STATUS ${targetList})

	foreach(target ${targetList})

		get_target_property(name ${target} READABLE_NAME)
		if (NOT name)
			set(name ${target})
		endif()

		get_target_property(LICENSE_FILE ${target} LICENSE_FILE)
		if (LICENSE_FILE)

			file(READ ${LICENSE_FILE} fileContent)

			set(LICENSES_TEXT "${LICENSES_TEXT}

################ ${name} License ###############

${fileContent}")
		endif()

		get_target_property(CREDITS ${target} CREDITS)
		if (CREDITS)

			set(CREDITS_TEXT "${CREDITS_TEXT}

	${CREDITS}")

		endif()

	endforeach()

	file(WRITE ${CMAKE_BINARY_DIR}/LICENSES.txt ${LICENSES_TEXT})
	file(WRITE ${CMAKE_BINARY_DIR}/CREDITS.txt ${CREDITS_TEXT})

	MESSAGE(STATUS "Success")

endfunction(write_notices)