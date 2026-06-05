
project(Interfaces_Test)

set(sources "")
FolderSources(sources "" "" Interfaces_gtest_main.cc)
	FolderSources(sources "" filesystem pathtest.cpp)

add_gtest(Interfaces_Test ${sources})

target_link_libraries(Interfaces_Test PRIVATE Platform)
