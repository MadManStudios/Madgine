Project(Modules VERSION 1.0.0)

option(MODULES_ENABLE_TASK_TRACKING "" OFF)

CMAKE_DEPENDENT_OPTION(MODULES_ENABLE_THREADING "Enable multithreading utilities like thread local or workgroups" ON
                       "NOT EMSCRIPTEN" OFF)

CMAKE_DEPENDENT_OPTION(MODULES_USE_CUSTOM_THREADLOCAL "Replace the thread_local keyword with a custom implementation for thread-local variables." ON "MODULES_ENABLE_THREADING" OFF)
option(MODULES_USE_PTHREAD_THREADLOCAL_STORE "Use pthread_[set/get]specific instead of thread_local to store all thread-local variables. Useful in combination with MODULES_USE_CUSTOM_THREADLOCAL on platforms that don't support thread_local.")


set(sources)

FolderSources(sources "" "" modulesforward.h moduleslib.h)
	FolderSources(sources "" debug)
		FolderSources(sources debug profiler processstats.cpp processstats.h profile.h profiler.cpp profiler.h profilerthread.cpp profilerthread.h staticprocess.cpp staticprocess.h)
if (MODULES_ENABLE_TASK_TRACKING)
		FolderSources(sources debug tasktracking tasktracker.cpp tasktracker.h)
endif()
	FolderSources(sources "" ini inifile.cpp inifile.h inisection.cpp inisection.h)
if (MODULES_ENABLE_PLUGINS)
	FolderSources(sources "" plugins pluginmanager.cpp pluginmanager.h pluginsection.cpp pluginsection.h plugin.cpp plugin.h)
endif()
	FolderSources(sources "" threading customclock.cpp customclock.h datamutex.cpp datamutex.h global.h globalstorage.h scheduler.cpp scheduler.h task.h taskhandle.cpp taskhandle.h taskpromise.cpp taskpromise.h taskpromisesharedstate.cpp taskpromisesharedstate.h taskqueue.cpp taskqueue.h threadlocal.h threadstorage.cpp threadstorage.h workgroup.cpp workgroup.h workgrouphandle.h workgroupstorage.cpp workgroupstorage.h)
		FolderSources(sources threading awaitables awaitabletimepoint.cpp awaitabletimepoint.h)
	FolderSources(sources "" uniquecomponent indexholder.h uniquecomponent.h uniquecomponentcollector.h uniquecomponentcontainer.h uniquecomponentcollectormanager.cpp uniquecomponentcollectormanager.h uniquecomponentregistry.cpp uniquecomponentregistry.h)
	

add_workspace_library(Modules ${sources} SOURCE_ROOT .. PRECOMPILED_HEADER moduleslib.h)

configure_file(modulesconfig.in.h modulesconfig.h)
target_include_directories(Modules PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>)

target_link_libraries(Modules PUBLIC Interfaces)

if (WINDOWS)
	if (NOT BUILD_SHARED_LIBS AND MINGW)
		target_link_libraries (Modules PRIVATE -Wl,-Bstatic,--whole-archive pthread -Wl,-Bdynamic,--no-whole-archive)
	else()
		find_package(Threads)
		target_link_libraries (Modules PRIVATE ${CMAKE_THREAD_LIBS_INIT})
	endif()
endif()

if (MSVC)
	target_link_options(Modules PUBLIC "/NATVIS:${CMAKE_CURRENT_LIST_DIR}/Modules.natvis")
endif()

generate_binary_info(Modules)

install_to_workspace(Madgine TARGETS Modules EXPORT_LIB)

target_compile_definitions(Modules PRIVATE 
		PLUGIN_GROUP_DEFINITIONS=$<JOIN:$<TARGET_PROPERTY:Modules,PLUGIN_GROUP_DEFINITIONS>,,>)
