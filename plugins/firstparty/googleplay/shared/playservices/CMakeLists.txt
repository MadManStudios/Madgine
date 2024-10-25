Project(PlayServices)

if (ANDROID)

	add_workspace_library(PlayServices playservices.cpp SOURCE_ROOT .)

	target_link_libraries(PlayServices PUBLIC Interfaces)
	
	install_to_workspace(Madgine EXPORT_LIB TARGETS PlayServices)

	list(APPEND ANDROID_SOURCES
		${CMAKE_CURRENT_SOURCE_DIR}/
	)
	set(ANDROID_SOURCES "${ANDROID_SOURCES}" CACHE INTERNAL "")

endif()