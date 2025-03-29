#pragma once

#if ANDROID

#include <jni.h>

namespace Engine {
namespace JNI {

    INTERFACES_EXPORT void setVM(JavaVM *vm, JNIEnv *env, jobject activity);

    INTERFACES_EXPORT void initThread();
    INTERFACES_EXPORT void finalizeThread();

    INTERFACES_EXPORT jobject activity();
    
    INTERFACES_EXPORT void callStaticFunction(const char *className, const char *functionName, jobject object);
    INTERFACES_EXPORT void callStaticFunction2(const char *className, const char *functionName, std::string_view string, jlong v);
    INTERFACES_EXPORT void callStaticFunction3(const char *className, const char *functionName, std::string_view string1, jint v1, std::string_view string2, jlong v2);
    INTERFACES_EXPORT void callStaticFunction4(const char *className, const char *functionName, std::string_view string, jint v1, jint v2, jint v3, jint v4, jlong v5);

    INTERFACES_EXPORT int callMemberFunction(jobject object, const char *functionName, JNIEnv *env);

    INTERFACES_EXPORT void registerNatives(const char *className, std::span<const JNINativeMethod> methods);

    INTERFACES_EXPORT std::string getExceptionMessage(jthrowable ex, JNIEnv *env = nullptr);

}
}

#endif