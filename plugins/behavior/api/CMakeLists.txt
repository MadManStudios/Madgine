Project(Behavior)

set(sources "")
FolderSources(sources "" src)
	FolderSources(sources src Madgine behaviorforward.h behaviorlib.h)
		FolderSources(sources Madgine behavior behavior.cpp behavior.h behaviorcollector.cpp behaviorcollector.h behaviorcoroutine.cpp behaviorcoroutine.h behaviordescriptor.cpp behaviordescriptor.h behaviorhandle.cpp behaviorhandle.h behaviorlist.cpp behaviorlist.h behaviorreceiver.cpp behaviorreceiver.h behaviorsender.cpp behaviorsender.h dynamicparametertuple.cpp dynamicparametertuple.h nativebehaviorcollector.cpp nativebehaviorcollector.h parametertuple.cpp parametertuple.h)
		
add_plugin(Behavior Madgine Core ${sources} PRECOMPILED_HEADER src/Madgine/behaviorlib.h API_PLUGIN)

target_link_libraries(Behavior PUBLIC Meta Modules)
target_link_plugins(Behavior Debugger)


set(sources "")
FolderSources(sources "" tools)
	FolderSources(sources tools Madgine_Tools behaviortoolsforward.h behaviortoolslib.h behaviortool.cpp behaviortool.h)

add_tools(Behavior ${sources} PRECOMPILED_HEADER tools/Madgine_Tools/behaviortoolslib.h)

target_link_plugins(BehaviorTools Behavior DebuggerTools TextEditor)
