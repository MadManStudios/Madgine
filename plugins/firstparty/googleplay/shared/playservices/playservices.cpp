#include "Interfaces/interfaceslib.h"

#include "playservices.h"

#include "Interfaces/helpers/android_jni.h"

namespace PlayServices {

void VoidReceiver_setValue(JNIEnv *env, jclass, Engine::Execution::VirtualReceiverBase<std::string> *rec)
{
    rec->set_value();
}

void VoidReceiver_setError(JNIEnv *env, jclass, Engine::Execution::VirtualReceiverBase<std::string> *rec, jthrowable ex)
{
    rec->set_error(Engine::JNI::getExceptionMessage(ex, env));
}

void ScoresReceiver_addScore(JNIEnv *env, jclass, Leaderboards::ScoresState *rec, jlong id, jstring name, jlong rank, jint score)
{
    rec->mScores.mScores.emplace_back(id, env->GetStringUTFChars(name, NULL), rank, score);
}

void ScoresReceiver_setValue(JNIEnv *env, jclass, Leaderboards::ScoresState *rec)
{
    rec->set_value(std::move(rec->mScores));
}

void ScoresReceiver_setError(JNIEnv *env, jclass, Engine::Execution::VirtualReceiverBase<std::string, Leaderboards::Scores> *rec, jthrowable ex)
{
    rec->set_error(Engine::JNI::getExceptionMessage(ex, env));
}

void setup()
{
    Engine::JNI::callStaticFunction("com/Madgine/PlayServices", "Setup", Engine::JNI::activity());

    static const JNINativeMethod voidReceiverMethods[] = {
        { "setValue", "(J)V", reinterpret_cast<void *>(VoidReceiver_setValue) },
        { "setError", "(JLjava/lang/Exception;)V", reinterpret_cast<void *>(VoidReceiver_setError) },
    };
    Engine::JNI::registerNatives("com/Madgine/VoidReceiver", voidReceiverMethods);

    static const JNINativeMethod scoresReceiverMethods[] = {
        { "addScore", "(JJLjava/lang/String;JI)V", reinterpret_cast<void *>(ScoresReceiver_addScore) },
        { "setValue", "(J)V", reinterpret_cast<void *>(ScoresReceiver_setValue) },
        { "setError", "(JLjava/lang/Exception;)V", reinterpret_cast<void *>(ScoresReceiver_setError) },
    };
    Engine::JNI::registerNatives("com/Madgine/ScoresReceiver", scoresReceiverMethods);
}

namespace Achievements {

    void unlockImpl(Engine::Execution::VirtualReceiverBase<std::string> &rec, std::string_view id)
    {
        Engine::JNI::callStaticFunction2("com/Madgine/PlayServices", "Achievements_Unlock", id, reinterpret_cast<jlong>(&rec));
    }

}

namespace Leaderboards {
    void ScoresState::start()
    {
        Engine::JNI::callStaticFunction4("com/Madgine/PlayServices", "Leaderboards_GetLeaderboard", mId, mAccessmode, mReferenceRank, mRangeBegin, mRangeEnd, reinterpret_cast<jlong>(this));
    }

    void submitScoreImpl(Engine::Execution::VirtualReceiverBase<std::string> &rec, std::string_view leaderboardId, int32_t score, std::string_view name)
    {
        Engine::JNI::callStaticFunction3("com/Madgine/PlayServices", "Leaderboards_SubmitScore", leaderboardId, score, name, reinterpret_cast<jlong>(&rec));
    }
}
}
