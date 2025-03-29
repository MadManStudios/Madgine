cmake_policy(SET CMP0057 NEW)

set(filenames)

foreach(list ${LISTS})
	
	message(STATUS "Copying data-list: ${list}")

	file(READ ${list} contents)

	# Convert file contents into a CMake list (where each element in the list
	# is one line of the file)		
	string(REGEX REPLACE ";" "\\\\;" contents "${contents}")
	string(REGEX REPLACE "\n" ";" contents "${contents}")

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


