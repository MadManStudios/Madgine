project(Python3)


add_subdirectory(shared)

if (NOT TARGET Python3::Python)
	MESSAGE(STATUS "Failed to setup Python3 - Skipping plugin")
	return()
endif()


set(sources "")
FolderSources(sources "" src)
FolderSources(sources src Python3 python3behaviors.cpp python3behaviors.h python3env.cpp python3env.h python3fileloader.cpp python3fileloader.h python3lib.h python3forward.h python3streamredirect.cpp python3streamredirect.h)
	FolderSources(sources Python3 util pyapifunction.cpp pyapifunction.h pybinding.cpp pybinding.h pyboundapifunction.cpp pyboundapifunction.h pydictptr.cpp pydictptr.h pyenum.cpp pyenum.h pyflags.cpp pyflags.h pyframeptr.cpp pyframeptr.h pylistptr.cpp pylistptr.h pymoduleptr.cpp pymoduleptr.h pyobjectiter.cpp pyobjectiter.h pyobjectptr.cpp pyobjectptr.h pyobjectutil.cpp pyobjectutil.h pyownedscopeptr.cpp pyownedscopeptr.h pyscopeiterator.cpp pyscopeiterator.h pysender.cpp pysender.h python3lock.cpp python3lock.h pytype.cpp pytype.h pyscopeptr.cpp pyscopeptr.h pyvirtualiterator.cpp pyvirtualiterator.h pyvirtualrange.cpp pyvirtualrange.h)
		FolderSources(sources util math pymatrix3.cpp pymatrix3.h pymatrix4.cpp pymatrix4.h pyquaternion.cpp pyquaternion.h pyvector2.cpp pyvector2.h pyvector3.cpp pyvector3.h pyvector4.cpp pyvector4.h)

add_plugin(Python3 Madgine Behavior ${sources} PRECOMPILED_HEADER src/Python3/python3lib.h IMPORTED_DEPS Python3::Python)

target_link_plugins(Python3 Root Resources Debugger Behavior)

target_link_libraries(Python3 PUBLIC Python3::Python)

if (_Python3_LIBRARY_DEBUG)
	target_compile_definitions(Python3 PUBLIC HAS_PYTHON3_DEBUG_LIB)
endif (_Python3_LIBRARY_DEBUG)

get_target_property(stdlib Python3::Python STDLIB)
file(GLOB files LIST_DIRECTORIES true RELATIVE ${stdlib} ${stdlib}/*)

list(REMOVE_ITEM files site-packages)

set(filename python${Python3_VERSION_MAJOR}${Python3_VERSION_MINOR}.zip)

add_custom_command(TARGET Python3
	POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:Python3::Python> ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
	COMMAND ${CMAKE_COMMAND} -E tar "c" ${CMAKE_BINARY_DIR}/${filename} --format=zip ${files}
	WORKING_DIRECTORY ${stdlib})

target_compile_definitions(Python3 PRIVATE PYTHON3_STDLIB_ZIP="${filename}")

target_ship_file(Python3 ${CMAKE_BINARY_DIR}/${filename})

set(sources "")
FolderSources(sources "" tools)
	FolderSources(sources tools Madgine_Tools)
		FolderSources(sources Madgine_Tools Python3 python3debugvisualizers.cpp python3immediatewindow.cpp python3immediatewindow.h)

add_tools(Python3 ${sources} PRECOMPILED_HEADER tools/Madgine_Tools/Python3/python3toolslib.h)

target_link_plugins(Python3Tools TextEditor DebuggerTools)




