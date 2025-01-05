Project(Debugger)

set(sources)
FolderSources(sources "" src)
	FolderSources(sources src Madgine debugforward.h debuglib.h)
		FolderSources(sources Madgine debug debuggablelifetime.cpp debuggablelifetime.h debuggablesender.cpp debuggablesender.h debugger.cpp debugger.h debuglocation.cpp debuglocation.h)

	
add_plugin(Debugger Madgine Core ${sources} PRECOMPILED_HEADER src/Madgine/debuglib.h)

target_link_plugins(Debugger Root)


set(sources "")
FolderSources(sources "" tools)
	FolderSources(sources tools Madgine_Tools)
		FolderSources(sources Madgine_Tools debugger debuggerview.cpp debuggerview.h)			

add_tools(Debugger ${sources} PRECOMPILED_HEADER tools/Madgine_Tools/debugtoolslib.h)

target_link_plugins(DebuggerTools Tools)
