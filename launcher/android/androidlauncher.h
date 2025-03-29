#pragma once

#include "Modules/threading/workgrouphandle.h"

struct ANativeActivity;
struct ANativeWindow;
struct AInputQueue;
struct ARect;

namespace Engine {
namespace Android {

    struct AndroidLauncher {

        AndroidLauncher(ANativeActivity *activity);

    protected:
        void go();

        void onDestroy();
        void onNativeWindowCreated(ANativeWindow *window);
        void onNativeWindowDestroyed(ANativeWindow *window);
        void onNativeWindowResized(ANativeWindow *window);
        void onConfigurationChanged();
        void onContentRectChanged(const ARect *rect);
        void onPause();
        void onResume();
        void onInputQueueCreated(AInputQueue *queue);
        void onInputQueueDestroyed(AInputQueue *queue);

    private:
        ANativeActivity *mActivity;
        Threading::WorkGroupHandle mThread;
        Window::MainWindow *mWindow = nullptr;
    };

}
}