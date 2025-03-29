

set(BUILD_DEMOS OFF CACHE BOOL "" FORCE)
set(BUILD_TESTS OFF CACHE BOOL "" FORCE)
add_subdirectory(zep)
set_target_properties(Zep PROPERTIES EXCLUDE_FROM_ALL TRUE)
target_include_directories(Zep PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/zep/include>)

add_notices(Zep 
	READABLE_NAME "Zep" 
	LICENSE_FILE zep/LICENSE 
	CREDITS "TextEditor: Zep - https://github.com/Rezonality/zep")

