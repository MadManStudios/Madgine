
include(util/config)

set(MADGINE_LAUNCHER_SPLASH_IMAGE "" CACHE FILEPATH "Path to an image file for the applications splash screen")
set(MADGINE_LAUNCHER_WINDOW_TITLE "" CACHE STRING "Title of the Main window of the game")
set(MADGINE_LAUNCHER_ICON "${CMAKE_CURRENT_SOURCE_DIR}/madgineIcon.ico" CACHE FILEPATH "Path to an image file for the applications tray icon")

set(sources "")
FolderSources(sources "" "" launcher.cpp main.cpp terminate_handler.cpp)
if (EMSCRIPTEN)
	FolderSources(sources "" emscripten emscripten_main.cpp)
endif(EMSCRIPTEN)
if (ANDROID)
	FolderSources(sources "" android android_main.cpp androidlauncher.cpp androidlauncher.h)
endif(ANDROID)
if (OSX)
    FolderSources(sources "" osx osx_main.mm)
endif (OSX)
if (IOS)
	FolderSources(sources "" ios ios_main.mm)
endif (IOS)
if (WINDOWS)
	FolderSources(sources "" windows windows_main.cpp)
endif (WINDOWS)

add_workspace_application(${MADGINE_BINARY_NAME} ${sources})

set_property(DIRECTORY ${CMAKE_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${MADGINE_BINARY_NAME})


MESSAGE("Using binary name: ${MADGINE_BINARY_NAME}")


set_target_properties(${MADGINE_BINARY_NAME} PROPERTIES 
		PLUGIN_BASE Madgine
		)

target_link_plugins(${MADGINE_BINARY_NAME} App Client Handler Root)
if (MADGINE_SERVER)
	target_sources(${MADGINE_BINARY_NAME} PRIVATE server.cpp)
	target_link_plugins(${MADGINE_BINARY_NAME} Server)
endif()

if (EMSCRIPTEN)
	_target_link_libraries(${MADGINE_BINARY_NAME} PRIVATE "--shell-file ${CMAKE_CURRENT_SOURCE_DIR}/emscripten/template.html")
elseif (ANDROID)
	target_link_libraries(${MADGINE_BINARY_NAME} PRIVATE android)	
elseif (OSX)
    target_link_libraries(${MADGINE_BINARY_NAME} PRIVATE "-framework Cocoa")
elseif (IOS)
    target_link_libraries(${MADGINE_BINARY_NAME} PRIVATE "-framework UIKit")
endif()


set_config(MADGINE_LAUNCHER_WINDOW_TITLE LAUNCHER WINDOW_TITLE)
set_config(MADGINE_LAUNCHER_SPLASH_IMAGE LAUNCHER SPLASH_IMAGE)
set_config(MADGINE_LAUNCHER_ICON LAUNCHER ICON)


if (WINDOWS)
	configure_file(windows/resources.in.rc resources.rc)
	configure_file(windows/resources.in.h resources.h)
	target_sources(${MADGINE_BINARY_NAME} PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/resources.rc)
endif ()


configure_file(launcherconfig.in.h launcherconfig.h)
target_include_directories(${MADGINE_BINARY_NAME} PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>)


set(sources "")
FolderSources(sources "" tools)
	FolderSources(sources tools Madgine_Tools)
		FolderSources(sources Madgine_Tools launcher launchertool.cpp launchertool.h)

add_tools(${MADGINE_BINARY_NAME} ${sources} PRECOMPILED_HEADER tools/Madgine_Tools/launchertoolslib.h NO_DEFAULT_LINK)

target_link_plugins(${MADGINE_BINARY_NAME}Tools ClientTools)


target_depend_on_all_plugins(${MADGINE_BINARY_NAME})

generate_binary_info(${MADGINE_BINARY_NAME})

install_to_workspace(Madgine TARGETS ${MADGINE_BINARY_NAME})

write_notices(${MADGINE_BINARY_NAME})


if (TARGET copy_data)
	add_dependencies(${MADGINE_BINARY_NAME} copy_data)
endif()


