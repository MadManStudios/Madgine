
push_static()

set (BUILD_UNIT_TESTS OFF CACHE BOOL "" FORCE)
set (BUILD_EXTRAS OFF CACHE BOOL "" FORCE)
set (BUILD_BULLET2_DEMOS OFF CACHE BOOL "" FORCE)
add_subdirectory(bullet3-3.24)
target_include_directories(LinearMath PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/bullet3-3.24/src>)

add_notices(LinearMath 
	READABLE_NAME Bullet3D 
	CREDITS "Physics-Simulation: Bullet3D - http://bulletphysics.org")

target_include_directories(LinearMath INTERFACE bullet3-3.24/src)

pop_static()