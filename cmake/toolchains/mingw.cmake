# Usage:
# cmake .. -DLLVM_ARCH=aarch64 i686 or x86_64 -DLLVM_PREFIX=/path/to/llvm-mingw-prefix -DCMAKE_TOOLCHAIN_FILE=../cmake/Toolchain-llvm-mingw.cmake

set(LLVM_PREFIX /Users/madman/Desktop/llvm-mingw-20220323-ucrt-macos-universal)
set(LLVM_ARCH x86_64)

# the name of the target operating system
SET(CMAKE_SYSTEM_NAME Windows)

# which compilers to use for C and C++
SET(CMAKE_C_COMPILER ${LLVM_PREFIX}/bin/${LLVM_ARCH}-w64-mingw32-clang)
SET(CMAKE_CXX_COMPILER ${LLVM_PREFIX}/bin/${LLVM_ARCH}-w64-mingw32-clang++)
SET(CMAKE_RC_COMPILER ${LLVM_PREFIX}/bin/${LLVM_ARCH}-w64-mingw32-windres)
SET(CMAKE_AR ${LLVM_PREFIX}/bin/${LLVM_ARCH}-w64-mingw32-ar)
SET(CMAKE_RANLIB ${LLVM_PREFIX}/bin/${LLVM_ARCH}-w64-mingw32-ranlib)
SET(CMAKE_SYSTEM_PROCESSOR ${LLVM_ARCH})
set(CMAKE_EXE_LINKER_FLAGS "-static-libgcc -static-libstdc++ -Wl,-pdb=")
SET(CMAKE_C_FLAGS -gcodeview)
SET(CMAKE_CXX_FLAGS -gcodeview)
SET(CMAKE_CXX_COMPILER_ID Clang)

SET(POSIX)
SET(MINGW)

# here is the target environment located
SET(CMAKE_FIND_ROOT_PATH ${LLVM_PREFIX}/generic-w64-mingw32 ${LLVM_PREFIX}/${LLVM_ARCH}-w64-mingw32/bin ${PROJECT_SOURCE_DIR}/dependencies-win-${LLVM_ARCH})

# adjust the default behaviour of the FIND_XXX() commands:
# search headers and libraries in the target environment, search
# programs in the host environment
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ALWAYS)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)