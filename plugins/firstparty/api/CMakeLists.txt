Project(FirstParty)

set(sources "")
FolderSources(sources "" src)
	FolderSources(sources src Madgine firstpartyforward.h firstpartylib.h)
		FolderSources(sources Madgine firstparty firstpartyservices.cpp firstpartyservices.h)

add_plugin(FirstParty Madgine API ${sources} PRECOMPILED_HEADER src/Madgine/firstpartylib.h)

plugin_group(FirstParty EXCLUSIVE)

target_link_plugins(FirstParty Root)

