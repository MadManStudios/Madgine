#include "../interfaceslib.h"

#if ANDROID

#    include "android_jni.h"



namespace Engine {
namespace JNI {

    static JavaVM *sVM = nullptr;
    static jobject sActivity;

    thread_local JNIEnv *sEnv = nullptr;

    static jobject gClassLoader;
    static jmethodID gFindClassMethod;

void setVM(JavaVM *vm, JNIEnv *env, jobject activity)
{
    sVM = vm;
    sActivity = activity;

    jclass acl = env->GetObjectClass(activity);
    jmethodID getClassLoader = env->GetMethodID(acl, "getClassLoader", "()Ljava/lang/ClassLoader;");
    gClassLoader = reinterpret_cast<jclass>(env->NewGlobalRef(env->CallObjectMethod(activity, getClassLoader)));
    jclass classLoader = env->FindClass("java/lang/ClassLoader");
    gFindClassMethod = env->GetMethodID(classLoader, "loadClass", "(Ljava/lang/String;)Ljava/lang/Class;");
}

void initThread()
{    
    sVM->AttachCurrentThread(&sEnv, nullptr);
}

void finalizeThread()
{
    sVM->DetachCurrentThread();
}

jobject activity()
{
    return sActivity;
}

void callStaticFunction(const char *className, const char *functionName, jobject object)
{
    jclass clazz = static_cast<jclass>(sEnv->CallObjectMethod(gClassLoader, gFindClassMethod, sEnv->NewStringUTF(className)));
    jmethodID method = sEnv->GetStaticMethodID(clazz, functionName, "(Landroid/app/Activity;)V");
    if (method == NULL) {
        sEnv->FatalError("method not found");
    }

    // method execution
    sEnv->CallStaticVoidMethod(clazz, method, object);
}

void callStaticFunction2(const char *className, const char *functionName, std::string_view string, jlong v)
{
    jclass clazz = static_cast<jclass>(sEnv->CallObjectMethod(gClassLoader, gFindClassMethod, sEnv->NewStringUTF(className)));
    jmethodID method = sEnv->GetStaticMethodID(clazz, functionName, "(Ljava/lang/String;J)V");
    if (method == NULL) {
        sEnv->FatalError("method not found");
    }

    // method execution
    sEnv->CallStaticVoidMethod(clazz, method, sEnv->NewStringUTF(string.data()), v);
}

void callStaticFunction3(const char *className, const char *functionName, std::string_view string1, jint v1, std::string_view string2, jlong v2)
{
    jclass clazz = static_cast<jclass>(sEnv->CallObjectMethod(gClassLoader, gFindClassMethod, sEnv->NewStringUTF(className)));
    jmethodID method = sEnv->GetStaticMethodID(clazz, functionName, "(Ljava/lang/String;ILjava/lang/String;J)V");
    if (method == NULL) {
        sEnv->FatalError("method not found");
    }

    // method execution
    sEnv->CallStaticVoidMethod(clazz, method, sEnv->NewStringUTF(string1.data()), v1, sEnv->NewStringUTF(string2.data()), v2);
}

void callStaticFunction4(const char *className, const char *functionName, std::string_view string, jint v1, jint v2, jint v3, jint v4, jlong v5)
{
    jclass clazz = static_cast<jclass>(sEnv->CallObjectMethod(gClassLoader, gFindClassMethod, sEnv->NewStringUTF(className)));
    jmethodID method = sEnv->GetStaticMethodID(clazz, functionName, "(Ljava/lang/String;IIIIJ)V");
    if (method == NULL) {
        sEnv->FatalError("method not found");
    }

    // method execution
    sEnv->CallStaticVoidMethod(clazz, method, sEnv->NewStringUTF(string.data()), v1, v2, v3, v4, v5);
}

int callMemberFunction(jobject object, const char *functionName, JNIEnv *env)
{
    jclass clazz = env->GetObjectClass(object);
    jmethodID method = env->GetMethodID(clazz, functionName, "()I");
    if (method == NULL) {
        env->FatalError("method not found");
    }

    // method execution
    return env->CallIntMethod(object, method);
}

void registerNatives(const char *className, std::span<const JNINativeMethod> methods)
{
    jclass clazz = static_cast<jclass>(sEnv->CallObjectMethod(gClassLoader, gFindClassMethod, sEnv->NewStringUTF(className)));
    int result = sEnv->RegisterNatives(clazz, methods.data(), methods.size());
    if (result != JNI_OK)
        throw 0;
}

std::string getExceptionMessage(jthrowable ex, JNIEnv *env)
{
    if (!env)
        env = sEnv;
    jclass clazz = env->GetObjectClass(ex);
    jmethodID getMessage = env->GetMethodID(clazz,
        "toString",
        "()Ljava/lang/String;");
    jstring message = (jstring)env->CallObjectMethod(ex, getMessage);
    const char *mstr = env->GetStringUTFChars(message, NULL);
    std::string result = mstr;
    env->ReleaseStringUTFChars(message, mstr);
    return result;
}

}
}

#endif
