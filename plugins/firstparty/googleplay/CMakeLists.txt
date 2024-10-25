Project(GooglePlayServices)

add_subdirectory(shared)

if (NOT TARGET PlayServices)
	return()
endif()



MESSAGE(STATUS "--- Google Play ---")

if (NOT EXISTS ${MADGINE_CONFIGURATION}/res/values/games-ids.xml)
	MESSAGE(STATUS "No games-ids.xml provided. Plugin disabled. Add res/values/games-ids.xml to your MADGINE_CONFIGURATION location.")
	MESSAGE(STATUS "--- Google Play End ---")
	return()
endif ()

#MESSAGE(STATUS "Creating steam_appid.txt for Debug build.")
#file(WRITE ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/steam_appid.txt ${Steam_AppId})

#MESSAGE(STATUS "AppId: ${Steam_AppId}")

MESSAGE(STATUS "--- Google Play End ---")

set(sources "")
FolderSources(sources "" src)
FolderSources(sources src Madgine)
	FolderSources(sources Madgine firstparty googleplayservices.cpp googleplayservices.h)

add_plugin(GooglePlayServices Madgine FirstParty ${sources} PRECOMPILED_HEADER src/Madgine/googleplayserviceslib.h)

target_link_plugins(GooglePlayServices FirstParty)

target_link_libraries(GooglePlayServices PRIVATE PlayServices)


