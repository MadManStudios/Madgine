Project(Scene)

set(sources "")
FolderSources(sources "" src)
	FolderSources(sources src Madgine sceneforward.h scenelib.h)
		FolderSources(sources Madgine scene scenemanager.cpp scenemanager.h scenecomponentcollector.cpp scenecomponentcollector.h scenecontainer.cpp scenecontainer.h sceneloader.cpp sceneloader.h scenecomponentbase.cpp scenecomponentbase.h)
			FolderSources(sources scene entity entitycomponentbase.cpp entitycomponentbase.h entitycomponentcollector.cpp entitycomponentcollector.h entity.cpp entity.h entitycomponent.h entitycomponentlistbase.cpp entitycomponentlist.cpp entitycomponenthandle.cpp entitycomponenthandle.h entityhandle.cpp entityhandle.h entityptr.cpp entityptr.h)
				FolderSources(sources entity components material.cpp material.h mesh.cpp mesh.h pointlight.cpp pointlight.h skeleton.cpp skeleton.h transform.cpp transform.h)
			FolderSources(sources scene behavior scenesenders.cpp scenesenders.h transform.cpp transform.h animation.cpp animation.h)

add_plugin(Scene Madgine Misc ${sources} PRECOMPILED_HEADER src/Madgine/scenelib.h)

target_link_plugins(Scene MeshLoader AnimationLoader App Behavior)

target_link_libraries(Scene PRIVATE MemorySerialize)

if (MSVC)
	target_link_options(Scene PUBLIC "/NATVIS:${CMAKE_CURRENT_LIST_DIR}/src/Scene.natvis")
endif()

Project(SceneRenderer)

set(sources "")
FolderSources(sources "" renderer)
	FolderSources(sources renderer Madgine scenerendererforward.h scenerendererlib.h)
		FolderSources(sources Madgine render pointshadowrenderdata.cpp pointshadowrenderdata.h pointshadowrenderpass.cpp pointshadowrenderpass.h scenemainwindowcomponent.cpp scenemainwindowcomponent.h scenerenderdata.cpp scenerenderdata.h scenerenderpass.cpp scenerenderpass.h shadowrenderpass.cpp shadowrenderpass.h shadowscenerenderdata.cpp shadowscenerenderdata.h litscenerenderdata.cpp litscenerenderdata.h)
			FolderSources(sources render shaders light.hlsl pointshadow.hlsl scene.hlsl)

add_plugin(SceneRenderer Madgine Misc ${sources} SOURCE_ROOT renderer PRECOMPILED_HEADER renderer/Madgine/scenerendererlib.h NO_DATA)

target_link_plugins(SceneRenderer Scene Client)

target_include_directories(SceneRenderer PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

set(sources "")
FolderSources(sources "" tools)
	FolderSources(sources tools Madgine_Tools)
		FolderSources(sources Madgine_Tools sceneeditor entitycache.cpp entitycache.h gridpass.cpp gridpass.h sceneeditor.cpp sceneeditor.h scenefile.cpp scenefile.h scenetool.cpp scenetool.h sceneview.cpp sceneview.h)
			FolderSources(sources sceneeditor shaders grid.hlsl)
			FolderSources(sources sceneeditor tests sceneeditortests.cpp sceneeditortests.h)

add_tools(SceneRenderer ${sources} PRECOMPILED_HEADER tools/Madgine_Tools/scenerenderertoolslib.h SOURCE_ROOT tools)

target_link_plugins(SceneRendererTools RenderTools ClientTools DebuggerTools BehaviorTools ResourcesTools)

target_link_libraries(SceneRendererTools PRIVATE MemorySerialize FilesystemSerialize)

target_include_directories(SceneRendererTools PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
