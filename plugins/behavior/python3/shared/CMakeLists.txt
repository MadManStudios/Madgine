
if (NOT CMAKE_CROSSCOMPILING)

	find_package (Python3 COMPONENTS Development.Embed)

	if (NOT Python3_Development.Embed_FOUND OR NOT PYTHON_EXECUTABLE)
		MESSAGE(STATUS "Python3 not found.")
		return()
	endif()

	find_file (python_license NAMES LICENSE.txt PATHS ${Python3_LIBRARY_DIRS}/python${Python3_VERSION_MAJOR}.${Python3_VERSION_MINOR} ${PYTHON_EXECUTABLE}/.. NO_DEFAULT_PATH)

	if (NOT python_license)
		MESSAGE(STATUS "Failed to find Python3 License")
		return()
	endif()

	add_notices(Python3::Python 
		READABLE_NAME Python3
		LICENSE_FILE ${python_license}
		CREDITS "Scripting-Language: Python3 - https://www.python.org/")

else()

#docker run --rm -ti -v C:\Users\Bub\Downloads\Python-3.13.0\Python-3.13.0:/python-wasm/cpython -v C:\emsdk:/emsdk -w /python-wasm/cpython python3

	if (NOT MADGINE_PYTHON_ROOT OR NOT MADGINE_PYTHON_LIB)
		MESSAGE(STATUS "Cross-compiling targets require MADGINE_PYTHON_ROOT and MADGINE_PYTHON_LIB set to link against Python")
		return()
	endif()

	if (NOT EXISTS ${MADGINE_PYTHON_ROOT})
		MESSAGE(STATUS "${MADGINE_PYTHON_ROOT} (MADGINE_PYTHON_ROOT) does not exist")
		return()
	endif()

	if (NOT EXISTS ${MADGINE_PYTHON_LIB})
		MESSAGE(STATUS "${MADGINE_PYTHON_LIB} (MADGINE_PYTHON_LIB) does not exist")
		return()
	endif()

	find_file (python_license NAMES LICENSE.txt LICENSE PATHS "${MADGINE_PYTHON_ROOT}" NO_CMAKE_FIND_ROOT_PATH NO_DEFAULT_PATH)

	if (NOT python_license)
		MESSAGE(STATUS "Failed to find Python3 License")
		return()
	endif()

	add_library(Python3::Python STATIC IMPORTED GLOBAL)

	set_target_properties(Python3::Python PROPERTIES
		IMPORTED_LOCATION ${MADGINE_PYTHON_LIB})

	target_include_directories(Python3::Python INTERFACE ${MADGINE_PYTHON_ROOT}/include ${MADGINE_PYTHON_LIB}/..)

	target_link_libraries(Python3::Python INTERFACE ${MADGINE_PYTHON_MODULE_LIBS} "--preload-file \"${MADGINE_PYTHON_LIB}/../usr@/usr\"")


	add_notices(Python3::Python 
		READABLE_NAME Python3
		LICENSE_FILE ${python_license}
		CREDITS "Scripting-Language: Python3 - https://www.python.org/")

endif()