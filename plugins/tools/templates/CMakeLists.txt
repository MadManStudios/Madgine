project(Templates)

add_subdirectory(shared)

set(sources "")
FolderSources(sources "" src)
FolderSources(sources src Madgine_Tools templateslib.h templatesforward.h)
	FolderSources(sources Madgine_Tools templates templates.cpp templates.h)
		
add_plugin(Templates Madgine Utility ${sources} PRECOMPILED_HEADER src/Madgine_Tools/templateslib.h)

target_link_plugins(Templates Tools)

target_link_libraries(Templates PUBLIC TemplateEngine)



