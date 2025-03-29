#include "handlerlib.h"

#include "handlermanager.h"

#include "Madgine/window/mainwindow.h"

#include "Modules/debug/profiler/profile.h"

#include "Meta/keyvalue/metatable_impl.h"

#include "Meta/serialize/configs/controlled.h"

#include "handler.h"

#include "Modules/uniquecomponent/uniquecomponentcollector.h"

#include "Madgine/app/application.h"

#include "Modules/threading/awaitables/awaitabletimepoint.h"

#include "Modules/threading/awaitables/awaitablesender.h"

METATABLE_BEGIN(Engine::HandlerManager)
MEMBER(mHandlers)
METATABLE_END(Engine::HandlerManager)

namespace Engine {

    static std::chrono::milliseconds fixedUpdateInterval = 15ms;

    HandlerManager::HandlerManager(App::Application &app, Window::MainWindow &window)
        : mApp(app)
        , mWindow(window)
        , mHandlers(*this)
    {
        window.taskQueue()->addSetupSteps(
            [this]() { return callInit(); },
            [this]() { return callFinalize(); });
    }

    HandlerManager::~HandlerManager()
    {
    }

    Threading::Task<bool> HandlerManager::init()
    {
        co_await mWindow.state();

        //Execution::detach(mgr.updatedSignal().connect([this] { onUpdate(); })); TODO

        for (const std::unique_ptr<HandlerBase> &handler : mHandlers)
            co_await handler->callInit();

        mWindow.addListener(this);

        startLifetime();

        co_return true;
    }

    Threading::Task<void> HandlerManager::finalize()
    {
        endLifetime();
        co_await mLifetime.finished();

        mWindow.removeListener(this);

        for (const std::unique_ptr<HandlerBase> &handler : mHandlers)
            co_await handler->callFinalize();
    }

    void HandlerManager::startLifetime()
    {
        Execution::detach(mLifetime);

        for (const std::unique_ptr<HandlerBase> &handler : mHandlers)
            handler->startLifetime();
    }

    void HandlerManager::endLifetime()
    {
        mLifetime.end();
    }

    Debug::DebuggableLifetime<> &HandlerManager::lifetime()
    {
        return mLifetime;
    }

    void HandlerManager::onActivate(bool active)
    {
        if (active)
            startLifetime();
        else
            endLifetime();
    }

    App::Application &HandlerManager::app() const
    {
        return mApp;
    }

    Window::MainWindow &HandlerManager::window() const
    {
        return mWindow;
    }

    void HandlerManager::shutdown()
    {
        mWindow.shutdown();
    }

    void HandlerManager::clear()
    {
        /*while (!mModalWindowList.empty()) {
            closeModalWindow(mModalWindowList.top());
        }*/
    }

    void HandlerManager::hideCursor(bool keep)
    {
        if (!isCursorVisible())
            return;
        mKeepingCursorPos = keep;
        if (keep) {
            /*const OIS::MouseState &mouseState = mMouse->getMouseState();
				mKeptCursorPosition = { (float)mouseState.X.abs, (float)mouseState.Y.abs };*/
        }
        //mGUI.hideCursor();

        for (const std::unique_ptr<HandlerBase> &h : mHandlers)
            h->onMouseVisibilityChanged(false);
    }

    void HandlerManager::showCursor()
    {
        if (isCursorVisible())
            return;
        if (mKeepingCursorPos) {
            /*OIS::MouseState &mutableMouseState = const_cast<OIS::MouseState &>(mMouse->getMouseState());
				mutableMouseState.X.abs = mKeptCursorPosition.x;
				mutableMouseState.Y.abs = mKeptCursorPosition.y;
				callSafe([&]() {
					mouseMoved(OIS::MouseEvent(mMouse, mutableMouseState));*/
            //mGUI.showCursor();
            /*});*/
        } else {
            //mGUI.showCursor();
        }
        for (const std::unique_ptr<HandlerBase> &h : mHandlers)
            h->onMouseVisibilityChanged(true);
    }

    bool HandlerManager::isCursorVisible() const
    {
        return /* mGUI.isCursorVisible()*/ true;
    }

    std::set<HandlerBase *> HandlerManager::getHandlers()
    {
        std::set<HandlerBase *> result;
        for (const std::unique_ptr<HandlerBase> &h : mHandlers) {
            result.insert(h.get());
        }
        return result;
    }

    std::string_view HandlerManager::key() const
    {
        return "UI";
    }

    HandlerBase &HandlerManager::getHandler(size_t i)
    {
        return mHandlers.get(i);
    }

    Threading::TaskQueue *HandlerManager::viewTaskQueue() const
    {
        return mWindow.taskQueue();
    }

    Threading::TaskQueue *HandlerManager::modelTaskQueue() const
    {
        return mApp.taskQueue();
    }

}
