#pragma once

#include "macros.h"

#if _WIN32
#    define WINDOWS 1
#else
#    define WINDOWS 0
#endif

#if __APPLE__
#    define __unix__ 1
#endif

#if __MINGW32__ || __unix__
#    define POSIX 1
#else
#    define POSIX 0
#endif

#if __unix__
#    define UNIX 1
#    if __EMSCRIPTEN__
#        define EMSCRIPTEN 1
#    else
#        define EMSCRIPTEN 0
#    endif
#    if __linux__
#        if __ANDROID__
#            define ANDROID 1
#            define LINUX 0
#        else
#            define LINUX 1
#            define ANDROID 0
#        endif
#    endif
#    if __APPLE__
#        include <TargetConditionals.h>
#        if TARGET_IPHONE_SIMULATOR || TARGET_OS_IPHONE
#            define IOS 1
#            define OSX 0
#        elif TARGET_OS_MAC
#            define OSX 1
#            define IOS 0
#        else
#            error "Unknown Apple platform"
#        endif
#    else
#        define OSX 0
#        define IOS 0
#    endif
#else
#    define UNIX 0
#    define ANDROID 0
#    define LINUX 0
#    define EMSCRIPTEN 0
#    define OSX 0
#    define IOS 0
#endif

#ifdef _WIN32
#    ifdef __GNUC__
#        define DLL_EXPORT_TAG __attribute__((dllexport))
#        define DLL_IMPORT_TAG __attribute__((dllimport))
#    else
#        define DLL_EXPORT_TAG __declspec(dllexport)
#        define DLL_IMPORT_TAG __declspec(dllimport)
#    endif
#else
#    define DLL_EXPORT_TAG __attribute__((visibility("default")))
#    define DLL_IMPORT_TAG __attribute__((weak))
#endif

#ifdef STATIC_BUILD
#    define DLL_IMPORT
#    define DLL_EXPORT
#else
#    define DLL_IMPORT DLL_IMPORT_TAG
#    define DLL_EXPORT DLL_EXPORT_TAG
#endif

/*
//+ clang on windows
//- MSVC
#define DLL_IMPORT_VARIABLE2(Type, Name, ...) template <__VA_ARGS__> DLL_IMPORT_VARIABLE Type Name;
#define DLL_EXPORT_VARIABLE2(Type, ns, Name, Init, ...) template <> \
                                              DLL_EXPORT_VARIABLE Type (ns Name<__VA_ARGS__>) = Init;
*/

#define DLL_IMPORT_VARIABLE(Type, Name, ArgumentType) \
    template <ArgumentType T>                         \
    extern DLL_IMPORT Type Name##_instance;           \
    template <ArgumentType T>                         \
    Type *Name = &Name##_instance<T>;

#define DLL_EXPORT_VARIABLE(qualifier, Type, ns, Name, Init, ...) \
    template <>                                                        \
    DLL_EXPORT qualifier Type ns Name##_instance<__VA_ARGS__> = Init;

#define DLL_IMPORT_VARIABLE2(Type, Name, ...) \
    template <__VA_ARGS__>                    \
    Type &Name();

#define DLL_EXPORT_VARIABLE3(qualifier, Type, ActualType, ns, Name, ext, Init, ...) \
    static qualifier ActualType CONCAT2(__##Name##ext##_helper_, __LINE__) = Init;  \
    template <>                                                                     \
    DLL_EXPORT Type &ns Name<__VA_ARGS__>()                                         \
    {                                                                               \
        return CONCAT2(__##Name##ext##_helper_, __LINE__);                          \
    }                                                                               \
    template Type &ns Name<__VA_ARGS__>();

#define DLL_EXPORT_VARIABLE3_ORDER(qualifier, Type, ActualType, ns, Name, ext, Init, ...) \
    template <>                                                                           \
    DLL_EXPORT Type &ns Name<__VA_ARGS__>()                                               \
    {                                                                                     \
        static qualifier ActualType dummy = Init;                                         \
        return dummy;                                                                     \
    }                                                                                     \
    template Type &ns Name<__VA_ARGS__>();                                                \
    static Type &CONCAT2(__##Name##ext##_helper_, __LINE__) = ns Name<__VA_ARGS__>();

#define DLL_EXPORT_VARIABLE2(qualifier, Type, ns, Name, Init, ...) DLL_EXPORT_VARIABLE3(qualifier, Type, Type, ns, Name, , SINGLE_ARG(Init), __VA_ARGS__)
#define DLL_EXPORT_VARIABLE2_ORDER(qualifier, Type, ns, Name, Init, ...) DLL_EXPORT_VARIABLE3_ORDER(qualifier, Type, Type, ns, Name, , SINGLE_ARG(Init), __VA_ARGS__)

/*#define TEMPLATE_INSTANTIATION(type) template type
#define TEMPLATE_INSTANTIATION_EXPORT(type) template type DLL_EXPORT
#define TEMPLATE_INSTANTIATION_IMPORT(type) extern template type*/

#if WINDOWS
#    pragma warning(disable : 4251)
#    pragma warning(disable : 4275)
#    pragma warning(disable : 4661)
#endif

#ifdef _MSC_VER
#    define _CRT_SECURE_NO_WARNINGS
#endif

#if __clang__
#    define CLANG 1
#else
#    define CLANG 0
#endif