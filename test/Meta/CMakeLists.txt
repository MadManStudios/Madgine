
project(Meta_Test)


set(sources "")
FolderSources(sources "" "" Meta_gtest_main.cc)
	FolderSources(sources "" keyvalue scopeptrtest.cpp valuetypetest.cpp)
	FolderSources(sources "" math matrixtest.cpp normaltest.cpp quaterniontest.cpp)
	FolderSources(sources "" serialize calltest.cpp querytest.cpp testunit.cpp)
		FolderSources(sources serialize container containertest.cpp)
		FolderSources(sources serialize formatter formattertestbase.cpp iniformattertest.cpp jsonformattertest.cpp safebinaryformattertest.cpp unsafebinaryformattertest.cpp xmlformattertest.cpp)
		FolderSources(sources serialize primitives bytebuffertest.cpp)
		FolderSources(sources serialize table serializetabletest.cpp)	

add_gtest(Meta_Test ${sources})

target_link_libraries(Meta_Test PRIVATE Interfaces Meta CLI)
