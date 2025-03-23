project(TextEditor)

add_subdirectory(shared)

set(sources "")
FolderSources(sources "" src)
FolderSources(sources src Madgine_Tools texteditorlib.h texteditorforward.h)
	FolderSources(sources Madgine_Tools texteditor interactiveprompt.cpp interactiveprompt.h textdocument.cpp textdocument.h texteditor.cpp texteditor.h)
		
add_plugin(TextEditor Madgine Tools ${sources} PRECOMPILED_HEADER src/Madgine_Tools/texteditorlib.h)

target_link_plugins(TextEditor Tools)

target_link_libraries(TextEditor PUBLIC Zep)





