#include "Madgine/clientlib.h"
#include "Madgine/rootlib.h"

#include "androidlauncher.h"

#include "Madgine/root/root.h"

#include "Madgine/window/mainwindow.h"

#include <android/native_activity.h>

#include "Generic/systemvariable.h"

#include "../launcher.h"
#include "Interfaces/filesystem/fsapi.h"
#include "Interfaces/helpers/android_jni.h"

namespace Engine {

namespace Window {
    void setup(ANativeActivity *activity);
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

        Window::setup(activity);
        
        JNI::setVM(activity->vm, activity->env, activity->clazz);
        Threading::WorkGroup::addStaticThreadGuards(JNI::initThread, JNI::finalizeThread);

        mThread = Threading::WorkGroupHandle("Madgine", &AndroidLauncher::go, this);
    }

    void AndroidLauncher::go()
    {
        ANativeActivity *activity = mActivity;

        Filesystem::setup(activity);

        static Engine::Root::Root root;

        launch([this](Engine::Window::MainWindow &mainWindow) { mWindow = &mainWindow; });

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