#
# Toolchain for cross-compiling to JS using Emscripten with WebAssembly
#
# Modify EMSCRIPTEN_PREFIX to your liking; use EMSCRIPTEN environment variable
# to point to it or pass it explicitly via -DEMSCRIPTEN_PREFIX=<path>.
#
#  mkdir build-emscripten-wasm && cd build-emscripten-wasm
#  cmake .. -DCMAKE_TOOLCHAIN_FILE=../toolchains/generic/Emscripten-wasm.cmake
#

set(CMAKE_SYSTEM_NAME Emscripten)

if(NOT EMSCRIPTEN_PREFIX)

    if(EXISTS "$ENV{EMSDK}/upstream/emscripten")
        file(TO_CMAKE_PATH "$ENV{EMSDK}/upstream/emscripten" EMSCRIPTEN_PREFIX)
    elseif(EXISTS "$ENV{EMSDK}/fastcomp/emscripten")
        file(TO_CMAKE_PATH "$ENV{EMSDK}/fastcomp/emscripten" EMSCRIPTEN_PREFIX)
    else()
        MESSAGE(SEND_ERROR "Emscripten SDK not found! Please make sure the environment variable EMSDK was set properly using 'emsdk_env --global'")
    endif()
endif()

# Help CMake find the platform file
set(CMAKE_MODULE_PATH "${EMSCRIPTEN_PREFIX}/cmake/Modules" ${CMAKE_MODULE_PATH})

set(EMSCRIPTEN_TOOLCHAIN_PATH "${EMSCRIPTEN_PREFIX}/system")

if(CMAKE_HOST_WIN32)
    set(EMCC_SUFFIX ".bat")
else()
    set(EMCC_SUFFIX "")
endif()
set(CMAKE_C_COMPILER "${EMSCRIPTEN_PREFIX}/emcc${EMCC_SUFFIX}")
set(CMAKE_CXX_COMPILER "${EMSCRIPTEN_PREFIX}/em++${EMCC_SUFFIX}")
set(CMAKE_AR "${EMSCRIPTEN_PREFIX}/emar${EMCC_SUFFIX}" CACHE FILEPATH "Emscripten ar" FORCE)
set(CMAKE_RANLIB "${EMSCRIPTEN_PREFIX}/emranlib${EMCC_SUFFIX}" CACHE FILEPATH "Emscripten ranlib" FORCE)

set(CMAKE_FIND_ROOT_PATH ${CMAKE_FIND_ROOT_PATH}
    "${EMSCRIPTEN_TOOLCHAIN_PATH}"
    "${EMSCRIPTEN_PREFIX}")

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

# Otherwise FindCorrade fails to find _CORRADE_MODULE_DIR. Why the heck is this
# not implicit is beyond me.
set(CMAKE_SYSTEM_PREFIX_PATH ${CMAKE_FIND_ROOT_PATH})

set(CMAKE_SHARED_LIBRARY_SUFFIX ".wasm")

cmake_minimum_required(VERSION 3.7)
set(CMAKE_CXX_FLAGS_INIT "-s USE_PTHREADS=0 -s ALLOW_MEMORY_GROWTH=1 -mtail-call")
set(CMAKE_EXE_LINKER_FLAGS_INIT "-s ERROR_ON_UNDEFINED_SYMBOLS=1 -s ALLOW_MEMORY_GROWTH=1 -s MIN_WEBGL_VERSION=2 -s MAX_WEBGL_VERSION=2 -sGL_ASSERTIONS=1 -sTRACE_WEBGL_CALLS=1 -sGL_DEBUG=1 -sFETCH")
set(CMAKE_SHARED_LINKER_FLAGS_INIT "-s SIDE_MODULE=1 -s EXPORT_ALL=1 -s ERROR_ON_UNDEFINED_SYMBOLS=1")
set(CMAKE_CXX_FLAGS_RELEASE_INIT "-DNDEBUG -O3")
set(CMAKE_EXE_LINKER_FLAGS_RELEASE_INIT "-O3 --llvm-lto 1")
set(CMAKE_SHARED_LINKER_FLAGS_RELEASE_INIT "-O3 --llvm-lto 1")
set(CMAKE_CXX_FLAGS_DEBUG_INIT "-g -s DISABLE_EXCEPTION_CATCHING=0")
set(CMAKE_EXE_LINKER_FLAGS_DEBUG_INIT "-g")
set(CMAKE_SHARED_LINKER_FLAGS_DEBUG_INIT "-g")
