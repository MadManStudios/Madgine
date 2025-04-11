Project(DirectX12)


set(sources "")
FolderSources(sources "" src)
FolderSources(sources src DirectX12 directx12forward.h directx12lib.h directx12meshdata.h directx12meshloader.cpp directx12meshloader.h directx12pixelshaderloader.cpp directx12pixelshaderloader.h directx12pipelineloader.cpp directx12pipelineloader.h directx12rendertexture.cpp directx12rendertexture.h directx12renderwindow.cpp directx12renderwindow.h directx12vertexshaderloader.cpp directx12vertexshaderloader.h directx12textureloader.cpp directx12textureloader.h directx12rendertarget.cpp directx12rendertarget.h directx12rendercontext.cpp directx12rendercontext.h directx12shadercodegen.cpp directx12shadercodegen.h)
	FolderSources(sources DirectX12 util dx12dump.cpp directx12buffer.cpp directx12buffer.h directx12commandallocator.cpp directx12commandallocator.h directx12commandlist.cpp directx12commandlist.h directx12descriptorheap.cpp directx12descriptorheap.h directx12heapallocator.cpp directx12heapallocator.h directx12texture.cpp directx12texture.h directx12pipeline.cpp directx12pipeline.h directx12pipelineinstance.cpp directx12pipelineinstance.h directx12queryheap.cpp directx12queryheap.h)

add_plugin(DirectX12 Madgine Renderer ${sources} PRECOMPILED_HEADER src/DirectX12/directx12lib.h)

target_link_libraries(DirectX12 PRIVATE d3d12 dxgi dxguid Render dxcompiler)

add_custom_command(TARGET DirectX12 POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy $<PATH:GET_PARENT_PATH,$<TARGET_PROPERTY:ShaderGen,IMPORTED_LOCATION>>/dxcompiler.dll ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

FetchContent_Declare(
  DXC_DXIL 
  URL https://github.com/microsoft/DirectXShaderCompiler/releases/download/v1.8.2502/dxc_2025_02_20.zip
)

FetchContent_MakeAvailable(DXC_DXIL)

add_custom_command(TARGET DirectX12 POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy ${dxc_dxil_SOURCE_DIR}/bin/x64/dxil.dll ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

set(sources "")
FolderSources(sources "" tools)
	FolderSources(sources tools DirectX12_Tools directx12rendercontexttool.cpp directx12rendercontexttool.h directx12toolsforward.h directx12toolslib.h)
		#FolderSources(sources DirectX12_Tools imgui imgui_impl_dx12.cpp imgui_impl_dx12.h directx12imroot.cpp directx12imroot.h)


add_tools(DirectX12 ${sources} PRECOMPILED_HEADER tools/DirectX12_Tools/directx12toolslib.h)

target_link_plugins(DirectX12Tools RenderTools)
target_link_libraries(DirectX12Tools PRIVATE d3d12 dxgi dxcompiler)

add_shadergen_target(DirectX12 -HLSL12)

add_subdirectory(util)