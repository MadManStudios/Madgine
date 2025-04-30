
project(Modules_Test)


set(sharedSources "")
	FolderSources(sharedSources "" uniquecomponent uniquecomponentshared.cpp uniquecomponentshared.h)

add_workspace_library(TestShared ${sharedSources} SOURCE_ROOT .)

target_link_libraries(TestShared PUBLIC Modules)

generate_binary_info(TestShared)

install_to_workspace(TestShared TARGETS TestShared)


set(libASources "")
	FolderSources(libASources "" uniquecomponent libA.cpp)

add_plugin(LibA Test Test ${libASources} SOURCE_ROOT .)

target_link_libraries(LibA PUBLIC TestShared)


set(libBSources "")
	FolderSources(libBSources "" uniquecomponent libB.cpp)

add_plugin(LibB Test Test ${libBSources} SOURCE_ROOT .)

target_link_plugins(LibB LibA)


set(sources "")
FolderSources(sources "" "" Modules_gtest_main.cc)	
	FolderSources(sources "" uniquecomponent uniquecomponentshared.h uniquecomponenttest.cpp)		

add_gtest(Modules_Test ${sources})

target_link_libraries(Modules_Test PRIVATE Modules)

target_link_plugins(Modules_Test LibB)

generate_binary_info(Modules_Test)
