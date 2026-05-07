#include "../interfaceslib.h"

#if ANDROID

#    include "android_jni.h"
#    include "android_utility.h"

namespace Engine {
namespace Android {

    void triggerRumble(std::chrono::milliseconds duration)
    {
        JNIEnv *env = JNI::env();

        jclass activityClass = env->GetObjectClass(JNI::activity());
        jmethodID getSystemService = env->GetMethodID(activityClass, "getSystemService", "(Ljava/lang/String;)Ljava/lang/Object;");
        jfieldID vibratorServiceField = env->GetStaticFieldID(env->FindClass("android/content/Context"), "VIBRATOR_SERVICE", "Ljava/lang/String;");
        jstring vibratorServiceStr = (jstring)env->GetStaticObjectField(env->FindClass("android/content/Context"), vibratorServiceField);
        jobject vibratorService = env->CallObjectMethod(JNI::activity(), getSystemService, vibratorServiceStr);

        // 2. Call Vibrate (Simplified for older APIs, use VibrationEffect for API 26+)
        jclass vibratorClass = env->GetObjectClass(vibratorService);
        jmethodID vibrateMethod = env->GetMethodID(vibratorClass, "vibrate", "(J)V");
        env->CallVoidMethod(vibratorService, vibrateMethod, duration.count());
    }

}
}

#endif
