
if (NOT CMAKE_CROSSCOMPILING)

	find_package (Python3 REQUIRED COMPONENTS Interpreter Development.Embed)

	if (NOT Python3_Development.Embed_FOUND)
		MESSAGE(STATUS "Python3 not found.")
		return()
	endif()

	cmake_path(GET Python3_LIBRARY_DIRS PARENT_PATH rootPath)
	file (GLOB_RECURSE python_license "${rootPath}*/LICENSE.txt" "${rootPath}*/LICENSE")

	if (NOT python_license)
		MESSAGE(STATUS "Failed to find Python3 License")
		return()
	endif()

	add_notices(Python3::Python 
		READABLE_NAME Python3
		LICENSE_FILE ${python_license}
		CREDITS "Scripting-Language: Python3 - https://www.python.org/")

	set_target_properties(Python3::Python PROPERTIES STDLIB ${Python3_STDLIB})

	set(Python3_VERSION_MAJOR ${Python3_VERSION_MAJOR} PARENT_SCOPE)
	set(Python3_VERSION_MINOR ${Python3_VERSION_MINOR} PARENT_SCOPE)

else()

	set(MADGINE_PYTHON_ROOT "" CACHE PATH "")
	
	set(pythonRoot ${MADGINE_PYTHON_ROOT})

	if (NOT pythonRoot)
		option(MADGINE_DISABLE_PYTHON_FETCH OFF "Disable automatic fetching of python binaries for cross-compilation")
		if (NOT MADGINE_DISABLE_PYTHON_FETCH)
			if (EMSCRIPTEN)
				FetchContent_Declare(
				  PythonEmscripten
				  URL      https://github.com/MadManStudios/MadgineDependencyArtifacts/releases/download/python-emscripten/python.zip
				)
				FetchContent_MakeAvailable(PythonEmscripten)
				set(pythonRoot ${pythonemscripten_SOURCE_DIR})
			elseif(ANDROID)
				if (ANDROID_ABI STREQUAL "arm64-v8a")
					FetchContent_Declare(
					  PythonAndroid
				      URL      https://www.python.org/ftp/python/3.14.5/python-3.14.5-aarch64-linux-android.tar.gz
					)
				elseif (ANDROID_ABI STREQUAL "x86_64")
					FetchContent_Declare(
					  PythonAndroid
				      URL      https://www.python.org/ftp/python/3.14.5/python-3.14.5-x86_64-linux-android.tar.gz
					)
				else ()
					MESSAGE(STATUS "Unsupported ANDROID_ABI provided for Python: ${ANDROID_ABI}")
					return()
				endif()
				FetchContent_MakeAvailable(PythonAndroid)
				set(pythonRoot ${pythonandroid_SOURCE_DIR})
			endif()
		endif()
		if (NOT pythonRoot)
			MESSAGE(STATUS "Cross-compiling targets require MADGINE_PYTHON_ROOT set or MADGINE_DISABLE_PYTHON_FETCH unset to link against Python")
			return()
		endif()
	endif()

	if (NOT EXISTS ${pythonRoot})
		MESSAGE(STATUS "${MADGINE_PYTHON_ROOT} (MADGINE_PYTHON_ROOT) does not exist")
		return()
	endif()

	file (GLOB_RECURSE python_license "${pythonRoot}*/LICENSE.txt" "${pythonRoot}*/LICENSE")
	if (NOT python_license)
		MESSAGE(WARNING "Failed to find Python3 License")		
		return()
	endif()

	file (GLOB_RECURSE include_file "${pythonRoot}/*/Python.h")
	if (NOT include_file)
		MESSAGE(WARNING "Failed to find Python.h header")		
		return()
	endif()
	cmake_path(GET include_file PARENT_PATH include_dir)

	file (GLOB_RECURSE os_file "${pythonRoot}/*/os.py")
	if (os_file)
		cmake_path(GET os_file PARENT_PATH stdlib)
	else()
		file (GLOB_RECURSE stdlib "${pythonRoot}*/python*.zip")		
	endif()
	
	if (NOT stdlib)
		MESSAGE(WARNING "Failed to find stdlib")		
		return()
	endif()

	file (GLOB_RECURSE static_library "${pythonRoot}*/libpython*${CMAKE_STATIC_LIBRARY_SUFFIX}")
	file (GLOB_RECURSE shared_library "${pythonRoot}*/libpython*.*${CMAKE_SHARED_LIBRARY_SUFFIX}")
	if (static_library)
		add_library(Python3::Python STATIC IMPORTED GLOBAL)

		set_target_properties(Python3::Python PROPERTIES
			IMPORTED_LOCATION ${static_library})
	elseif(shared_library)
		add_library(Python3::Python SHARED IMPORTED GLOBAL)

		set_target_properties(Python3::Python PROPERTIES
			IMPORTED_LOCATION ${shared_library})
	else()
		MESSAGE(WARNING "Failed to find python binaries")		
		return()
	endif()

		
	set_target_properties(Python3::Python PROPERTIES STDLIB ${stdlib})

	target_include_directories(Python3::Python INTERFACE ${include_dir})

	if (EMSCRIPTEN)
		target_link_libraries(Python3::Python INTERFACE -sUSE_BZIP2 -sUSE_SQLITE3)
	endif ()
	

	add_notices(Python3::Python 
		READABLE_NAME Python3
		LICENSE_FILE ${python_license}
		CREDITS "Scripting-Language: Python3 - https://www.python.org/")

endif()