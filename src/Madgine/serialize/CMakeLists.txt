
if (NOT EMSCRIPTEN)
	add_subdirectory(network)
	set_target_properties(NetworkSerialize PROPERTIES EXCLUDE_FROM_ALL TRUE)
endif()

add_subdirectory(filesystem)
set_target_properties(FilesystemSerialize PROPERTIES EXCLUDE_FROM_ALL TRUE)

add_subdirectory(memory)
set_target_properties(MemorySerialize PROPERTIES EXCLUDE_FROM_ALL TRUE)
