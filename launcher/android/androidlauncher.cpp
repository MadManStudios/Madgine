#include "Madgine/clientlib.h"
#include "Madgine/rootlib.h"

#include "androidlauncher.h"

#include <android/native_activity.h>

#include "Generic/systemvariable.h"

#include "Platform/filesystem/fsapi.h"
#include "Platform/helpers/android_jni.h"

#include "Madgine/root/root.h"
#include "Madgine/window/mainwindow.h"

#include "../launcher.h"

namespace Engine {

namespace Platform {
    namespace Window {
        void setup(ANativeActivity *activity);
    }
}

namespace Android {

    template <auto f, typename... Args>
    static void delegate(ANativeActivity *activity, Args... args)
    {
        (static_cast<AndroidLauncher *>(activity->instance)->*f)(args...);
    }

    AndroidLauncher::AndroidLauncher(ANativeActivity *activity)
        : mActivity(activity)
    {
        activity->instance = this;

        activity->callbacks->onDestroy = delegate<&AndroidLauncher::onDestroy>;

        Platform::Window::setup(activity);

        Platform::JNI::setVM(activity->vm, activity->env, activity->clazz);
        Threading::WorkGroup::addStaticThreadGuards(Platform::JNI::initThread, Platform::JNI::finalizeThread);

        mThread = Threading::WorkGroupHandle("Madgine", &AndroidLauncher::go, this);
    }

    void AndroidLauncher::go()
    {
        ANativeActivity *activity = mActivity;

        Platform::Filesystem::setup(activity);

        static Engine::Core::Root root;

        launch([this](Engine::Core::Application &, Engine::Core::MainWindow &mainWindow) { mWindow = &mainWindow; });

        ANativeActivity_finish(activity);
    }

    void AndroidLauncher::onDestroy()
    {
        mWindow->shutdown();
        mThread.detach();
        mActivity->instance = nullptr;
        delete this;
    }

}
}