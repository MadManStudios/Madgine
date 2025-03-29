
project(Generic_Test)

set(sources "")
FolderSources(sources "" "" Generic_gtest_main.cc)
	FolderSources(sources "" generic areaviewtest.cpp tupleunpackertest.cpp)
		FolderSources(sources generic bits bitstest.cpp)
		FolderSources(sources generic container generationvectortest.cpp)
		FolderSources(sources generic execution executiontest.cpp)
	

add_gtest(Generic_Test ${sources})

target_link_libraries(Generic_Test PRIVATE Interfaces CLI)
