include(Util)

once()

include (Plugins)

if (ANDROID)

	set (Android_List_dir ${CMAKE_CURRENT_LIST_DIR} CACHE INTERNAL "")

	set (ANDROID_DEPENDENCIES "" CACHE INTERNAL "")
	set (ANDROID_RESOURCES "" CACHE INTERNAL "")
	set (ANDROID_SOURCES "" CACHE INTERNAL "")

	if (NOT ANDROID_SDK)
		MESSAGE(SEND_ERROR "No ANDROID_SDK location provided!")
	endif()

	MESSAGE(STATUS "Targeting Android-SDK version: ${ANDROID_PLATFORM_LEVEL}")

	set (MADGINE_FORCE_DATA_COLLECT ON CACHE INTERNAL "")

	file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/gradle)

	if (CMAKE_HOST_WIN32)	
		add_custom_command(OUTPUT ${CMAKE_BINARY_DIR}/gradle/gradlew
			COMMAND gradle init
			COMMAND gradle wrapper --gradle-version=7.3.3 --distribution-type=all
			COMMAND gradle --stop
			WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/gradle)
	else()
		add_custom_command(OUTPUT ${CMAKE_BINARY_DIR}/gradle/gradlew
			COMMAND gradle init
			COMMAND gradle wrapper --gradle-version=7.3.3 --distribution-type=all			
			WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/gradle)
	endif()

	add_custom_target(gradlew DEPENDS ${CMAKE_BINARY_DIR}/gradle/gradlew)

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
		#configure_file(${Android_List_dir}/android/launch.vs.json.in ${CMAKE_SOURCE_DIR}/.vs/launch.vs.json @ONLY)
		#configure_file(${Android_List_dir}/android/debug_apk.bat.in debug_${target}_apk.bat @ONLY)
		#configure_file(${Android_List_dir}/android/launch_apk.sh.in launch_${target}_apk.sh @ONLY)
		#configure_file(${Android_List_dir}/android/gdbcommands.in gdbcommands_${target}_apk @ONLY)
		#configure_file(${Android_List_dir}/android/launch_jdb.bat.in launch_jdb.bat @ONLY)

		add_library(${target} SHARED ${ARGN})

		set_target_properties(${target} PROPERTIES
		LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/bin/${CMAKE_ANDROID_ARCH_ABI}
		LIBRARY_OUTPUT_DIRECTORY_DEBUG ${CMAKE_CURRENT_BINARY_DIR}/bin/${CMAKE_ANDROID_ARCH_ABI}
		LIBRARY_OUTPUT_DIRECTORY_RELEASE ${CMAKE_CURRENT_BINARY_DIR}/bin/${CMAKE_ANDROID_ARCH_ABI}
		LIBRARY_OUTPUT_DIRECTORY_RELWITHDEBINFO ${CMAKE_CURRENT_BINARY_DIR}/bin/${CMAKE_ANDROID_ARCH_ABI})

		if (NOT MODULES_ENABLE_PLUGINS)
			patch_toplevel_target(${target})
		endif()

		add_executable(${target}_apk android/dummy.cpp)

		target_link_libraries(${target}_apk PRIVATE ${target})
	
		if (CMAKE_HOST_WIN32)	
			add_custom_command(
				TARGET ${target}
				POST_BUILD				
				COMMAND ${CMAKE_BINARY_DIR}/gradle/gradlew assembleDebug
				COMMAND ${CMAKE_BINARY_DIR}/gradle/gradlew --stop
				COMMENT "Build APK - ${target}"			
				BYPRODUCTS apk/${target}-debug.apk
			)	
		else()
			add_custom_command(
				TARGET ${target}
				POST_BUILD				
				COMMAND ${CMAKE_BINARY_DIR}/gradle/gradlew assembleDebug
				COMMENT "Build APK - ${target}"			
				BYPRODUCTS apk/${target}-debug.apk
			)		
		endif()

		add_dependencies(${target} gradlew)

	endmacro(add_workspace_application)

endif()