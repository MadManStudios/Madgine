


set(sources parser.cpp parser.h)
	

add_workspace_library(TemplateEngine ${sources} SOURCE_ROOT ..)

target_link_libraries(TemplateEngine PUBLIC Meta Interfaces)

install_to_workspace(TemplateEngine TARGETS TemplateEngine)