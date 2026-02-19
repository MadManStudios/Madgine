
set(MADGINE_AS_SDK TRUE)


if (EXISTS ${CMAKE_CURRENT_LIST_DIR}/../CMakeLists.txt)
#SDK from Source

	add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/.. Madgine)

	include(Util) # Set necessary variables in PARENT_SCOPE

else()
#installed SDK

	

endif()