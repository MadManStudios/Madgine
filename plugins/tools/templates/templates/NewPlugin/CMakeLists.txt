cmake_minimum_required (VERSION 3.10)

cmake_policy(SET CMP0011 NEW)
cmake_policy(SET CMP0049 NEW)
cmake_policy(SET CMP0091 NEW)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

Project(%name%)

if (CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
	include($ENV{MADGINE}/cmake/MadgineSDK.cmake)
endif()

add_subdirectory(api)