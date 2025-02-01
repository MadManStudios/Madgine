
Project(Render)     

add_subdirectory(shared)

set(sources "")
FolderSources(sources "" src)
	FolderSources(sources src Madgine renderforward.h renderlib.h)
		FolderSources(sources Madgine render camera.cpp camera.h pipelineloader.cpp pipelineloader.h rendercontext.cpp rendercontext.h renderdata.cpp renderdata.h renderpass.cpp renderpass.h rendertarget.cpp rendertarget.h rendercontextcollector.cpp rendercontextcollector.h textureloader.cpp textureloader.h)	
			FolderSources(sources render fonts font.h fontloader.cpp fontloader.h glyph.cpp glyph.h)
			FolderSources(sources render passes bloompass.cpp bloompass.h blurpass.cpp blurpass.h)
				FolderSources(sources passes shaders bloom.PS_hlsl bloom.VS_hlsl blur.PS_hlsl blur.VS_hlsl)
			FolderSources(sources render shadinglanguage sl_support_begin.h sl_support_end.h)
		
add_plugin(Render Madgine Core ${sources} PRECOMPILED_HEADER src/Madgine/renderlib.h)

plugin_group(Renderer EXCLUSIVE ATLEAST_ONE)

target_link_plugins(Render Debugger MeshLoader ImageLoader)
target_link_plugin_groups(Render Renderer)

target_link_libraries(Render PUBLIC RenderLib Meta Modules CodeGen FilesystemSerialize MemorySerialize)
target_link_libraries(Render PRIVATE freetype lib_msdfgen)



set(sources "")
FolderSources(sources "" tools)
	FolderSources(sources tools Madgine_Tools interactivecamera.cpp interactivecamera.h)
		FolderSources(sources Madgine_Tools render rendercontexttool.cpp rendercontexttool.h)

add_tools(Render ${sources} PRECOMPILED_HEADER tools/Madgine_Tools/rendertoolslib.h)

target_link_plugins(RenderTools Tools)

