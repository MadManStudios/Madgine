cmake_policy(SET CMP0057 NEW)

set(filenames)

foreach(list ${LISTS})
	
	message(STATUS "Copying data-list: ${list}")

	file(STRINGS ${list} contents)

	file(INSTALL ${contents} DESTINATION ${TARGET})

	foreach(file ${contents})
		get_filename_component(name ${file} NAME)
		list(APPEND filenames ${name})
	endforeach()

endforeach()

#file(GLOB targetFiles "${TARGET}/*")
#foreach(file ${targetFiles})
#	get_filename_component(name ${file} NAME)
#	if (NOT ${name} IN_LIST filenames)
#		MESSAGE(STATUS "Removing old file: ${name}")
#		file(REMOVE ${file})
#	endif()
#endforeach()


