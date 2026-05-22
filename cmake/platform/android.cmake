include_guard(GLOBAL)

include (Plugins)

if (ANDROID)

	set (Android_List_dir ${workspace_file_dir}/cpack CACHE INTERNAL "")

	set (ANDROID_DEPENDENCIES "" CACHE INTERNAL "")
	set (ANDROID_RESOURCES "" CACHE INTERNAL "")
	set (ANDROID_SOURCES "" CACHE INTERNAL "")

	if (NOT ANDROID_SDK)
		MESSAGE(SEND_ERROR "No ANDROID_SDK location provided!")
	endif()

	MESSAGE(STATUS "Targeting Android-SDK version: ${ANDROID_PLATFORM_LEVEL}")

	macro(add_workspace_application target)

		set(target ${target})

		string(REGEX REPLACE "\\\\" "\\\\\\\\" ANDROID_SDK_ESCAPED "${ANDROID_SDK}")

		if (ANDROID_DEPENDENCIES)
			list(JOIN ANDROID_DEPENDENCIES "', '" ANDROID_ADDITIONAL_DEPENDENCIES)
			set(ANDROID_ADDITIONAL_DEPENDENCIES ", '${ANDROID_ADDITIONAL_DEPENDENCIES}'")			
		endif()
		
		if (ANDROID_SOURCES)
			list(JOIN ANDROID_SOURCES "', '" ANDROID_ADDITIONAL_SOURCES)
			set(ANDROID_ADDITIONAL_SOURCES "'${ANDROID_ADDITIONAL_SOURCES}'")			
		endif()

		if (ANDROID_RESOURCES)
			list(JOIN ANDROID_RESOURCES "', '" ANDROID_ADDITIONAL_RESOURCES)
			set(ANDROID_ADDITIONAL_RESOURCES ", '${ANDROID_ADDITIONAL_RESOURCES}'")			
		endif()

		configure_file(${Android_List_dir}/android/build.gradle.in build.gradle @ONLY)
		configure_file(${Android_List_dir}/android/local.properties.in local.properties @ONLY)
		configure_file(${Android_List_dir}/android/gradle.properties.in gradle.properties @ONLY)
		configure_file(${Android_List_dir}/android/AndroidManifest.xml.in AndroidManifest.xml.in @ONLY)
		configure_file(${Android_List_dir}/android/settings.gradle.in settings.gradle @ONLY)
		file(GENERATE OUTPUT AndroidManifest.xml INPUT ${CMAKE_CURRENT_BINARY_DIR}/AndroidManifest.xml.in)
		
		install(FILES ${CMAKE_CURRENT_BINARY_DIR}/build.gradle
					${CMAKE_CURRENT_BINARY_DIR}/local.properties
					${CMAKE_CURRENT_BINARY_DIR}/gradle.properties
					${CMAKE_CURRENT_BINARY_DIR}/settings.gradle
					${CMAKE_CURRENT_BINARY_DIR}/AndroidManifest.xml
				DESTINATION .
				COMPONENT ${target})

		add_library(${target} SHARED ${ARGN})

		if (NOT MODULES_ENABLE_PLUGINS)
			patch_toplevel_target(${target})
		endif()

	endmacro(add_workspace_application)

	macro(target_ship_folder target path)
		get_target_property(component ${target} INSTALL_COMPONENT)
		if (component)
			install(DIRECTORY ${path} DESTINATION assets COMPONENT ${component})
		endif()
	endmacro(target_ship_folder)
	
	macro(target_ship_file target path)
		get_target_property(component ${target} INSTALL_COMPONENT)
		if (component)
			install(FILES ${path} DESTINATION assets COMPONENT ${component})
		endif()
	endmacro(target_ship_file)

	function(install_to_workspace name)

		set(options)
		set(oneValueArgs)
		set(multiValueArgs TARGETS)
		cmake_parse_arguments(OPTIONS "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

		install(
			TARGETS ${OPTIONS_TARGETS} 
			LIBRARY DESTINATION bin/${CMAKE_ANDROID_ARCH_ABI} COMPONENT ${name}
			BUNDLE DESTINATION App COMPONENT ${name}
		)

		foreach(target ${OPTIONS_TARGETS})
			get_target_property(TARGET_SOURCE_DIR ${target} SOURCE_DIR)
			if (EXISTS ${TARGET_SOURCE_DIR}/data)
				install(DIRECTORY ${TARGET_SOURCE_DIR}/data DESTINATION assets COMPONENT ${name})
			endif()

			#target_include_directories(${target} INTERFACE $<INSTALL_INTERFACE:$<INSTALL_PREFIX>/${target}/include>)
		endforeach()

	endfunction(install_to_workspace)

	function(install_runtime_artifacts component)
		install(IMPORTED_RUNTIME_ARTIFACTS ${ARGN} LIBRARY DESTINATION bin/${CMAKE_ANDROID_ARCH_ABI} COMPONENT ${component})
	endfunction(install_runtime_artifacts)

endif()