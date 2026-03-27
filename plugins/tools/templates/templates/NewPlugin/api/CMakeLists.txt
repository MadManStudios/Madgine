

set(sources "")
FolderSources(sources "" src)	

add_plugin(%name% %authorTag% API ${sources} PRECOMPILED_HEADER %name#lc_string%lib.h)

target_link_plugins(%name%)