include_guard(GLOBAL)

function(add_notices target)
	
	get_target_property(ALIASED_TARGET ${target} ALIASED_TARGET)
	if (ALIASED_TARGET)
		set(target ${ALIASED_TARGET})
	endif()

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

macro(list_all_targets list DIR)
    get_property(TGTS DIRECTORY "${DIR}" PROPERTY BUILDSYSTEM_TARGETS)
    foreach(TGT IN LISTS TGTS)
		get_target_property(excluded ${TGT} EXCLUDE_FROM_ALL)
		if (NOT excluded)
			list(APPEND ${list} ${TGT})
		endif()
    endforeach()

    get_property(SUBDIRS DIRECTORY "${DIR}" PROPERTY SUBDIRECTORIES)
    foreach(SUBDIR IN LISTS SUBDIRS)
		get_property(excluded DIRECTORY ${SUBDIR} PROPERTY EXCLUDE_FROM_ALL)
		if (NOT excluded)
			list_all_targets(${list} "${SUBDIR}")
		endif()
    endforeach()
endmacro()

function(write_notices)

	MESSAGE(STATUS "Generating legal notices...")

	list_all_targets(targets ${CMAKE_SOURCE_DIR})

	foreach(target ${targets})
		get_dependencies(targetList ${target})
	endforeach()

	set(CREDITS_TEXT "This software uses third-party libraries. Those include:")
	
	set(LICENSES_TEXT 
			"Copyright (c) 2023 MadManRises
	
	The respective licenses and copyrights of the used third-party libraries are listed in the following.")

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

	install(FILES ${CMAKE_BINARY_DIR}/LICENSES.txt ${CMAKE_BINARY_DIR}/CREDITS.txt DESTINATION . COMPONENT MadgineLauncher)

	MESSAGE(STATUS "Success")

endfunction(write_notices)

cmake_language(DEFER DIRECTORY ${CMAKE_SOURCE_DIR} CALL write_notices)