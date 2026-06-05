#pragma once

#if ANDROID

#    include <jni.h>

namespace Engine {
namespace Platform {
    namespace JNI {

        PLATFORM_EXPORT void setVM(JavaVM *vm, JNIEnv *env, jobject activity);

        PLATFORM_EXPORT void initThread();
        PLATFORM_EXPORT void finalizeThread();

        PLATFORM_EXPORT jobject activity();
        PLATFORM_EXPORT JNIEnv *env();

        PLATFORM_EXPORT void callStaticFunction(const char *className, const char *functionName, jobject object);
        PLATFORM_EXPORT void callStaticFunction2(const char *className, const char *functionName, std::string_view string, jlong v);
        PLATFORM_EXPORT void callStaticFunction3(const char *className, const char *functionName, std::string_view string1, jint v1, std::string_view string2, jlong v2);
        PLATFORM_EXPORT void callStaticFunction4(const char *className, const char *functionName, std::string_view string, jint v1, jint v2, jint v3, jint v4, jlong v5);

        PLATFORM_EXPORT int callMemberFunction(jobject object, const char *functionName);
        PLATFORM_EXPORT bool callMemberFunction2(jobject object, const char *functionName);
        PLATFORM_EXPORT int callMemberFunction3(jobject object, const char *functionName, jint v);

        PLATFORM_EXPORT void registerNatives(const char *className, std::span<const JNINativeMethod> methods);

        PLATFORM_EXPORT std::string getExceptionMessage(jthrowable ex, JNIEnv *env = nullptr);

        PLATFORM_EXPORT jobject construct(const char *className, jint v1, jint v2);

    }
}
}

#endif