

Project(%name%)

set(sources "")

FolderSources(sources "" src )	

add_plugin(%name% %name% Game ${sources} PRECOMPILED_HEADER src/%name#lc_string%lib.h)

target_link_plugins(%name% )