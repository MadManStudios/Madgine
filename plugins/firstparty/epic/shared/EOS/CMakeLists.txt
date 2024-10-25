Project(EOS)

set(EOS_Dir "" CACHE FILEPATH "")

if (EOS_Dir)

	add_library(EOS SHARED IMPORTED GLOBAL)

	set_target_properties(EOS PROPERTIES
		IMPORTED_LOCATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/EOSSDK-Win64-Shipping.dll
		IMPORTED_IMPLIB ${EOS_Dir}/lib/EOSSDK-Win64-Shipping.lib)

	target_include_directories(EOS INTERFACE ${EOS_Dir}/include)

	add_custom_target(EOSCopy ${CMAKE_COMMAND} -E copy_if_different
			${EOS_Dir}/Bin/EOSSDK-Win64-Shipping.dll
			${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

	add_dependencies(EOS EOSCopy)

endif()