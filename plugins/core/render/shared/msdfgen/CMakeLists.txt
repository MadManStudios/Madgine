project(msdfgen)

add_definitions(-DMSDFGEN_USE_CPP11)


# Note: Clang doesn't support openMP by default...
#find_package(OpenMP)
#if (OPENMP_FOUND)
#	set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
#	set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
#endif()

#----------------------------------------------------------------
# Support Functions
#----------------------------------------------------------------


file(GLOB_RECURSE msdfgen_HEADERS
	"core/*.h"
	"core/*.hpp"
)

file(GLOB_RECURSE msdfgen_SOURCES
	"core/*.cpp"
)

# Build the library (aliased name because it's the same target name the exe)
add_library(lib_msdfgen STATIC ${msdfgen_SOURCES} ${msdfgen_HEADERS})
target_include_directories(lib_msdfgen PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)
set_target_properties(lib_msdfgen PROPERTIES OUTPUT_NAME msdfgen)

set_property(TARGET lib_msdfgen PROPERTY POSITION_INDEPENDENT_CODE ON)

install_to_workspace(Madgine EXPORT_LIB TARGETS lib_msdfgen)