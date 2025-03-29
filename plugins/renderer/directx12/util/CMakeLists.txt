

project(dx12drivertest)

add_executable(dx12drivertest main.cpp)

set_target_properties(dx12drivertest PROPERTIES
	RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/bin
	RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_CURRENT_BINARY_DIR}/bin
	RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_CURRENT_BINARY_DIR}/bin
	RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO ${CMAKE_CURRENT_BINARY_DIR}/bin)


target_link_libraries(dx12drivertest PRIVATE d3d12 dxgi)

target_compile_options(dx12drivertest PRIVATE /MT)
