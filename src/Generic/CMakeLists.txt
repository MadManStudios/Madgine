Project(Generic VERSION 1.0.0)

add_workspace_interface_library(Generic SOURCE_ROOT ..)

configure_file(genericconfig.in.h genericconfig.h)
target_include_directories(Generic INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>)

if (MSVC)
	target_link_options(Generic INTERFACE "/NATVIS:${CMAKE_CURRENT_LIST_DIR}/Generic.natvis")
endif()

install_interface_to_workspace(Madgine TARGETS Generic SOURCE_ROOT ..)
