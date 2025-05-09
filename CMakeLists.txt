cmake_minimum_required (VERSION 3.8)

cmake_policy(VERSION 3.18)
cmake_policy(SET CMP0048 NEW)
cmake_policy(SET CMP0011 NEW)
cmake_policy(SET CMP0049 NEW)
cmake_policy(SET CMP0070 NEW)

if (NOT CXX_STANDARD_OVERRIDE)
	set(CMAKE_CXX_STANDARD 20 CACHE INTERNAL "")
else()
	set(CMAKE_CXX_STANDARD ${CXX_STANDARD_OVERRIDE} CACHE INTERNAL "")
endif()
set(CMAKE_CXX_STANDARD_REQUIRED ON CACHE INTERNAL "")
set(CMAKE_CXX_EXTENSIONS OFF CACHE INTERNAL "")


list(APPEND CMAKE_MODULE_PATH
	"${CMAKE_CURRENT_SOURCE_DIR}/cmake"
	"${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules"
)

if (MADGINE_AS_SDK)
	set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} PARENT_SCOPE)
endif(MADGINE_AS_SDK)

set (MADGINE_CONFIGURATION "" CACHE FILEPATH "A directory containing configuration data for a release build of the engine")

if (MADGINE_CONFIGURATION)
	if (NOT EXISTS ${MADGINE_CONFIGURATION})
		MESSAGE(FATAL_ERROR "The provided directory for MADGINE_CONFIGURATION does not exist: ${MADGINE_CONFIGURATION}")
	endif ()
	if (NOT IS_ABSOLUTE ${MADGINE_CONFIGURATION})
		set (MADGINE_CONFIGURATION ${CMAKE_SOURCE_DIR}/${MADGINE_CONFIGURATION} CACHE FILEPATH "A directory containing configuration data for a release build of the engine" FORCE)
	endif ()

	file(GLOB configs "${MADGINE_CONFIGURATION}/*.cmake")
	foreach(config ${configs})
		message(STATUS "Adding configuration-code: ${config}")
		include (${config})
	endforeach()

	include(util/ini)

	file(GLOB inis "${MADGINE_CONFIGURATION}/*.ini")
	foreach(ini ${inis})
		get_filename_component(name ${ini} NAME_WE)
		string(TOUPPER ${name} name)
		read_ini_file(${ini} ${name})
	endforeach()

	cmake_path(GET MADGINE_CONFIGURATION FILENAME MADGINE_CONFIGURATION_NAME)
	set(MADGINE_CONFIGURATION_SUFFIX "-${MADGINE_CONFIGURATION_NAME}")
endif()



if (DEFINED ENV{MADGINE_VERSION})
	set (MADGINE_VERSION $ENV{MADGINE_VERSION})
else()
	include(util/GitVersion)
	get_version_from_git(version)
	set (MADGINE_VERSION ${version})
endif()


project(Madgine VERSION ${MADGINE_VERSION})



include(Workspace)
include(Packaging)
include(PrecompiledHeader)
include(util/Project)
include(Plugins)
include(Tools)
include(Testing)
include(Legal)
include(Dependencies)

enable_packaging()

add_subdirectory(util)

add_subdirectory(src)

add_subdirectory(plugins)

add_subdirectory(samples)

resolve_dependencies()

set (LAUNCHER_LOCATION ${CMAKE_CURRENT_LIST_DIR}/launcher CACHE INTERNAL "")
function(create_launcher MADGINE_BINARY_NAME)
	if (TARGET MadgineTooling)
		set_target_properties(MadgineTooling PROPERTIES BinaryName ${MADGINE_BINARY_NAME})
	endif()
	set(MADGINE_SERVER OFF)
	add_subdirectory(${LAUNCHER_LOCATION} launcher)
endfunction()
function(create_server MADGINE_BINARY_NAME)
	if (TARGET MadgineTooling)
		set_target_properties(MadgineTooling PROPERTIES BinaryName ${MADGINE_BINARY_NAME})
	endif()
	set(MADGINE_SERVER ON)
	add_subdirectory(${LAUNCHER_LOCATION} server)
endfunction()

if (NOT MADGINE_AS_SDK)
	add_subdirectory(docs)
	create_launcher(MadgineLauncher)
	if (MODULES_ENABLE_PLUGINS OR PLUGINSELECTION_Core_Server)		
		create_server(MadgineServer)
	endif()
endif()


if (BUILD_TESTING)
	add_subdirectory(test)
endif()


set (CPACK_NSIS_MODIFY_PATH ON)
set (CPACK_PACKAGE_VENDOR "MadMan Studios")

#set (CPACK_PACKAGE_EXECUTABLES "Maditor;Maditor")


cpack_add_component(MadgineLauncher REQUIRED)
cpack_add_component(Runtime REQUIRED)

get_cmake_property(CPACK_COMPONENTS_ALL COMPONENTS)
list(REMOVE_ITEM CPACK_COMPONENTS_ALL "assimp_dev" "libassimp5.2.4" "libassimp5.2.4_dev" "Trash" "LibA" "LibB" "TestShared")


cpack_add_component_group(APIGroup
	DISPLAY_NAME API)			
cpack_add_component_group(AudioGroup
	DISPLAY_NAME Audio
	PARENT_GROUP APIGroup)
cpack_add_component(Audio
	DISPLAY_NAME API
	GROUP AudioGroup)
cpack_add_component(PortAudioApi	
	GROUP AudioGroup)
cpack_add_component_group(CoreGroup
	DISPLAY_NAME Core)			
cpack_add_component_group(ClientGroup
	DISPLAY_NAME Client
	PARENT_GROUP CoreGroup)
cpack_add_component(Client
	DISPLAY_NAME API
	GROUP ClientGroup
	REQUIRED)		
cpack_add_component_group(RootGroup
	DISPLAY_NAME Root
	PARENT_GROUP CoreGroup)
cpack_add_component(Root
	DISPLAY_NAME API
	GROUP RootGroup
	REQUIRED)		
cpack_add_component_group(ServerGroup
	DISPLAY_NAME Server
	PARENT_GROUP CoreGroup)
cpack_add_component(Server
	DISPLAY_NAME API
	GROUP ServerGroup)		
cpack_add_component_group(AppGroup
	DISPLAY_NAME App
	PARENT_GROUP CoreGroup)
cpack_add_component(App
	DISPLAY_NAME API
	GROUP AppGroup
	REQUIRED)	
cpack_add_component_group(ResourcesGroup
	DISPLAY_NAME Resources
	PARENT_GROUP CoreGroup)			
cpack_add_component(Resources
	DISPLAY_NAME API
	GROUP ResourcesGroup
	REQUIRED)		
cpack_add_component(AnimationLoader
	GROUP ResourcesGroup)		
cpack_add_component(ImageLoader
	GROUP ResourcesGroup
	REQUIRED)		
cpack_add_component(MeshLoader
	GROUP ResourcesGroup
	REQUIRED)		
cpack_add_component(SkeletonLoader
	GROUP ResourcesGroup)		
cpack_add_component_group(BehaviorGroup
	DISPLAY_NAME Behavior
	PARENT_GROUP CoreGroup)
cpack_add_component(Behavior
	DISPLAY_NAME API
	GROUP BehaviorGroup
	REQUIRED)	
cpack_add_component(NodeGraph	
	GROUP BehaviorGroup)	
cpack_add_component(Handler
	GROUP BehaviorGroup
	REQUIRED)	
cpack_add_component_group(RenderGroup
	DISPLAY_NAME Render
	PARENT_GROUP CoreGroup)
cpack_add_component(Render
	DISPLAY_NAME API
	GROUP RenderGroup
	REQUIRED)	
cpack_add_component(DirectX12
	GROUP RenderGroup)	
cpack_add_component(OpenGL
	GROUP RenderGroup)	
cpack_add_component(Vulkan
	GROUP RenderGroup)	
cpack_add_component_group(MiscGroup
	DISPLAY_NAME Misc)
cpack_add_component(Scene
	GROUP MiscGroup)	
cpack_add_component(SceneRenderer
	GROUP MiscGroup)	
cpack_add_component(Widgets
	GROUP MiscGroup)	
cpack_add_component(WidgetHandler
	GROUP MiscGroup)	
cpack_add_component_group(ToolsGroup
	DISPLAY_NAME Tools)
cpack_add_component(Tools
	DISPLAY_NAME API
	GROUP ToolsGroup
	REQUIRED)	
cpack_add_component(Templates	
	GROUP ToolsGroup)	
cpack_add_component(TextEditor	
	GROUP ToolsGroup)	
cpack_add_component_group(GameGroup
	DISPLAY_NAME Game)
cpack_add_component(ClickBrick
	GROUP GameGroup)	
cpack_add_component_group(DebugGroup
	DISPLAY_NAME Debug
	PARENT_GROUP CoreGroup)
cpack_add_component(Debugger
	DISPLAY_NAME API
	GROUP DebugGroup
	REQUIRED)	
cpack_add_component_group(TestGroup
	DISPLAY_NAME Test)
cpack_add_component(LibA
	GROUP TestGroup)	
cpack_add_component(LibB
	GROUP TestGroup)	
cpack_add_component(CLI
	HIDDEN)
cpack_add_component(CodeGen
	HIDDEN)
cpack_add_component(Curl
	HIDDEN)
cpack_add_component(FilesystemSerialize
	HIDDEN)
cpack_add_component(Im3D
	HIDDEN)
cpack_add_component(ImGui
	HIDDEN)
cpack_add_component(MemorySerialize
	HIDDEN)
cpack_add_component(NetworkSerialize
	HIDDEN)
cpack_add_component(NodeEditor
	HIDDEN)
cpack_add_component(TemplateEngine
	HIDDEN)
cpack_add_component(Trees
	HIDDEN)

packaging()



