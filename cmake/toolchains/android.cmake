


if(NOT ANDROID_SDK)
    if(DEFINED ENV{ANDROID_SDK})
        file(TO_CMAKE_PATH "$ENV{ANDROID_SDK}" ANDROID_SDK)
    else()
        set(ANDROID_SDK "C:\\android-sdk")
    endif()
endif()


if (NOT EXISTS ${ANDROID_SDK})
	MESSAGE(SEND_ERROR "Please set location of ANDROID_SDK")
endif()

set(ANDROID_NO_UNDEFINED report-all)
set(ANDROID_PLATFORM android-28) # API level
#specify ANDROID_ABI in arguments

if(NOT ANDROID_NDK)
    if(DEFINED ENV{ANDROID_NDK})
        file(TO_CMAKE_PATH "$ENV{ANDROID_NDK}" ANDROID_NDK)
    else()
        set(ANDROID_NDK ${ANDROID_SDK}/ndk-bundle)
    endif()
endif()

set(ANDROID_TOOLCHAIN clang)

include (${ANDROID_NDK}/build/cmake/android.toolchain.cmake)
