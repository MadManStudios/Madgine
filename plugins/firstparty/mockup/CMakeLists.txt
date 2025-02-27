Project(MockupServices)


set(sources "")
FolderSources(sources "" src)
FolderSources(sources src Madgine)
	FolderSources(sources Madgine firstparty mockupstate.cpp mockupstate.h)


add_workspace_library(MockupLib SHARED ${sources} PRECOMPILED_HEADER src/Madgine/mockupliblib.h)

target_link_libraries(MockupLib PUBLIC NetworkSerialize FirstParty)

install_to_workspace(MockupLib TARGETS MockupLib EXPORT_LIB)
export_to_workspace(MockupLib)


set(sources "")
FolderSources(sources "" src)
FolderSources(sources src Madgine)
	FolderSources(sources Madgine firstparty mockupservices.cpp mockupservices.h)

add_plugin(MockupServices Madgine FirstParty ${sources} PRECOMPILED_HEADER src/Madgine/mockupserviceslib.h)

target_link_plugins(MockupServices FirstParty)

target_link_libraries(MockupServices PRIVATE MockupLib)

get_target_property(exclude MockupServices EXCLUDE_FROM_ALL)


set(sources "")
FolderSources(sources "" src)
FolderSources(sources src Madgine)
	FolderSources(sources Madgine firstparty mockupserver.cpp)


add_workspace_executable(MockupServer ${sources})

target_link_libraries(MockupServer PRIVATE MockupLib)

set_target_properties(MockupServer MockupLib PROPERTIES EXCLUDE_FROM_ALL ${exclude})