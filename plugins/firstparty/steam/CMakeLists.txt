Project(SteamServices)

add_subdirectory(shared)

if (NOT TARGET Steamworks)
	return()
endif()



MESSAGE(STATUS "--- Steam ---")

if (NOT Steam_AppId)
	MESSAGE(STATUS "Using default AppId. Set 'Steam_AppId' in your toplevel CMakeLists file.")
	set (Steam_AppId 480)
endif ()

MESSAGE(STATUS "Creating steam_appid.txt for Debug build.")
file(WRITE ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/steam_appid.txt ${Steam_AppId})

MESSAGE(STATUS "AppId: ${Steam_AppId}")

MESSAGE(STATUS "--- Steam End ---")




set(sources "")
FolderSources(sources "" src)
FolderSources(sources src Madgine)
	FolderSources(sources Madgine firstparty steamservices.cpp steamservices.h steamstreambuf.cpp steamstreambuf.h steamstreamdata.cpp steamstreamdata.h steamsyncmanager.cpp steamsyncmanager.h)

add_plugin(SteamServices Madgine FirstParty ${sources} PRECOMPILED_HEADER src/Madgine/steamserviceslib.h)

target_link_plugins(SteamServices FirstParty)

target_link_libraries(SteamServices PUBLIC Steamworks)

target_compile_definitions(SteamServices PRIVATE STEAM_APP_ID=${Steam_AppId})

