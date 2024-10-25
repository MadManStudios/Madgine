Project(NodeGraph)

add_subdirectory(shared)

set(sources "")
FolderSources(sources "" src)
	FolderSources(sources src Madgine nodegraphforward.h nodegraphlib.h)
		FolderSources(sources Madgine nodegraph codegenerator.cpp codegenerator.h nodegraph.cpp nodegraph.h nodegraphloader.cpp nodegraphloader.h nodeexecution.cpp nodeexecution.h nodebase.cpp nodebase.h nodecollector.cpp nodecollector.h nodeinterpreter.cpp nodeinterpreter.h)
			FolderSources(sources nodegraph nodes accessornode.cpp accessornode.h functionnode.cpp functionnode.h pumpnode.cpp pumpnode.h testnode.cpp testnode.h)
				FolderSources(sources nodes util librarynode.cpp sendernode.cpp)
			
add_plugin(NodeGraph Madgine Behavior ${sources} PRECOMPILED_HEADER src/Madgine/nodegraphlib.h)

target_link_plugins(NodeGraph Resources Debugger Behavior)
target_link_libraries(NodeGraph PUBLIC FilesystemSerialize CodeGen )



set(sources "")
FolderSources(sources "" tools)
	FolderSources(sources tools Madgine_Tools nodegraphtoolsforward.h nodegraphtoolslib.h)
		FolderSources(sources Madgine_Tools nodegraph debugvisualizer.cpp debugvisualizer.h imguihelpers.cpp imguihelpers.h nodegrapheditor.cpp nodegrapheditor.h)

add_tools(NodeGraph ${sources} PRECOMPILED_HEADER tools/Madgine_Tools/nodegraphtoolslib.h)

target_link_plugins(NodeGraphTools Tools DebuggerTools BehaviorTools)
target_link_libraries(NodeGraphTools PUBLIC imgui_node_editor)